#include <stdio.h>
#include <stdint.h>
#include "system.h"
#include "io.h"
#include <altera_avalon_spi.h>
#include <altera_avalon_mutex.h>
#include "altera_avalon_pio_regs.h"
#include <stdlib.h>
#include "sys/alt_irq.h"


// Shared Memory Definitions
//#define SHARED_MAILBOX_BASE 0x03FFF000
//#define SHARED_MAILBOX_BASE 0x03312D20
#define SHARED_MAILBOX_BASE 0x0298969F

#define IM_SIZE (240 * 320)
//#define IM_SIZE (10)

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
#define X_LB 0x32
#define X_HB 0x33
#define Y_LB 0x34
#define Y_HB 0x35
#define Z_LB 0x36
#define Z_HB 0x37

#define CONFIG_LENGHT 16 * 2 // number of config details to send to initlise gyro

#define READ_X_AXIS 0xc0 | X_LB // enable read bit and multi byte
#define READ_Y_AXIS 0xc0 | Y_LB // enable read bit and multi byte
#define READ_Z_AXIS 0xc0 | Z_LB // enable read bit and multi byte

// define desired setup settings with write location then value to write
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


//use later on
void pio_isr_proc1(void* context) {
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(IRQ_TO_PROC1_BASE, 0x1);
    //modify flag
    printf("$");
}

//SET IRQ CONTROLLER
//alt_ic_isr_register(IRQ_TO_PROC1_IRQ_INTERRUPT_CONTROLLER_ID, IRQ_TO_PROC1_IRQ, pio_isr_proc1, NULL, NULL);
//IOWR_ALTERA_AVALON_PIO_IRQ_MASK(IRQ_FROM_PROC1_BASE, 0x1); // Enable IRQ

//TO RECIEVE
//while (flag == 0){dowhatever};
//flag = 0;

//TO SEND
//IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE, 1); // rise
//wait();
//IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE, 0); // Pulse low

int main() {
    //printf("processor1\n");

    // mutex
    alt_mutex_dev* mutex = altera_avalon_mutex_open(MUTEX_0_NAME);

    // gyro
    /*
    alt_u8 gyro_data_out;
    for (int i = 0; i < 6; i += 2) {
        alt_avalon_spi_command(SPI_0_BASE, 0, 2, gyro_config + i, 0, &gyro_data_out, 0);
    }*/

    alt_u8 readX = READ_X_AXIS;
    alt_u8 readY = READ_Y_AXIS;
    alt_u8 readZ = READ_Z_AXIS;
    alt_16 xData;
    alt_16 yData;
    alt_16 zData;

    alt_u8 cam_set = 0;

    // buffer to hold SPI data before moving to SDRAM
    static uint8_t image_buffer[IM_SIZE];
    int framenum = 0;
    while (1) {
        // 1. Read Gyro (Processor 1 owns SPI)
        //alt_avalon_spi_command(SPI_0_BASE, 0, 1, &readX, 2, &xData, 0);
        //alt_avalon_spi_command(SPI_0_BASE, 0, 1, &readY, 2, &yData, 0);
        //alt_avalon_spi_command(SPI_0_BASE, 0, 1, &readZ, 2, &zData, 0);
        //printf("X axis: %4d\t Y axis: %4d\t Z axis %4d\n", xData, yData, zData);


        // 2. Capture Frame from Camera into local buffer
        //spibase, CS, byte length, setting, n of bytes returned, where to put, flags
        // use &imge_buffer instead?
        alt_avalon_spi_command(SPI_0_BASE, 1, 1, &cam_set, IM_SIZE, image_buffer, 0);

        // ARIFICIAL DATA TO MAKE SURE IT WORKS
        /*
        for (int i = 0; i< IM_SIZE; i++){
        	if (i<(IM_SIZE/2)){
        		image_buffer[i] = 16;
        	}else{image_buffer[i] = 18;}
        }
		*/

        // 3. Critical Section: Copy to Shared Mailbox
        altera_avalon_mutex_lock(mutex, 1);
        //printf("just locked mutex \n");
        //if (altera_avalon_mutex_is_mine(mutex) || altera_avalon_mutex_lock(mutex, 1) == 0) {
        for (int i = 0; i < IM_SIZE; i++) {
        	//#define IOWR_8DIRECT(BASE, OFFSET, DATA)

            IOWR_8DIRECT(SHARED_MAILBOX_BASE, i, image_buffer[i]);
            // each image_buffer[i] is a byte, each 320 bytes is a row
        }
        framenum += 1;
        altera_avalon_mutex_unlock(mutex);
        //printf("just unlocked mutex \n");
        //for (volatile int i = 0; i<500000; i++){}
        //printf("sent frame %d to shared memory in sdram first pixel value: %d \n", framenum, image_buffer[0]);

    }
    return 0;
}
