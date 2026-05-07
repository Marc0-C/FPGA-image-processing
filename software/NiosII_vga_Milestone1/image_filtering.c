#include "io.h"

void flip_image(alt_u8 image_data[], alt_u8 image_return[], int axis, int size){

	int im_width = 320;
	int im_height = 240;

	im_width >>= size;
	im_height >>= size;
	int im_size = im_width*im_height;


	for (int i = 0; i < im_size; i += im_width){
		for (int j = 0; j < im_width; j += 1){
			if (axis == 0) {
				image_return[j + i] = image_data[i+im_width-1-j];
			} else {
				image_return[j + i] = image_data[j+im_size-im_width-i];
			}
		}
	}
	return;
}

void blur(alt_u8 image_data[], alt_u8 image_return[], int size){
	int im_width = 320;
	int im_height = 240;

	im_width >>= size;
	im_height >>= size;

	for (int i = 0; i < im_height; i++){
		for (int j = 0; j < im_width; j++){
			int sum = 0;
			for (int ki = -1; ki < 2; ki++){
				for (int kj = -1; kj < 2; kj++){
					int ii = i + ki;
					int jj = j + kj;
					if (ii < 0) ii = 0;
					if (ii >= im_height) ii = im_height - 1;
					if (jj < 0) jj = 0;
					if (jj >= im_width) jj = im_width - 1;

					sum += image_data[ii*im_width+jj];
				}
			}
			image_return[i*im_width+j] = sum / 9;
		}
	}
	return;
}

void edge_detector(alt_u8 image_data[], alt_u8 image_return[], int size){
	int im_width = 320;
	int im_height = 240;

	im_width >>= size;
	im_height >>= size;


	int g_x;
	int g_y;
	int g;
	int thresh;
	if (size == 0) {
		thresh = 100;
	}else thresh = 200;
	int white = 15<<4;

	alt_u8 (*top)[3];
	alt_u8 (*mid)[3];
	alt_u8 (*bot)[3];

	for (int j = 0; j < im_width; j++){
		for (int i = 0; i < im_height; i++){
			g_x = 0;
			g_y = 0;

			if (i == 0){
				mid = &image_data[j-1];
				bot = mid;
			}
			top = mid;
			mid = bot;
			if (i != im_height-1) {
				bot = &image_data[(i+1)*im_width+j-1];
			}

			g_x = *top[0]+2*(*mid[0])+*bot[0]-(*top[2]+2*(*mid[2])+*bot[2]);
			g_y = *top[0]+2*(*top[1])+*top[2]-(*bot[0]+2*(*bot[1])+*bot[2]);


			g = abs(g_x) + abs(g_y);
			if (g > thresh){
				image_return[i*im_width+j] = white;
			} else {
				image_return[i*im_width+j] = 0;
			}
		}
	}
	return;
}
