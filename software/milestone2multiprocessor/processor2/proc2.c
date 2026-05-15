#include <stdio.h>
#include "sys/alt_stdio.h"
#include <stdint.h>
#include "system.h"
#include "io.h"
//#include "altera_avalon_mutex.h"
#include "altera_avalon_pio_regs.h"
#include <stdint.h>
#include <stdlib.h>
#include "sys/alt_irq.h"


#define MAILBOX_BASE_PRINT 0x017A1220

int frame = 0;
int main()
{
  printf("Processor 2\n");
  while(1){
		// iterate through pixels
	    for (int i = 0; i < ((320*240)/2); i++) {

			// calculate address
	    	int address = 3*i;

			// init colour variables ("low" and "high" bits of raw rgb data)
			alt_u8 l1, h1, l2;
			alt_u8 h2, l3, h3;

			

			// read and calculate pixel data for all 3 colours/bytes... didn't match background information diagram
			alt_u8 pix = IORD_8DIRECT(MAILBOX_BASE_PRINT, address);
			l1 = pix & 0xF;
			h1 = (pix >> 4) & 0xF; // shift by 4 since 2x 4bit packed in 8bit data type

			// next byte
			address++;
			pix = IORD_8DIRECT(MAILBOX_BASE_PRINT, address);
			l2 = pix & 0xF;
			h2 = (pix >> 4) & 0xF;

			// next byte
			address++;
			pix = IORD_8DIRECT(MAILBOX_BASE_PRINT, address);
			l3 = pix & 0xF;
			h3 = (pix >> 4) & 0xF;


			// write to vga controller
			IOWR(PIX_BUFF_ADDR_BASE, 0, 2*i+1);
			IOWR(PIX_DATA_R_BASE, 0, h1);//h1
			IOWR(PIX_DATA_G_BASE, 0, l1);//#l1
			IOWR(PIX_DATA_B_BASE, 0, h2);//h2

			IOWR(PIX_BUFF_ADDR_BASE, 0,2*i);
			IOWR(PIX_DATA_R_BASE, 0, l2);//l2
			IOWR(PIX_DATA_G_BASE, 0, h3);
			IOWR(PIX_DATA_B_BASE, 0, l3);


	    }

  }

  return 0;
}
