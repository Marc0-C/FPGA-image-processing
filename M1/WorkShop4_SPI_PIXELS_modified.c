#include <stdio.h>
// TODO add correct include

#include "system.h" // allows access to the components
#include "io.h"     // macros to access PIOs

#include <altera_avalon_spi.h> // need to find these headers
#include <altera_avalon_spi_regs.h>



// define registers

// define image format
#define WIDTH 320
#define HEIGHT 240
#define IMAGE_SIZE (WIDTH * HEIGHT / 2) // 4bits/pixel, so 2 pixels per byte

uint8_t image_buffer[IMAGE_SIZE]; // buffer

void image_create() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {

            // simple gradient pattern
            uint8_t pixel = (x % 16); // pixel = 4 bits

            int index = (y * WIDTH + x) / 2;  // adress = column + (row *width)

            if (x % 2 == 0) {
                // all even pixels are in the lower 4 bits of the byte
                image_buffer[index] = pixel & 0x0F;
            } else {
                // odd pixels in the other 4
                image_buffer[index] |= (pixel << 4); // |= bitwise or
            }
        }
    }
}
// ADD LATER
/*
void request_image_spi() {
    uint8_t tx = 0x00;

    alt_avalon_spi_command(
        //SPI adress,
        //SPI cs number,
        //size of send buffer,
        //pionter to send buffer,
        //sice of recieve buffer,
        //pointer to send buffer,
        0 //flag
    );
}
*/
int main() {

    printf("Test\n");

    while (1) {

        image_create();
        printf("image created");

        // delay
        for (volatile int i = 0; i < 1000000; i++);
    }

    return 0;
}






















