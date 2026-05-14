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
	    for (int i = 0; i < (320*240); i++) {
	    	IOWR(PIX_BUFF_ADDR_BASE, 0, i);
	        IOWR(PIX_DATA_BASE, 0, IORD_8DIRECT(MAILBOX_BASE_PRINT, i)); // bit shift by 4 coz 8 bit value only have 4 bits colour
	    }
	    frame++;
	    if (frame%10==0){
	    	//printf("%d frames sent", frame);
	    }

  }

  return 0;
}
