#include "io.h"

void flip_image(alt_u8 image_data[], alt_u8 image_return[], int axis, int size){

	int im_width = 320;
	int im_height = 240;

	im_width >>= size+1;
	im_height >>= size;
	int im_size = im_width*im_height;

	alt_u8 temp;
	int id;
	for (int i = 0; i < im_height; i++){
		for (int j = 0; j < im_width; j++){
			id = i*im_width+j;
			temp = image_data[id];
			image_return[im_size-1-id] = (temp << 4) + (temp >> 4);
		}
	}
	return;
}

void blur(alt_u8 image_data[], alt_u8 image_return[], int size){

	int im_width = 320;
	int im_height = 240;

	im_width >>= size + 1;
	im_height >>= size;

	int sumL;
	int sumR;
	int sumCentL;
	int sumCentR;
	int sums[4];

	alt_u8 *top;
	alt_u8 *mid;
	alt_u8 *bot;


	for (int i = 0; i < im_height; i++){
		for (int j = 0; j < im_width; j++){

			if (j == 0){
				mid = &image_data[i*im_width];
				if (i == 0){
					top = mid;
					bot = &image_data[(i+1)*im_width];
				} else if (i == im_height - 1) {
					top = &image_data[(i-1)*im_width];
					bot = mid;
				} else {
					top = &image_data[(i-1)*im_width];
					bot = &image_data[(i+1)*im_width];
				}

				sumCentL = (*top >> 4) + (*mid >> 4) + (*bot >> 4);
				sumCentR = (*top & 0x0F) + (*mid & 0x0F) + (*bot & 0x0F);
				sums[1] = sumCentL;
				sums[2] = sumCentL;
				sums[3] = sumCentR;
			}

			sumL = sums[1] + sums[2] + sums[3];
			sums[0] = sums[2];
			sums[1] = sums[3];

			if (j != im_width-1) {
				top++;
				mid++;
				bot++;
				sumCentL = (*top >> 4) + (*mid >> 4) + (*bot >> 4);
				sumCentR = (*top & 0x0F) + (*mid & 0x0F) + (*bot & 0x0F);
				sums[2] = sumCentL;
				sums[3] = sumCentR;
			} else sums[2] = sums[3];

			sumR = sums[0] + sums[1] + sums[2];


			image_return[i*im_width+j] = ((sumL / 9)<<4) + ((sumR / 9) & 0x0F);
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
	if (size == 0) thresh = 100;
	else thresh = 200;
	int white = 15<<4;

	alt_u8 (*top)[3];
	alt_u8 (*mid)[3];
	alt_u8 (*bot)[3];

	for (int j = 0; j < im_width; j++){
		for (int i = 0; i < im_height; i++){
			g_x = 0;
			g_y = 0;

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
				g_x = *top[0]+2*(*mid[0])+*bot[0]-(*top[1]+2*(*mid[1])+*bot[1]);
				g_y = *top[0]+2*(*top[0])+*top[1]-(*bot[0]+2*(*bot[0])+*bot[1]);
			} else if (j == im_width-1){
				g_x = *top[1]+2*(*mid[1])+*bot[1]-(*top[2]+2*(*mid[2])+*bot[2]);
				g_y = *top[1]+2*(*top[2])+*top[2]-(*bot[1]+2*(*bot[2])+*bot[2]);
			} else{
				g_x = *top[0]+2*(*mid[0])+*bot[0]-(*top[2]+2*(*mid[2])+*bot[2]);
				g_y = *top[0]+2*(*top[1])+*top[2]-(*bot[0]+2*(*bot[1])+*bot[2]);
			}

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
