// C code to be run on the processor, processing inputs and formatting outputs for VGA, Gyroscope, and HEX Display.
// Created By : Group A06 - Timothy Cocks, Hugo Knights-Burke, Marco Carrera, Flynn Kennedy
// Created Date: 17/04
// version ='1.0'

#include <stdio.h>
#include <stdint.h>
#include "system.h"
#include "io.h"
#include <altera_avalon_spi.h>
#include <altera_avalon_spi_regs.h>
#include <altera_avalon_pio_regs.h>

void display_fps(int duration); // declare prototype above so no "implicit declaration" warnings, not needed but i dont like warnings

int main()
{
	#define IM_SIZE (240 * 320)
	char cam_set = 0;

	char image_buffer[IM_SIZE];

	int start_time, end_time, duration;

	// set them all 0 to init
	for (int i=0; i<IM_SIZE; i++){
		IOWR(PIX_BUFF_ADDR_BASE, 0, i);
		IOWR(PIX_DATA_BASE, 0, 0);
	}

	// Set other 3 hex to blank since dont need them yet:
	IOWR_32DIRECT(HEX2_BASE, 0, 0xFFFFFF);

	while(1) {
		start_time = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE); // gets start time from counter

		// gets frame from espcam and puts it in image sdram
		alt_avalon_spi_command(SPI_0_BASE, 1, 1, &cam_set, IM_SIZE, &image_buffer, 0);

		// iterate through all of the frames and write the pixel data to pixel buffer
		for (int i=0; i<240; i++){
			for (int j=0; j<320; j++){
				IOWR(PIX_BUFF_ADDR_BASE, 0, i*320+j);
				IOWR(PIX_DATA_BASE, 0, image_buffer[320*i+319-j-1]>>4);
			}

		}
		end_time = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE); // gets end time from counter

		duration = end_time - start_time; // gets the time since frame started to frame finished

		display_fps(duration); // call function to calculate & display the fps on hex
	}

  return 0;
}

// function for displaying hex
void display_fps(int duration) {
	uint8_t hex_lookup[] = { // map numbers to corresponding 7 seg bits being on (hex is like inverse, so 1 = off) from 2072 lol
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
	fps = 1000000.0 / duration; // convert us to s

	// REMOVE THIS WHEN INTERRUP WORKING, NOT NEEDED
	printf("FPS: %.2f\n", fps); // print to console the calculated fps to 2 decimal places

	/*
	 * From Quartus Code:
	 * output	[7:0] HEX0,
	 * ... until HEX5
	 *
	 * and
	 * .hex1_export_export          ({HEX2,HEX1,HEX0}),
	 * .hex2_export_export          ({HEX5,HEX4,HEX3}),
	 *
	 *  NIOS II system.h:
	 *  HEX1_BASE = right 3 displays
	 *  HEX2_BASE = left 3 displays (shouldnt need)
	 *
	 */

	int digit2 = (int)fps; 						// ones
	int digit1 = (int)(fps * 10.0) % 10;  		// tenths
	int digit0 = (int)(int)(fps * 100.0) % 10; 	// hundredths

	// write FPS to respective displays:
	uint8_t display2 = hex_lookup[digit2];
	uint8_t display1 = hex_lookup[digit1];
	uint8_t display0 = hex_lookup[digit0];

	// add decimal point to ones digit by anding with 8th bit as 0
	display2 = display2 & 0b01111111;

	// Combine the 3 bytes into one value to send over HEX1_BASE
	int hex_data = (display2 << 16) | (display1 << 8) | display0;

	// Write to HEX1_BASE (Right 3 displays)
	IOWR_32DIRECT(HEX1_BASE, 0, hex_data);
}
