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
	int sum;

	alt_u8 *top;
	alt_u8 *mid;
	alt_u8 *bot;

	for (int j = 0; j < im_width; j++){
		for (int i = 0; i < im_height; i++){

			int jj = j;
			if (jj == 0) jj++;
			if (jj == im_width - 1) jj--;
			if (i == 0){
				mid = &image_data[jj-1];
				bot = mid;
			}
			top = mid;
			mid = bot;

			if (i != im_height-1) {
				bot = &image_data[(i+1)*im_width+jj-1];
			}

			if (j == 0){
				sum = top[0]+top[0]+top[1]+mid[0]+mid[0]+mid[1]+bot[0]+bot[0]+bot[1];
			} else if (j == im_width-1){
				sum = top[1]+top[2]+top[2]+mid[1]+mid[2]+mid[2]+bot[1]+bot[2]+bot[2];
			} else{
				sum = top[0]+top[1]+top[2]+mid[0]+mid[1]+mid[2]+bot[0]+bot[1]+bot[2];
			}
			image_return[i*im_width+j] = sum / 9;
		}
	}
	return;
}


