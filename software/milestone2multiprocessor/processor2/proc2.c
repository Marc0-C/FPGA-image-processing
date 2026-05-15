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

	    for (int i = 0; i < ((320*240)/2); i++) {
	    	int addr = 3*i;

			alt_u8 l1, h1, l2;
			alt_u8 h2, l3, h3;

			alt_u8 pix = IORD_8DIRECT(MAILBOX_BASE_PRINT, addr);

			l1 = pix & 0xF;
			h1 = (pix >> 4) & 0xF;

			addr++;
			pix = IORD_8DIRECT(MAILBOX_BASE_PRINT, addr);
			l2 = pix & 0xF;
			h2 = (pix >> 4) & 0xF;

			addr++;
			pix = IORD_8DIRECT(MAILBOX_BASE_PRINT, addr);
			l3 = pix & 0xF;
			h3 = (pix >> 4) & 0xF;

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
/*
void display_img(int width, int height, int startx, int starty, alt_u8 *buffer) {
    alt_u8 *pix = buffer;

    // only for debugging, weird black pixel check value of certain adresses
    //printf("TL: %d\n", buffer[0] >> 4);
    //printf("TR: %d\n", buffer[width - 1] >> 4);
    //printf("BL: %d\n", buffer[(height - 1) * width] >> 4);
    //printf("BR: %d\n", buffer[width * height - 1] >> 4);

    for (int i = 0; i < ((320*240)); i++) {
    	int addr = 3*i;

		alt_u8 l1, h1, l2;
		alt_u8 h2, l3, h3;

		alt_u8 pix = IORD_8DIRECT(MAILBOX_BASE_PRINT, addr);

		l1 = pix & 0xF;
		h1 = (pix >> 4) & 0xF;

		addr++;
		pix = IORD_8DIRECT(MAILBOX_BASE_PRINT, addr);
		l2 = pix & 0xF;
		h2 = (pix >> 4) & 0xF;

		addr++;
		pix = IORD_8DIRECT(MAILBOX_BASE_PRINT, addr);
		l3 = pix & 0xF;
		h3 = (pix >> 4) & 0xF;

		IOWR(PIX_BUFF_ADDR_BASE, 0, 320*240-i-1);
		IOWR(PIX_DATA_R_BASE, 0, h1);//h1
		IOWR(PIX_DATA_G_BASE, 0, l1);//#l1
		IOWR(PIX_DATA_B_BASE, 0, h2);//h2

		IOWR(PIX_BUFF_ADDR_BASE, 0, 320*240-i);
		IOWR(PIX_DATA_R_BASE, 0, l2);//l2
		IOWR(PIX_DATA_G_BASE, 0, h3);
		IOWR(PIX_DATA_B_BASE, 0, l3);


    }
}
*/
