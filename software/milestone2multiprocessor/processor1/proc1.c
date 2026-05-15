/*
   Processor 1, added for M2 task 3, takes the input from the camera via spi and stores it in 
   shared sdram memory with processor 0.
   Also recieves interrupts from keys and accelerometer and communicates them to
   processor 0 via interprocessor interrupts.
*/
// Created By : Team A06
// Created Date: 15/05/2026
// version = Final

#include <stdio.h>
#include <stdint.h>
#include "system.h"
#include "io.h"
#include <altera_avalon_spi.h>
#include <altera_avalon_mutex.h>
#include "altera_avalon_pio_regs.h"
#include <stdlib.h>
#include "sys/alt_irq.h"

// Shared Memory between processor0 and processor1
#define SHARED_MAILBOX_BASE 0x029896A0 
// Actual mailbox starts at 0x02895460
// 0x02895460 to 0x029896A0 is reserved for flags and other interprocessor data (excluding frames)

// For accelerometer rotation to rotate frames in quad mode
#define THRESHOLD 75 
#define IM_SIZE ((240 * 320)*3/2)
#define IM_SIZEsmll ((120 * 160)*3/2)

// Gyro write Registers
#define BW_RATE 0x2c
#define POWER_CONTROL 0x2d
#define DATA_FORMAT 0x31
#define INT_ENABLE 0x2E
#define INT_MAP 0x2F
#define THRESH_ACT 0x24
#define THRESH_INACT 0x25
#define TIME_INACT 0x26
#define ACT_INACT_CTL 0x27
#define THRESH_FF 0x28
#define TIME_FF 0x29
#define TAP_AXES 0x2a
#define TAP_THRES 0x1d
#define LATENT 0x22
#define DUR 0x21
#define WINDOW 0x23

// Gyro read Registers
#define INT_SOURCE 0x30
#define Y_LB 0x34
#define Y_HB 0x35

#define CONFIG_LENGHT 16 * 2 // number of config details to send to initlise gyro
#define READ_Y_AXIS 0xc0 | Y_LB // enable read bit and multi byte

// Define desired setup settings with write location then value to write
alt_u8 gyro_config[CONFIG_LENGHT] = {
    DATA_FORMAT, 0x0b, // 4-wire SPI, full resolution, +/- 16g
    THRESH_ACT, 0x04,
    THRESH_INACT, 0x02,
    TIME_INACT, 0x02,
    ACT_INACT_CTL, 0xff,
    THRESH_FF, 0x09,
    TIME_FF, 0x46,
    TAP_THRES, 0x1f,
    TAP_AXES, 0x07,
    LATENT, 0x85,
    DUR, 0x40,
    WINDOW, 0xc0,
    BW_RATE, 0x0a,
    INT_ENABLE, 0x00,
    INT_MAP, 0x20,
    POWER_CONTROL, 0x08};

volatile int quad_mode = 0; // 0 = single, 1 = quad. volatile because changes outside of main loop
volatile int lastEdge = 0;
volatile int keyPressed = 0;

int rotation = 0;
volatile int quadMode = 0;

// ISR function for KEY PRESSES
void key_isr(void* context)
{
	// Clear edge capture register to acknowledge interrupt
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_BASE, 0);
    quadMode = !quadMode;
    keyPressed = 1;
}

int main() {
    alt_mutex_dev* mutex = altera_avalon_mutex_open(MUTEX_0_NAME);
	
	//initialise flag adresses to 0
    IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-1, 0);
    IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-2, 0);
    IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-3, 0);
    IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-4, 0);
    IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-5, 0);
    IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-6, 0);
    IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-10, 0);

    // gyro
    alt_u8 gyro_data_out;
    for (int i = 0; i < CONFIG_LENGHT; i += 2) {
		alt_avalon_spi_command(SPI_0_BASE, 0, 2, gyro_config + i, 0, &gyro_data_out, 0);
	}
	
    alt_u8 readY = READ_Y_AXIS;
    alt_16 yData;

	// Camera settings var
    alt_u8 camSet = 21; 

	// For writing junk info into when changing cam settings
    static uint8_t dummy_buffer[IM_SIZE]; 
	
	// Keep track of if mode changes, to update cam settings
    int lastQuadMode = 0;

    // buffer to hold SPI data before moving to SDRAM
    static uint8_t image_buffer[IM_SIZE];

    camSet = 21; // default settings except "settings changed" = true
    alt_avalon_spi_command(SPI_0_BASE, 1, 1, &camSet, IM_SIZE, image_buffer, 0);

    // Clear edge capture register
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_BASE, 0);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(SW_BASE, 0);
    // Enable interrupts
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEY_BASE, 0x1);
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(SW_BASE, 0x3FF);
    // set wire to 0
    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE, 0);

    // Id interrupt controller, irq number, name of isr function, context, flag
    alt_ic_isr_register(KEY_IRQ_INTERRUPT_CONTROLLER_ID,KEY_IRQ,key_isr,NULL,0x0);
  
    while (1) {
        // Read Gyro (Processor 1 owns SPI)
		alt_avalon_spi_command(SPI_0_BASE, 0, 1, &readY, 2, (alt_u8*)&yData, 0);

		if (quadMode != lastQuadMode) {
			if (quadMode == 1){
				lastQuadMode = 1;
				camSet = 23; // default settings except downscaled & "settings changed" = true

				while (IORD_ALTERA_AVALON_PIO_DATA(CAM_READY_BASE) == 0) {
					// For debugging
					printf("Not ready to change settings yet\n");
				}

				alt_avalon_spi_command(SPI_0_BASE, 1, 1, &camSet, IM_SIZEsmll, dummy_buffer, 0);
			}
			else {
				lastQuadMode = 0;
				// Default settings except "settings changed" = true
				camSet = 21; 

				while (IORD_ALTERA_AVALON_PIO_DATA(CAM_READY_BASE) == 0) {
					printf("Not ready to change settings yet\n");
				}

				alt_avalon_spi_command(SPI_0_BASE, 1, 1, &camSet, IM_SIZE, dummy_buffer, 0);
			}
		}

		// If quad mode on run accelerometer interrupts
    	if (quadMode == 1){
    		if (yData > 75) {
    			rotation = (rotation + 1) % 4;
    			altera_avalon_mutex_lock(mutex, 1);
				// Set rotation value in shared sdram
    			IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 2 , rotation); 
				// Set rotation ocurred flag to 1 in sdram
    			IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 5 , 1); 
    			altera_avalon_mutex_unlock(mutex);
				
    			//send interrupt to proc0
        	    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE , 0x1);
        	    printf("sent rotation interrupt \n");
        	    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE, 0);
    		}else if (yData < -75) {
    			rotation = (rotation + 3) % 4; // -1 mod 4
    			altera_avalon_mutex_lock(mutex, 1);
				// Set rotation value in shared sdram
    			IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 2 , rotation); 
				// Set rotation ocurred flag to 1 in sdram
    			IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 5 , 1); 
    			altera_avalon_mutex_unlock(mutex);
				
    			//send interrupt to proc0
        	    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE , 0x1);
        	    printf("sent rotation interrupt \n");
        	    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE, 0);
    		}

    		while (IORD_ALTERA_AVALON_PIO_DATA(CAM_READY_BASE) == 0) {

			}
    	    alt_avalon_spi_command(SPI_0_BASE, 1, 0, NULL, IM_SIZEsmll, image_buffer, 0);
    	}else{

    		while (IORD_ALTERA_AVALON_PIO_DATA(CAM_READY_BASE) == 0) {

			}
    		alt_avalon_spi_command(SPI_0_BASE, 1, 0, NULL, IM_SIZE, image_buffer, 0);
    	}

		// Copy frame to shared mailbox
        for (int i = 0; i < IM_SIZE; i++) {
            IOWR_8DIRECT(SHARED_MAILBOX_BASE, i, image_buffer[i]);
        }

		// Set flag of frame sent for fps calculation in processor 0
        altera_avalon_mutex_lock(mutex, 1);
		IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 10 , 1);
		altera_avalon_mutex_unlock(mutex);

    	if (keyPressed == 1){
			// Send quadMode flag & key interrupt flag to sdram
    		altera_avalon_mutex_lock(mutex, 1);
    		IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 1 , quadMode);	
       	    IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 4 , 1);
       	    altera_avalon_mutex_unlock(mutex);
			
       	    // Send interrupt to proc 0
    	    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE , 0x1);
    	    printf("sent key interrupt %d \n", quadMode);
    	    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE, 0);

    	    keyPressed = 0;
    	}

    }
    return 0;
}
