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
#include "altera_avalon_pio_regs.h"

int main()
{
	#define IM_SIZE (240 * 320)
	char cam_set = 0;
	char cam_data;

	char image_buffer[IM_SIZE];

	int start_time, end_time, duration;
	float fps;

	for (int i=0; i<IM_SIZE; i++){
			  IOWR(PIX_BUFF_ADDR_BASE, 0, i);
			  IOWR(PIX_DATA_BASE, 0, 0);
		  }

	while(1) {
			start_time = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE);

			alt_avalon_spi_command(SPI_0_BASE, 1, 1, &cam_set, IM_SIZE, &image_buffer, 0);


	  for (int i=0; i<240; i++){
		  for (int j=0; j<320; j++){
		  IOWR(PIX_BUFF_ADDR_BASE, 0, i*320+j);
		  IOWR(PIX_DATA_BASE, 0, image_buffer[320*i+319-j-1]>>4);
	  }
	  end_time = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE);

	  duration = end_time - start_time;
	  printf("useconds : %d", duration);
	}
	}

  return 0;
}
