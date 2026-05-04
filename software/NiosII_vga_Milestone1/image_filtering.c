#include "io.h"

void flip_image(alt_u8 image_data[], alt_u8 image_return[], int axis, int size){

	int im_width = 320;
	int im_height = 240;
	int im_size = im_width*im_height;

	im_width >>= size;
	im_height >>= size;


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

	int sobel_x[] = {1,0,-1,
					 2,0,-2,
					 1,0,-1};
	int sobel_y[] = {1,2,1,
					 0,0,0,
					 -1,-2,-1};

	int g_x;
	int g_y;
	int g;

	for (int i = 0; i < im_height; i++){
		for (int j = 0; j < im_width; j++){
			g_x = 0;
			g_y = 0;
			for (int ki = -1; ki < 2; ki++){
				for (int kj = -1; kj < 2; kj++){
					int ii = i + ki;
					int jj = j + kj;
					if (ii < 0) ii = 0;
					if (ii >= im_height) ii = im_height - 1;
					if (jj < 0) jj = 0;
					if (jj >= im_width) jj = im_width - 1;
					g_x += image_data[ii*im_width+jj]*sobel_x[(ki+1)*3+kj+1];
					g_y += image_data[ii*im_width+jj]*sobel_y[(ki+1)*3+kj+1];
				}
			}
			g = abs(g_x) + abs(g_y);
			if (g > 100){
				image_return[i*im_width+j] = 15<<4;
			} else {
				image_return[i*im_width+j] = 0;
			}
		}
	}
	return;
}
