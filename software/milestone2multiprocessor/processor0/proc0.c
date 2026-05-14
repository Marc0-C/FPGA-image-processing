#include <stdio.h>
#include "sys/alt_stdio.h"
#include <stdint.h>
#include "system.h"
#include "io.h"
#include "altera_avalon_mutex.h"
#include "altera_avalon_pio_regs.h"
#include <stdint.h>
#include <stdlib.h>
#include "sys/alt_irq.h"


//#define SHARED_MAILBOX_BASE 0x03FFF000
//#define SHARED_MAILBOX_BASE 0x03312D20
#define SHARED_MAILBOX_BASE 0x029896A0//actual mailbox starts at 0x02895460 but its reserved for flags
// -1 KEY, -2 ROTATE, -3 SWITCH, -4,-5,-6: CONTROL key, rot and switch
#define MAILBOX_BASE_PRINT 0x017A1220

#define IM_SIZE (240 * 320)
#define IM_SIZEsmll (120 * 160)
//#define IM_SIZE (10)

void display_fps(int duration);
volatile int newframe = 0;
volatile int flagg = 0;
volatile int key = 0;
int rot = 0;
int sw = 0;

int top_left = 0;
int top_right = 0;
int bottom_left = 0;
int bottom_right = 0;
int rot_top_left = 0;
int rot_top_right = 0;
int rot_bottom_left = 0;
int rot_bottom_right = 0;




// ISR function for KEY PRESSES
void key_isr(void* context)
{
	flagg = 1; //tell rest of program interrupt has been called
	// Clear edge capture register to acknowledge interrupt
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(IRQ_TO_PROC0_BASE, 0);

}

// ISR function for SWITCHES

//use later on
void pio_isr_proc0(void* context) {
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(IRQ_TO_PROC0_BASE, 0x0);
    //modify flag
    printf("$");
}

//SET IRQ CONTROLLER
//alt_ic_isr_register(IRQ_TO_PROC0_IRQ_INTERRUPT_CONTROLLER_ID, IRQ_TO_PROC0_IRQ, pio_isr_proc0, NULL, NULL);
//IOWR_ALTERA_AVALON_PIO_IRQ_MASK(IRQ_FROM_PROC0_BASE, 0x1); // Enable IRQ

//TO RECIEVE
//while (flag == 0){dowhatever};
//flag = 0;

//TO SEND
//IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC0_BASE, 1); // rise
//wait();
//IOWR_ALTERA_AVALON_PIO_DATA(IRQ_FROM_PROC0_BASE, 0); // Pulse low


int main() {
    //printf("processor0\n");
    alt_mutex_dev* mutex = altera_avalon_mutex_open(MUTEX_0_NAME);
    int start_time, end_time, duration;
    static uint8_t processing_buffer[IM_SIZE];
    // Initialise Displays
    IOWR_32DIRECT(HEX2_BASE, 0, 0xFFFFFF);
    int first_pixelvalue = 0;
    int last_pixelvalue = 0;

    // FOR INTERRUPTS:


    // clear edge capture register
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(IRQ_TO_PROC0_BASE, 0);

    // enable interrupt for key 0 (bitmask depends on your setup)
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(IRQ_TO_PROC0_BASE, 0x1);

    // register ISR
    alt_ic_isr_register(IRQ_TO_PROC0_IRQ_INTERRUPT_CONTROLLER_ID, IRQ_TO_PROC0_IRQ, key_isr,NULL,0x0 );


    while (1) {

    	if (flagg == 1){ // interrupt has been called, check what caused it: key, switch or rotation
    		if(IORD_8DIRECT(SHARED_MAILBOX_BASE, - 4) == 1){ // if interrupt caused by key
    			altera_avalon_mutex_lock(mutex, 1);
    			//key = IORD_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-1); // save key value
    			key = IORD_8DIRECT(SHARED_MAILBOX_BASE, -1);
    			printf("key interrupt received: %d \n", key);
    			//IOWR_ALTERA_AVALON_PIO_DATA(SHARED_MAILBOX_BASE-4, 0); // set key control to 0
    			IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 4 , 0);
    			altera_avalon_mutex_unlock(mutex);
    		}
    		if(IORD_8DIRECT(SHARED_MAILBOX_BASE,-5) == 1){ // interrupt caused by rotation
    			altera_avalon_mutex_lock(mutex, 1);
    			rot = IORD_8DIRECT(SHARED_MAILBOX_BASE,-2); //save rotation value
    			printf("rotation interrupt received: %d \n", rot);
    			IOWR_8DIRECT(SHARED_MAILBOX_BASE, -5, 0); // set rotaiton control to 0
    			altera_avalon_mutex_unlock(mutex);
    		}
    		if(IORD_8DIRECT(SHARED_MAILBOX_BASE,-6) == 1){ // interrupt caused by switches
    			altera_avalon_mutex_lock(mutex, 1);
    			sw = IORD_8DIRECT(SHARED_MAILBOX_BASE,-3); //save switch value
    			printf("switch interrupt received: %d \n", sw);
    			IOWR_8DIRECT(SHARED_MAILBOX_BASE,-6, 0); // set switch control to 0
    			altera_avalon_mutex_unlock(mutex);
    			top_left = (sw >> 0) & 0x3;  // SW[1:0]
    			top_right = (sw >> 2) & 0x3;  // SW[3:2]
    			bottom_left = (sw >> 4) & 0x3;  // SW[5:4]
    			bottom_right = (sw >> 6) & 0x3;  // SW[7:6]
    		}

    		flagg = 0;
    	}



        start_time = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE);
        // get sdram frame to buffer
		//altera_avalon_mutex_lock(mutex, 1);
        for (int i = 0; i < IM_SIZE; i++) {
    	// base, offset, copy all pixels to processing_buffer
    	processing_buffer[i] = IORD_8DIRECT(SHARED_MAILBOX_BASE, i);
		            }
		//altera_avalon_mutex_unlock(mutex);

        // Processing & VGA Output
        // loop through rows and columns

		if(key == 0){
			//apply switch functionality for not quadrature
						//sw = 0
			//printf("topleft, %d \n", top_left);
			if (top_left == 0){display_img(320, 240, 0, 0, processing_buffer);}
			if (top_left == 1){display_imgwhite(320, 240, 0, 0, processing_buffer);}
			if (top_left == 2){display_imgblack(320, 240, 0, 0, processing_buffer);}
			if (top_left == 3){display_imggrey(320, 240, 0, 0, processing_buffer);}


		}else{ //quad mode on


			 			// apply rotation mapping
			if (rot == 0) {
			    rot_top_left     = top_left;
			    rot_top_right    = top_right;
			    rot_bottom_left  = bottom_left;
			    rot_bottom_right = bottom_right;
			}
			else if (rot == 1) {
			    rot_top_left     = bottom_left;
			    rot_top_right    = top_left;
			    rot_bottom_left  = bottom_right;
			    rot_bottom_right = top_right;
			}
			else if (rot == 2) {
			    rot_top_left     = bottom_right;
			    rot_top_right    = bottom_left;
			    rot_bottom_left  = top_right;
			    rot_bottom_right = top_left;
			}
			else if (rot == 3) {
			    rot_top_left     = top_right;
			    rot_top_right    = bottom_right;
			    rot_bottom_left  = top_left;
			    rot_bottom_right = bottom_left;
			}
			if (rot_top_left == 0) {display_img(160, 120, 0, 0, processing_buffer, 0);}
			if (rot_top_left == 1) {display_imgwhite(160, 120, 0, 0, processing_buffer, 0);}
			if (rot_top_left == 2) {display_imgblack(160, 120, 0, 0, processing_buffer, 0);}
			if (rot_top_left == 3) {display_imggrey(160, 120, 0, 0, processing_buffer, 0);}

			if (rot_top_right == 0) {display_img(160, 120, 160, 0, processing_buffer, 0);}
			if (rot_top_right == 1) {display_imgwhite(160, 120, 160, 0, processing_buffer, 0);}
			if (rot_top_right == 2) {display_imgblack(160, 120, 160, 0, processing_buffer, 0);}
			if (rot_top_right == 3) {display_imggrey(160, 120, 160, 0, processing_buffer, 0);}

			if (rot_bottom_left == 0) {display_img(160, 120, 0, 120, processing_buffer, 0);}
			if (rot_bottom_left == 1) {display_imgwhite(160, 120, 0, 120, processing_buffer, 0);}
			if (rot_bottom_left == 2) {display_imgblack(160, 120, 0, 120, processing_buffer, 0);}
			if (rot_bottom_left == 3) {display_imggrey(160, 120, 0, 120, processing_buffer, 0);}

			if (rot_bottom_right == 0) {display_img(160, 120, 160, 120, processing_buffer, 0);}
			if (rot_bottom_right == 1) {display_imgwhite(160, 120, 160, 120, processing_buffer, 0);}
			if (rot_bottom_right == 2) {display_imgblack(160, 120, 160, 120, processing_buffer, 0);}
			if (rot_bottom_right == 3) {display_imggrey(160, 120, 160, 120, processing_buffer, 0);}


			//rot = 0

						//apply switch functionality for quadrature
			//sw = 0
			/*
			display_img(160, 120, 0, 0, processing_buffer);
			display_img(160, 120, 160, 0, processing_buffer);
			display_img(160, 120, 0, 120, processing_buffer);
			display_img(160, 120, 160, 120, processing_buffer);
			*/
		}

        end_time = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE);

        //wait for interrupt from p1 (new frame in sdram)?

        duration = end_time - start_time;
        display_fps(duration);

    }
    return 0;
}


void display_img(int width, int height, int startx, int starty, alt_u8 *processing_buffer)  {
    alt_u8 *img = processing_buffer;

    // only for debugging, weird black pixel check value of certain adresses
    //printf("TL: %d\n", buffer[0] >> 4);
    //printf("TR: %d\n", buffer[width - 1] >> 4);
    //printf("BL: %d\n", buffer[(height - 1) * width] >> 4);
    //printf("BR: %d\n", buffer[width * height - 1] >> 4);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int screen_x = startx + j;
            int screen_y = starty + i;

            //IOWR(PIX_BUFF_ADDR_BASE, 0, (screen_y * 320) + screen_x);
            //IOWR(PIX_DATA_BASE, 0, img[i * width + j - 1] >> 4); // bit shift by 4 coz 8 bit value only have 4 bits colour
            int location = (screen_y * 320) + screen_x;
            int pixel = img[i * width + j - 1] >> 4;
            IOWR_8DIRECT(MAILBOX_BASE_PRINT, location , pixel);

        }
    }
}
void display_imgwhite(int width, int height, int startx, int starty, alt_u8 *processing_buffer)  {
    alt_u8 *img = processing_buffer;

    // only for debugging, weird black pixel check value of certain adresses
    //printf("TL: %d\n", buffer[0] >> 4);
    //printf("TR: %d\n", buffer[width - 1] >> 4);
    //printf("BL: %d\n", buffer[(height - 1) * width] >> 4);
    //printf("BR: %d\n", buffer[width * height - 1] >> 4);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int screen_x = startx + j;
            int screen_y = starty + i;

            //IOWR(PIX_BUFF_ADDR_BASE, 0, (screen_y * 320) + screen_x);
            // 0xF is the 4-bit value for white (all bits high)
            //IOWR(PIX_DATA_BASE, 0, 0xF);
            int location = (screen_y * 320) + screen_x;
            int pixel = 0xF;
            IOWR_8DIRECT(MAILBOX_BASE_PRINT, location , pixel);

        }
    }
}
void display_imgblack(int width, int height, int startx, int starty, alt_u8 *processing_buffer)  {
    alt_u8 *img = processing_buffer;

    // only for debugging, weird black pixel check value of certain adresses
    //printf("TL: %d\n", buffer[0] >> 4);
    //printf("TR: %d\n", buffer[width - 1] >> 4);
    //printf("BL: %d\n", buffer[(height - 1) * width] >> 4);
    //printf("BR: %d\n", buffer[width * height - 1] >> 4);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int screen_x = startx + j;
            int screen_y = starty + i;

            //IOWR(PIX_BUFF_ADDR_BASE, 0, (screen_y * 320) + screen_x);
            // 0xF is the 4-bit value for white (all bits high)
            //IOWR(PIX_DATA_BASE, 0, 0x0);
            int location = (screen_y * 320) + screen_x;
            int pixel = 0xF;
            IOWR_8DIRECT(MAILBOX_BASE_PRINT, location , 0x0);

        }
    }
}
void display_imggrey(int width, int height, int startx, int starty, alt_u8 *processing_buffer)  {
    alt_u8 *img = processing_buffer;

    // only for debugging, weird black pixel check value of certain adresses
    //printf("TL: %d\n", buffer[0] >> 4);
    //printf("TR: %d\n", buffer[width - 1] >> 4);
    //printf("BL: %d\n", buffer[(height - 1) * width] >> 4);
    //printf("BR: %d\n", buffer[width * height - 1] >> 4);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int screen_x = startx + j;
            int screen_y = starty + i;

            //IOWR(PIX_BUFF_ADDR_BASE, 0, (screen_y * 320) + screen_x);
            // 0xF is the 4-bit value for white (all bits high)
            //IOWR(PIX_DATA_BASE, 0, 0x8);
            int location = (screen_y * 320) + screen_x;
            int pixel = 0xF;
            IOWR_8DIRECT(MAILBOX_BASE_PRINT, location , 0x8);

        }
    }



}


void display_fps(int duration) {
    uint8_t hex_lookup[] = {0b11000000, 0b11111001, 0b10100100, 0b10110000, 0b10011001,
                            0b10010010, 0b10000010, 0b11111000, 0b10000000, 0b10010000};
    if (duration == 0) return;
    float fps = 1000000.0 / duration;
    int d3 = ((int)fps / 10) % 10;
    int d2 = (int)fps % 10;
    int d1 = (int)(fps * 10.0) % 10;
    int d0 = (int)(fps * 100.0) % 10;
    //int hex_data = ((hex_lookup[d2] & 0x7F) << 16) | (hex_lookup[d1] << 8) | hex_lookup[d0];
    int hex_data = (hex_lookup[d3] << 16) | ((hex_lookup[d2] & 0x7F) << 8) | hex_lookup[d1];
    IOWR_32DIRECT(HEX1_BASE, 0, hex_data);
}
