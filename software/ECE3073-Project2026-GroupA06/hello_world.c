/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include "system.h"
#include "io.h"
#include <altera_avalon_spi.h>
#include <altera_avalon_spi_regs.h>

int main()
{
	#define IM_SIZE (240 * 320)
	char cam_set = 0;
	char cam_data;

	char image_buffer[IM_SIZE];
	while(1) {

			alt_avalon_spi_command(SPI_0_BASE, 1, 1, &cam_set, IM_SIZE, &image_buffer, 0);


	  for (int i=0; i<IM_SIZE; i++){
		  IOWR(PIX_BUFF_ADDR_BASE, 0, i);
		  IOWR(PIX_DATA_BASE, 0, image_buffer[i]>>4);
	  }
	}

  return 0;
}
