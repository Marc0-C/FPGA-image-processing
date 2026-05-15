/*
   Processor 0, original from M1, takes the frames stored in sdram by processor 1
   Recieves inter-processor interrupts to check the flags in sdram for status of
   quadmode and rotation, also checks switches.
   Image processing, including flipping, blurring and edge detection are done here.
   Processed frames are stored in another shared space in sdram with processor 2.
*/
// Created By : Team A06
// Created Date: 15/05/2026
// version = Final

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

// Shared Memory between processor0 and processor1
#define SHARED_MAILBOX_BASE 0x029896A0
// Actual mailbox starts at 0x02895460
// 0x02895460 to 0x029896A0 is reserved for flags and other interprocessor data (excluding frames)

// Shared Memory between processor0 and processor2
#define MAILBOX_BASE_PRINT 0x017A1220

#define IM_SIZE ((240 * 320)*3/2)
#define IM_SIZE_QUAD ((120 * 160)*3/2)


void display_fps(int duration);
volatile int interruptFlag = 0;
volatile int quadMode = 0;
int rot = 0;
int sw = 0;

int swTopLeft = 0;
int swTopRight = 0;
int swBottomLeft = 0;
int swBottomRight = 0;
int rotTopLeft = 0;
int rotTopRight = 0;
int rotBottomLeft = 0;
int rotBottomRight = 0;


// ISR function for inter-processor interrupts
void proc1_isr(void* context)
{
	// Interrupt has been called interruptFlag
	interruptFlag = 1;
	// Clear edge capture register to acknowledge interrupt
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(IRQ_TO_PROC0_BASE, 0);
}

// Image processing function prototypes from image_filtering.c
void flip_image(alt_u8 *input_buffer, alt_u8 *output_buffer, int axis, int size);
void blur(alt_u8 *input_buffer, alt_u8 *output_buffer, int size);
void edge_detector(alt_u8 *input_buffer, alt_u8 *output_buffer, int size);


int main() {
    alt_mutex_dev* mutex = altera_avalon_mutex_open(MUTEX_0_NAME);
    int startTime, endTime, duration;

    // 2 full sized buffers, one for the normal image and quad mode,
    // and the other for a full sized filter image
    static uint8_t processingBuffer[IM_SIZE];
    static uint8_t specialImage[IM_SIZE];

    // Pointers to offset locations in processing buffer for quad images
    uint8_t *flipQuadImage = processingBuffer + IM_SIZE_QUAD;
    uint8_t *blurQuadImage = processingBuffer + IM_SIZE_QUAD * 2;
    uint8_t *edgeQuadImage = processingBuffer + IM_SIZE_QUAD * 3;

    // Initialise Displays
    IOWR_32DIRECT(HEX2_BASE, 0, 0xFFFFFF);
    int first_pixelvalue = 0;
    int last_pixelvalue = 0;

    // clear edge capture register
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(IRQ_TO_PROC0_BASE, 0);
    // enable interrupt for key 0
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(IRQ_TO_PROC0_BASE, 0x1);
    // register ISR
    alt_ic_isr_register(IRQ_TO_PROC0_IRQ_INTERRUPT_CONTROLLER_ID, IRQ_TO_PROC0_IRQ, proc1_isr,NULL,0x0 );

    while (1) {

    	int timeFlag = 0;
		int calculatingTime = 0;
		altera_avalon_mutex_lock(mutex, 1);
		timeFlag = IORD_8DIRECT(SHARED_MAILBOX_BASE, - 10);
		altera_avalon_mutex_unlock(mutex);

		if (timeFlag == 1){
			//endTime_prev = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE);

			altera_avalon_mutex_lock(mutex, 1);
			IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 10, 0);
			altera_avalon_mutex_unlock(mutex);


			if (interruptFlag == 1){
				// interrupt has been called, check what caused it: key, switch or rotation
				altera_avalon_mutex_lock(mutex, 1);

				// If interrupt caused by key
				if(IORD_8DIRECT(SHARED_MAILBOX_BASE, - 4) == 1){ // if interrupt caused by key
					// Save quadmode value
					quadMode = IORD_8DIRECT(SHARED_MAILBOX_BASE, -1);
					printf("key interrupt received: %d \n", quadMode);
					// Set key interrupt flag value to 0
					IOWR_8DIRECT(SHARED_MAILBOX_BASE, - 4 , 0);
				}

				// If interrupt caused by rotation
				if(IORD_8DIRECT(SHARED_MAILBOX_BASE,-5) == 1){
					// Save rotation value
					rot = IORD_8DIRECT(SHARED_MAILBOX_BASE,-2);
					printf("rotation interrupt received: %d \n", rot);
					// Set rotaiton interrupt flag to 0
					IOWR_8DIRECT(SHARED_MAILBOX_BASE, -5, 0);
				}
				// reset interrupt flag and unlock memory
				interruptFlag = 0;
				altera_avalon_mutex_unlock(mutex);
			}

			// assign switches
			int sw = IORD_ALTERA_AVALON_PIO_DATA(SW_BASE);

			int swTopLeft = (sw >> 0) & 0x3;  // SW[1:0]
			int swTopRight = (sw >> 2) & 0x3;  // SW[3:2]
			int swBottomLeft = (sw >> 4) & 0x3;  // SW[5:4]
			int swBottomRight = (sw >> 6) & 0x3;  // SW[7:6]


			// get sdram frame to buffer
			for (int i = 0; i < IM_SIZE; i++) {
				// base, offset, copy all pixels to processingBuffer
				processingBuffer[i] = IORD_8DIRECT(SHARED_MAILBOX_BASE, i);
			}

			if(quadMode == 0){
				//apply switch functionality for not quadrature
				if (swTopLeft == 0) {
					display_img(320, 240, 0, 0, processingBuffer);
				}
				if (swTopLeft == 1) {
					flip_image(processingBuffer, specialImage, 2, 0);
					display_img(320, 240, 0, 0, specialImage);
				}
				if (swTopLeft == 2) {
					blur(processingBuffer, specialImage, 0);
					display_img(320, 240, 0, 0, specialImage);
				}
				if (swTopLeft == 3) {
					edge_detector(processingBuffer, specialImage, 0);
					display_img(320, 240, 0, 0, specialImage);
				}

			} else {
				//quad mode on
				// apply rotation mapping
				if (rot == 0) {
					rotTopLeft     = swTopLeft;
					rotTopRight    = swTopRight;
					rotBottomLeft  = swBottomLeft;
					rotBottomRight = swBottomRight;
				}
				else if (rot == 1) {
					rotTopLeft     = swBottomLeft;
					rotTopRight    = swTopLeft;
					rotBottomLeft  = swBottomRight;
					rotBottomRight = swTopRight;
				}
				else if (rot == 2) {
					rotTopLeft     = swBottomRight;
					rotTopRight    = swBottomLeft;
					rotBottomLeft  = swTopRight;
					rotBottomRight = swTopLeft;
				}
				else if (rot == 3) {
					rotTopLeft     = swTopRight;
					rotTopRight    = swBottomRight;
					rotBottomLeft  = swTopLeft;
					rotBottomRight = swBottomLeft;
				}

				// only run filters if they are needed
				if (rotTopLeft == 1 || rotTopRight == 1 || rotBottomLeft == 1 || rotBottomRight == 1) {
					flip_image(processingBuffer, flipQuadImage, 2, 1);
				}
				if (rotTopLeft == 2 || rotTopRight == 2 || rotBottomLeft == 2 || rotBottomRight == 2) {
					blur(processingBuffer, blurQuadImage, 1);
				}
				if (rotTopLeft == 3 || rotTopRight == 3 || rotBottomLeft == 3 || rotBottomRight == 3) {
					edge_detector(processingBuffer, edgeQuadImage, 1);
				}

				// Display Top Left
				if (rotTopLeft == 0) {
					display_img(160, 120, 0, 0, processingBuffer);
				}
				if (rotTopLeft == 1) {
					display_img(160, 120, 0, 0, flipQuadImage);
				}
				if (rotTopLeft == 2) {
					display_img(160, 120, 0, 0, blurQuadImage);
				}
				if (rotTopLeft == 3) {
					display_img(160, 120, 0, 0, edgeQuadImage);
				}

				// Display Top Right
				if (rotTopRight == 0) {
					display_img(160, 120, 160, 0, processingBuffer);
				}
				if (rotTopRight == 1) {
					display_img(160, 120, 160, 0, flipQuadImage); }
				if (rotTopRight == 2) { display_img(160, 120, 160, 0, blurQuadImage);
				}
				if (rotTopRight == 3) {
					display_img(160, 120, 160, 0, edgeQuadImage);
				}

				// Display Bottom Left
				if (rotBottomLeft == 0) {
					display_img(160, 120, 0, 120, processingBuffer);
				}
				if (rotBottomLeft == 1) {
					display_img(160, 120, 0, 120, flipQuadImage);
				}
				if (rotBottomLeft == 2) {
					display_img(160, 120, 0, 120, blurQuadImage);
				}
				if (rotBottomLeft == 3) {
					display_img(160, 120, 0, 120, edgeQuadImage);
				}

				// Display Bottom Right
				if (rotBottomRight == 0) {
					display_img(160, 120, 160, 120, processingBuffer);
				}
				if (rotBottomRight == 1) {
					display_img(160, 120, 160, 120, flipQuadImage);
				}
				if (rotBottomRight == 2) {
					display_img(160, 120, 160, 120, blurQuadImage);
				}
				if (rotBottomRight == 3) {
					display_img(160, 120, 160, 120, edgeQuadImage);
				}

			}

			// calculate fps
			endTime = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE);
			duration = endTime - startTime;

			display_fps(duration);

			startTime = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE);
		}
    }
    return 0;
}

// Copy frames to sdram shared with processor 2
void display_img(int width, int height, int startX, int startY, alt_u8 *processingBuffer)  {
    alt_u8 *img = processingBuffer;

    // iterate through pixels
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width*3/2; j++) {
            int screenX = startX*3/2 + j;
            int screenY = startY + i;

            int location = (screenY * 320*3/2) + screenX;
			int pixel = img[i * width *3/2 + j];

			// load into shared sdram
			IOWR_8DIRECT(MAILBOX_BASE_PRINT, location , pixel);


        }
    }
}


// function for displaying hex
void display_fps(int duration) {
    uint8_t hexLookup[] = { // map numbers to corresponding 7 seg bits being on (hex is like inverse, so 1 = off) from 2072 lol
        //8th bit always 1 since that is the decimal point, toggle that manually later
        0b11000000, // 0
        0b11111001, // 1
        0b10100100, // 2
        0b10110000, // 3
        0b10011001, // 4
        0b10010010, // 5
        0b10000010, // 6
        0b11111000, // 7
        0b10000000, // 8
        0b10010000  // 9
    };

    float fps;
    fps = 100000.0 / duration;

    int value = (int)fps;

    int digit5 = 0;
    int digit4 = 0;


    int digit3 = (int)fps; // ones
    int digit2 = (int)(fps * 10.0) % 10; // tenths
    int digit1 = (int)(fps * 100.0) % 10; // hundredths
    int digit0 = (int)(fps * 1000.0) % 10;;


    // convert to 7seg
    uint8_t display5 = hexLookup[digit5];
    uint8_t display4 = hexLookup[digit4];
    uint8_t display3 = hexLookup[digit3];

    uint8_t display2 = hexLookup[digit2];
    uint8_t display1 = hexLookup[digit1];
    uint8_t display0 = hexLookup[digit0];

    display2 = display2 & 0b01111111;

    // Combine the 3 bytes into one value to send over HEX1_BASE
    int hexData1 = (display2 << 16) | (display1 << 8) | display0;

    int hexData2 = (display5 << 16) | (display4 << 8) | display3;

    //write to hex displays
    IOWR_32DIRECT(HEX1_BASE, 0, hexData1); // Write to HEX1_BASE (Right 3 displays)

    IOWR_32DIRECT(HEX2_BASE, 0, hexData2); // Write to HEX2_BASE (left 3 displays)

}
