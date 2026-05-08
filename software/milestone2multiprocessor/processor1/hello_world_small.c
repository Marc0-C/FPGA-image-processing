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
#define SHARED_MAILBOX_BASE 0x029896A0 //actual mailbox starts at 0x02895460 but its reserved for flags
// -1 KEY, -2 ROTATE, -3 SWITCH, -4,-5,-6: CONTROL key, rot and switch
#define THRESHOLD 75 // for accelerometer rotation to rotate frames in quad mode

#define IM_SIZE (240 * 320)
#define IM_SIZEsmll (120 * 160)

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

volatile int quad_mode = 0; // 0 = single, 1 = quad. volatile because changes outside of main loop
volatile int lastEdge = 0;
volatile int keypressed = 0;
volatile int switchmoved = 0;
int rotation = 0;
int switchvalue = 0;


//use later on
void pio_isr_proc1(void* context) {
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(IRQ_TO_PROC1_BASE, 0x1);
    //modify flag
    printf("$");
}

volatile int quadmode = 0;

// ISR function for KEY PRESSES
void key_isr(void* context)
{
	// Clear edge capture register to acknowledge interrupt
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_BASE, 0);
    //set wire irq to 0
    if (quadmode == 0){quadmode = 1;
    }else{quadmode = 0;}
    keypressed = 1;
}

void sw_isr(void* context)
{
	// Clear edge capture register to acknowledge interrupt
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(SW_BASE, 0);
	switchmoved = 1; //tell rest of program interrupt has been called


}

int main() {
    alt_mutex_dev* mutex = altera_avalon_mutex_open(MUTEX_0_NAME);
    IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-1, 0);
    IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-2, 0);
    IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-3, 0);
    IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-4, 0); //initialise controls to 0
    IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-5, 0);
    IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-6, 0);

    // gyro
    alt_u8 gyro_data_out;
    for (int i = 0; i < CONFIG_LENGHT; i += 2) {alt_avalon_spi_command(SPI_0_BASE, 0, 2, gyro_config + i, 0, &gyro_data_out, 0);}
    alt_u8 readY = READ_Y_AXIS;
    alt_16 yData;


    alt_u8 cam_set = 0;
    // buffer to hold SPI data before moving to SDRAM
    static uint8_t image_buffer[IM_SIZE];
    int framenum = 0;

    // clear edge capture register
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_BASE, 0);
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(SW_BASE, 0);
    // enable interrupt for key 0 (bitmask depends on your setup)
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEY_BASE, 0x1);
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(SW_BASE, 0x3FF);
    // set wire to 0
    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE, 0);
    //set shared sdram flag to 0

    //id interrupt controller, irq number, name of isr function, context, flag
    alt_ic_isr_register(KEY_IRQ_INTERRUPT_CONTROLLER_ID,KEY_IRQ,key_isr,NULL,0x0);
    alt_ic_isr_register(SW_IRQ_INTERRUPT_CONTROLLER_ID,SW_IRQ,sw_isr,NULL,0x0);

    while (1) {
        // Read Gyro (Processor 1 owns SPI)
		//alt_avalon_spi_command(SPI_0_BASE, 0, 1, &readX, 2, (alt_u8*)&xData, 0); // have to cast to u8 because otherwise warning even though acceleromter sends 16 bits
		alt_avalon_spi_command(SPI_0_BASE, 0, 1, &readY, 2, (alt_u8*)&yData, 0);
		//alt_avalon_spi_command(SPI_0_BASE, 0, 1, &readZ, 2, (alt_u8*)&zData, 0);
		//printf("X axis: %4d\t Y axis: %4d\t Z axis %4d\n", xData, yData, zData);
		//printf("Y axis: %4d\n",yData);




    	if (quadmode == 1){// if quad mode then accelerometer can send interrupts
    		if (yData > 75) {
    			rotation = (rotation + 1) % 4;
    			altera_avalon_mutex_lock(mutex, 1);
    			IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 2 , rotation); //set rotation in sdram
    			IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 5 , 1); //set rotation contol to 1
    			altera_avalon_mutex_unlock(mutex);
    			//send interrupt to proc0
        	    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE , 0x1);
        	    printf("sent rotation interrupt \n");
        	    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE, 0);
    		}else if (yData < -75) {
    			rotation = (rotation + 3) % 4; // -1 mod 4
    			altera_avalon_mutex_lock(mutex, 1);
    			IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 2 , rotation);
    			IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 5 , 1); //set rotation contol to 1
    			altera_avalon_mutex_unlock(mutex);
    			//send interrupt to proc0
        	    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE , 0x1);
        	    printf("sent rotation interrupt \n");
        	    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE, 0);
    		}
    	    cam_set = 18; // default settings except downscaled & "settings changed" = true
    	    alt_avalon_spi_command(SPI_0_BASE, 1, 1, &cam_set, IM_SIZEsmll, image_buffer, 0);
    	}else{
    		cam_set = 16;
    		alt_avalon_spi_command(SPI_0_BASE, 1, 1, &cam_set, IM_SIZE, image_buffer, 0);
    	}
    	if(switchmoved == 1){
    		switchvalue = IORD_ALTERA_AVALON_PIO_DATA(SW_BASE);
    		altera_avalon_mutex_lock(mutex, 1)
			IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 3 , switchvalue); //set rotation in sdram
			IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 6 , 1); //set rotation contol to 1
			altera_avalon_mutex_unlock(mutex);
    	    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE , 0x1);
    	    printf("sent SWITCH interrupt \n");
    	    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE, 0);
    		switchmoved = 0;
    	}

        // 3. Critical Section: Copy to Shared Mailbox
        //altera_avalon_mutex_lock(mutex, 1);
        //printf("just locked mutex \n");
        //if (altera_avalon_mutex_is_mine(mutex) || altera_avalon_mutex_lock(mutex, 1) == 0) {
        for (int i = 0; i < IM_SIZE; i++) {
        	//#define IOWR_8DIRECT(BASE, OFFSET, DATA)

            IOWR_8DIRECT(SHARED_MAILBOX_BASE, i, image_buffer[i]);
            // each image_buffer[i] is a byte, each 320 bytes is a row
        }
        framenum += 1;
        //altera_avalon_mutex_unlock(mutex);
        //printf("just unlocked mutex \n");
        //for (volatile int i = 0; i<500000; i++){}
        //printf("sent frame %d to shared memory in sdram first pixel value: %d \n", framenum, image_buffer[0]);

    	if (keypressed == 1){
    		altera_avalon_mutex_lock(mutex, 1);
    		//IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE - 1 , quadmode);//write quadmode to sdram
    		IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 1 , quadmode);


       	    //IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE - 4, 1);// set control to 1 to show interrupt is cause by key
       	    IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 4 , 1);
       	    altera_avalon_mutex_unlock(mutex);
       	    //send interrupt to proc 0
    	    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE , 0x1);
    	    //edge rise
    	    printf("sent key interrupt %d \n", quadmode);
    	    IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC1_BASE, 0);

    	    keypressed = 0;

    	}

    }
    return 0;
}
