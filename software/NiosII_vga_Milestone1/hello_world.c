/* C code to be run on the processor, processing inputs and formatting outputs for VGA, Gyroscope, and HEX Display.
 * Created By : Group A06 - Timothy Cocks, Hugo Knights-Burke, Marco Carrera, Flynn Kennedy
 * Created Date: 17/04
 * version ='1.0'
 */

#include <stdio.h>
#include <stdint.h>
#include "system.h"
#include "io.h"
#include <altera_avalon_spi.h>
#include <altera_avalon_spi_regs.h>
#include <altera_avalon_pio_regs.h>

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


#define IM_SIZE (240 * 320)

void display_fps(int duration); // declare prototype above so no "implicit declaration" warnings, not needed but i dont like warnings

void display_img(int width, int height, int startx, int starty, alt_u8 buffer[IM_SIZE]); // prototype to display images from buffer

int main()
{
    printf("Hello from Nios II!\n"); // basic print statement to ensure UART is working
    // ---- Initialize Gyro ----

    alt_u8 gyro_data_out; // variable to store output data


    for (int i = 0; i < CONFIG_LENGHT; i += 2) // load config with address then value
    {
        alt_avalon_spi_command(SPI_0_BASE, 0, 2, gyro_config + i, 0, &gyro_data_out, 0);
    }


    alt_16 xData;    // halfwords for the orientation data
    alt_16 yData;
    alt_16 zData;

    // read locations
    alt_u8 readX = READ_X_AXIS;
    alt_u8 readY = READ_Y_AXIS;
    alt_u8 readZ = READ_Z_AXIS;

    // camera stuff
	alt_u8 cam_set = 18; // binary 000 10010 cam settings

	alt_u8 image_buffer[IM_SIZE];

	int start_time, end_time, duration;

	// set them all 0 to init
	for (int i=0; i<IM_SIZE; i++){
		IOWR(PIX_BUFF_ADDR_BASE, 0, i);
		IOWR(PIX_DATA_BASE, 0, 0);
	}

	// Set other 3 hex to blank since dont need them yet:
	IOWR_32DIRECT(HEX2_BASE, 0, 0xFFFFFF);

    while (1)
    {
    	start_time = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE); // gets start time from counter

		alt_avalon_spi_command(SPI_0_BASE, 0, 1, &readX, 2, &xData, 0);

		alt_avalon_spi_command(SPI_0_BASE, 0, 1, &readY, 2, &yData, 0);

		alt_avalon_spi_command(SPI_0_BASE, 0, 1, &readZ, 2, &zData, 0);

		printf("X axis: %4d\t Y axis: %4d\t Z axis %4d\n", xData, yData, zData);





		if (IORD_ALTERA_AVALON_PIO_DATA(KEY_BASE)) {
			printf("SW\n");
			cam_set = 18; // default settings except downscaled & "settings changed" = true

			alt_avalon_spi_command(SPI_0_BASE, 1, 1, &cam_set, IM_SIZE, &image_buffer, 0);

			// image width, image height, startx, starty, buffer
			display_img(160, 120, 0, 0, image_buffer);

			display_img(160, 120, 160, 0, image_buffer); // draw second frame next to first

			display_img(160, 120, 0, 120, image_buffer);

			display_img(160, 120, 160, 120, image_buffer);


		} else {
			printf("NO SW\n");
			cam_set = 16; // default settings except "settings changed" = true

			alt_avalon_spi_command(SPI_0_BASE, 1, 1, &cam_set, IM_SIZE, &image_buffer, 0);

			// image width, image height, startx, starty, buffer
			display_img(320, 240, 0, 0, image_buffer);

		}

		end_time = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE); // gets end time from counter

		duration = end_time - start_time; // gets the time since frame started to frame finished

		display_fps(duration); // call function to calculate & display the fps on hex


    }

    return 0;
}

void display_img(int width, int height, int startx, int starty, alt_u8 buffer[IM_SIZE]) {
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int screen_x = startx + j;
			int screen_y = starty + i;

			IOWR(PIX_BUFF_ADDR_BASE, 0, (screen_y * 320) + screen_x);
			IOWR(PIX_DATA_BASE, 0, buffer[i * width + j] >> 4);
		}
	}
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
	int digit0 = (int)(fps * 100.0) % 10; 	// hundredths

	// write FPS to respective displays:
	uint8_t display2 = hex_lookup[digit2];
	uint8_t display1 = hex_lookup[digit1];
	uint8_t display0 = hex_lookup[digit0];

	// add decimal point to ones digit by anding with 8th bit as 0
	display2 = display2 & 0b01111111;

	// Combine the 3 bytes into one value to send over HEX1_BASE
	int hex_data = (display2 << 16) | (display1 << 8) | display0;

	// Write to HEX1_BASE (Right 3 displays)
	IOWR(HEX1_BASE, 0, hex_data);
}
