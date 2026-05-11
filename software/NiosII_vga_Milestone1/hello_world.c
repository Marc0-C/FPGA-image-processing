/* C code to be run on the processor, processing inputs and formatting outputs for VGA, Gyroscope, and HEX Display.
 * Created By : Group A06 - Timothy Cocks, Hugo Knights-Burke, Marco Carrera, Flynn Kennedy
 * Created Date: 17/04
 * version ='1.0'
 */

#include <stdio.h>
#include "system.h"
#include "io.h"
#include <altera_avalon_spi.h>
#include <altera_avalon_spi_regs.h>
#include <altera_avalon_pio_regs.h>

//for interrupts
#include "sys/alt_irq.h"

volatile int quad_mode = 0; // 0 = single, 1 = quad. volatile because changes outside of main loop
volatile int lastEdge = 0;


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
#define IM_SIZE_SMOL (120 * 160)
#define THRESHOLD 75 // for accelerometer rotation to rotate frames in quad mode

void display_fps(int duration); // declare prototype above so no "implicit declaration" warnings, not needed but i dont like warnings

void display_img(int width, int height, int startx, int starty, alt_u8 buffer[IM_SIZE]); // prototype to display images from buffer

// ISR function for KEY PRESSES
void key_isr(void* context, alt_u32 id)
{
    volatile int* edge_capture_ptr = (volatile int*) context;
    int edge = *edge_capture_ptr;

    // clear interrupt FIRST
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_BASE, 0);

    if (edge & 0x1) {
        quad_mode = !quad_mode;
    }
    printf("Interrupt KEY Pressed\n");
}

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
	alt_u8 cam_set = 22; // binary 0001 0110 cam settings

	alt_u8 image_buffer[IM_SIZE/2];
	alt_u8 special_image[IM_SIZE/2];
	int *flip_quad_image = &special_image+IM_SIZE_SMOL/2;
	int *blur_quad_image = &special_image+IM_SIZE_SMOL;
	int *edge_quad_image = &special_image+IM_SIZE_SMOL*3/2;

	int start_time, end_time, duration;

	//int quad_mode = 0; // 0 = single mode, 1 = quad mode // moved to above because of interrupt now
	int rotation = 0; // keeps track of acceleromter frame rotation for quad mode

	// for rotated versions quad mode
	int mode_top_left    = 0;
	int mode_top_right   = 0;
	int mode_bottom_left = 0;
	int mode_bottom_right= 0;


	// set them all 0 to init
	for (int i=0; i<IM_SIZE; i++){
		IOWR(PIX_BUFF_ADDR_BASE, 0, i);
		IOWR(PIX_DATA_BASE, 0, 0);
	}

	// Set other 3 hex to blank since dont need them yet:
	IOWR_32DIRECT(HEX2_BASE, 0, 0xFFFFFF);


	// FOR INTERRUPTS:

	void* edge_capture_ptr = (void*)0;

	    // clear edge capture register
	    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY_BASE, 0);

	    // enable interrupt for key 0 (bitmask depends on your setup)
	    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEY_BASE, 0x1);

	    // register ISR
	    alt_ic_isr_register(
	        KEY_IRQ_INTERRUPT_CONTROLLER_ID,
	        KEY_IRQ,
	        key_isr,
	        edge_capture_ptr,
	        0x0
	    );

    while (1)
    {
    	start_time = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE); // gets start time from counter

		alt_avalon_spi_command(SPI_0_BASE, 0, 1, &readX, 2, (alt_u8*)&xData, 0); // have to cast to u8 because otherwise warning even though acceleromter sends 16 bits

		alt_avalon_spi_command(SPI_0_BASE, 0, 1, &readY, 2, (alt_u8*)&yData, 0);

		alt_avalon_spi_command(SPI_0_BASE, 0, 1, &readZ, 2, (alt_u8*)&zData, 0);

		//printf("X axis: %4d\t Y axis: %4d\t Z axis %4d\n", xData, yData, zData);

		int sw = IORD_ALTERA_AVALON_PIO_DATA(SW_BASE);

		int sw_top_left = (sw >> 0) & 0x3;  // SW[1:0]
		int sw_top_right = (sw >> 2) & 0x3;  // SW[3:2]
		int sw_bottom_left = (sw >> 4) & 0x3;  // SW[5:4]
		int sw_bottom_right = (sw >> 6) & 0x3;  // SW[7:6]


		// debugging prints out the state of SW that correspond to the modes yep.
		//printf("TL: %d | TR: %d | BL: %d | BR: %d\n", top_left, top_right, bottom_left, bottom_right);


		// debugging prints out the state of SW that correspond to the modes yep.
		//printf("TL: %d | TR: %d | BL: %d | BR: %d\n", top_left, top_right, bottom_left, bottom_right);

		//toggle the quad and single modes

		/*if ( IORD_ALTERA_AVALON_PIO_DATA(KEY_BASE) == 0) {
			printf("KEY Pressed\n");
			if (quad_mode) {
				quad_mode = 0;
			} else {
				quad_mode = 1;
			}
		}*/

		if (quad_mode) {
			cam_set = 22; // default settings except downscaled & "settings changed" = true

			alt_avalon_spi_command(SPI_0_BASE, 1, 1, &cam_set, IM_SIZE_SMOL/2, image_buffer, 0);

			// ROTATE with Accelerometer Y Axis.
			// Y-axis rotation control
			if (yData > 75) {
			    rotation = (rotation + 1) % 4;
			    printf("Rotated CW\n");
			}
			else if (yData < -75) {
			    rotation = (rotation + 3) % 4; // -1 mod 4
			    printf("Rotated CCW\n");
			}

			// change to be if any quadrant is mode x,
			// then call function for corresponding mode
			// otherwise if dont need that image anywhere dont run that function




			if ((mode_top_left == 1 || mode_top_right == 1 || mode_bottom_left == 1 || mode_bottom_right == 1)) {
			    // image 1
				flip_image(image_buffer, flip_quad_image, 1, 1);
			}
			if ((mode_top_left == 2 || mode_top_right == 2 || mode_bottom_left == 2 || mode_bottom_right == 2)) {
			    // image 2
				blur(image_buffer, blur_quad_image, 1);

			}
			if ((mode_top_left == 3 || mode_top_right == 3 || mode_bottom_left == 3 || mode_bottom_right == 3)) {
			    // image 3
				edge_detector(image_buffer, edge_quad_image, 1);
			}



			// apply rotation mapping:

			// mode = actual mode to be displayed on each quadrant, based on
			// -- the raw sw data corresponding to each quadrant
			// -- the rotation being applied to the quadrants

			if (rotation == 0) {
			    mode_top_left     = sw_top_left;
			    mode_top_right    = sw_top_right;
			    mode_bottom_left  = sw_bottom_left;
			    mode_bottom_right = sw_bottom_right;
			}
			else if (rotation == 1) {
			    mode_top_left     = sw_bottom_left;
			    mode_top_right    = sw_top_left;
			    mode_bottom_left  = sw_bottom_right;
			    mode_bottom_right = sw_top_right;
			}
			else if (rotation == 2) {
			    mode_top_left     = sw_bottom_right;
			    mode_top_right    = sw_bottom_left;
			    mode_bottom_left  = sw_top_right;
			    mode_bottom_right = sw_top_left;
			}
			else if (rotation == 3) {
			    mode_top_left     = sw_top_right;
			    mode_top_right    = sw_bottom_right;
			    mode_bottom_left  = sw_top_left;
			    mode_bottom_right = sw_bottom_left;
			}



			// actually display the images in each quadrant

			// TOP LEFT
			if (mode_top_left == 0) {
			    display_img(160, 120, 0, 0, image_buffer);
			}
			else if (mode_top_left == 1) {
			    // image 1
				display_img(160, 120, 0, 0, flip_quad_image);

			}
			else if (mode_top_left == 2) {
			    // image 2
				display_img(160, 120, 0, 0, blur_quad_image);
			}
			else if (mode_top_left == 3) {
			    // image 3
				display_img(160, 120, 0, 0, edge_quad_image);
			}


			// TOP RIGHT
			if (mode_top_right == 0) {
			    display_img(160, 120, 160, 0, image_buffer);
			}
			else if (mode_top_right == 1) {
			    // display image 1
				display_img(160, 120, 160, 0, flip_quad_image);
			}
			else if (mode_top_right == 2) {
			    // display image 2
				display_img(160, 120, 160, 0, blur_quad_image);
			}
			else if (mode_top_right == 3) {
			    // display image 3
				display_img(160, 120, 160, 0, edge_quad_image);
			}


			// BOTTOM LEFT
			if (mode_bottom_left == 0) {
			    display_img(160, 120, 0, 120, image_buffer);
			}
			else if (mode_bottom_left == 1) {
			    // display image 1
				display_img(160, 120, 0, 120, flip_quad_image);
			}
			else if (mode_bottom_left == 2) {
			    // display image 2
				display_img(160, 120, 0, 120, blur_quad_image);
			}
			else if (mode_bottom_left == 3) {
			    // display image 3
				display_img(160, 120, 0, 120, edge_quad_image);
			}


			// BOTTOM RIGHT
			if (mode_bottom_right == 0) {
			    display_img(160, 120, 160, 120, image_buffer);
			}
			else if (mode_bottom_right == 1) {
			    // display image 1
				display_img(160, 120, 160, 120, flip_quad_image);
			}
			else if (mode_bottom_right == 2) {
			    // display image 2
				display_img(160, 120, 160, 120, blur_quad_image);
			}
			else if (mode_bottom_right == 3) {
			    // display image 3
				display_img(160, 120, 160, 120, edge_quad_image);
			}


		} else { // SINGLE MODE
			cam_set = 20; // default settings except "settings changed" = true

			alt_avalon_spi_command(SPI_0_BASE, 1, 1, &cam_set, IM_SIZE/2, image_buffer, 0);

			if (sw_top_left == 0) {
				// display image 0
				// image width, image height, startx, starty, buffer
				display_img(320, 240, 0, 0, image_buffer);
			}
			else if (sw_top_left == 1) {
			    // display image 1

				// modifications args: input buffer, output buffer, axis(0/1), size(0=big / 1=small)
				flip_image(image_buffer, special_image, 1, 0);
				display_img(320, 240, 0, 0, special_image);
			}
			else if (sw_top_left == 2) {
			    // display image 2

				// modifications args: input buffer, output buffer, 0=big / 1=small
				blur(image_buffer, special_image, 0);
				display_img(320, 240, 0, 0, special_image);
			}
			else if (sw_top_left == 3) {
			    // display image 3
				edge_detector(image_buffer, special_image, 0);
				display_img(320, 240, 0, 0, special_image);
			}
		}

		end_time = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE); // gets end time from counter

		duration = end_time - start_time; // gets the time since frame started to frame finished

		display_fps(duration); // call function to calculate & display the fps on hex


    }

    return 0;
}

void display_img(int width, int height, int startx, int starty, alt_u8 *buffer) {
    alt_u8 *img = buffer;

    // only for debugging, weird black pixel check value of certain adresses
    //printf("TL: %d\n", buffer[0] >> 4);
    //printf("TR: %d\n", buffer[width - 1] >> 4);
    //printf("BL: %d\n", buffer[(height - 1) * width] >> 4);
    //printf("BR: %d\n", buffer[width * height - 1] >> 4);

    alt_u8 pix;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width/2; j++) {
            int screen_x = startx + 2*j;
            int screen_y = starty + i;

            pix = img[i * width/2 + j - 1];

            IOWR(PIX_BUFF_ADDR_BASE, 0, (screen_y * 320) + screen_x);
            IOWR(PIX_DATA_BASE, 0, pix >> 4); // bit shift by 4 coz 8 bit value only have 4 bits colour

            IOWR(PIX_BUFF_ADDR_BASE, 0, (screen_y * 320) + screen_x+1);
            IOWR(PIX_DATA_BASE, 0, pix); // bit shift by 4 coz 8 bit value only have 4 bits colour


        }
    }
}


// function for displaying hex
void display_fps(int duration) {
	alt_u8 hex_lookup[] = { // map numbers to corresponding 7 seg bits being on (hex is like inverse, so 1 = off) from 2072 lol
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
	alt_u8 display2 = hex_lookup[digit2];
	alt_u8 display1 = hex_lookup[digit1];
	alt_u8 display0 = hex_lookup[digit0];

	// add decimal point to ones digit by anding with 8th bit as 0
	display2 = display2 & 0b01111111;

	// Combine the 3 bytes into one value to send over HEX1_BASE
	int hex_data = (display2 << 16) | (display1 << 8) | display0;

	// Write to HEX1_BASE (Right 3 displays)
	IOWR(HEX1_BASE, 0, hex_data);
}
