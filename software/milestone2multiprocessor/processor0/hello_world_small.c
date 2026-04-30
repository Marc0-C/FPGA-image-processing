#include <stdio.h>
#include "sys/alt_stdio.h"
#include <stdint.h>
#include "system.h"
#include "io.h"
#include "altera_avalon_mutex.h"
#include "altera_avalon_pio_regs.h"

//#define SHARED_MAILBOX_BASE 0x03FFF000
#define SHARED_MAILBOX_BASE 0x03312D20


#define IM_SIZE (240 * 320)
//#define IM_SIZE (10)

void display_fps(int duration);

int main() {
    //printf("processor0\n");
    alt_mutex_dev* mutex = altera_avalon_mutex_open(MUTEX_0_NAME);
    int start_time, end_time, duration;
    static uint8_t processing_buffer[IM_SIZE];
    // Initialise Displays
    IOWR_32DIRECT(HEX2_BASE, 0, 0xFFFFFF);
    int framenum = 0;
    int first_pixelvalue = 0;
    int last_pixelvalue = 0;
    while (1) {
        start_time = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE);

        // 1. Critical Section: Read from shared memory
        altera_avalon_mutex_lock(mutex, 1);
        //printf("just locked mutex \n");
        for (int i = 0; i < IM_SIZE; i++) {
        	// base, offset, copy all pixels to processing_buffer
        	processing_buffer[i] = IORD_8DIRECT(SHARED_MAILBOX_BASE, i);
            }
        altera_avalon_mutex_unlock(mutex);

        //for (volatile int i = 0; i<500000; i++){}
        //printf("just unlocked mutex \n");

        first_pixelvalue = processing_buffer[0];
        last_pixelvalue = processing_buffer[IM_SIZE-1];
        // 2. Processing & VGA Output
        // loop through rows and columns

        for (int i = 0; i < 240; i++) {
            for (int j = 0; j < 320; j++) {
                IOWR(PIX_BUFF_ADDR_BASE, 0, i * 320 + j); // coordinate to vga
                uint8_t pixel = processing_buffer[320 * i + 319 - j - 2]; //flip image
                IOWR(PIX_DATA_BASE, 0, pixel >> 4); //pass to 4 bit colour
            }
        }

        framenum += 1;
        //if (framenum%20 == 0){printf("sent frame %d to vga, firstpixelvalue %d, lastpixelvalue%d: \n", framenum, first_pixelvalue, last_pixelvalue);}

        end_time = IORD_ALTERA_AVALON_PIO_DATA(USEC_COUNTER_PIO_BASE);
        duration = end_time - start_time;
        display_fps(duration);
        //for (volatile int i = 0; i<500000; i++){}
        //printf("duration %d", duration);
    }
    return 0;
}

// keep as in M1
void display_fps(int duration) {
    uint8_t hex_lookup[] = {0b11000000, 0b11111001, 0b10100100, 0b10110000, 0b10011001,
                            0b10010010, 0b10000010, 0b11111000, 0b10000000, 0b10010000};
    if (duration == 0) return;
    float fps = 1000000.0 / duration;
    int d2 = (int)fps;
    int d1 = (int)(fps * 10.0) % 10;
    int d0 = (int)(fps * 100.0) % 10;
    int hex_data = ((hex_lookup[d2] & 0x7F) << 16) | (hex_lookup[d1] << 8) | hex_lookup[d0];
    IOWR_32DIRECT(HEX1_BASE, 0, hex_data);
}
