#include "io.h"

void flip_image(alt_u8 image_data[], alt_u8 image_return[], int axis, int size){

	int im_width = 320;
	int im_height = 240;

	im_width >>= size+1;
	im_height >>= size;
	int im_size = 3*im_width*im_height;

	alt_u8 b0;
	alt_u8 b1;
	alt_u8 b2;
	int id;
	for (int i = 0; i < im_height; i++){
		for (int j = 0; j < im_width; j++){
			id = i*im_width+j;
			b0 = image_data[3*id];
			b1 = image_data[3*id+1];
			b2 = image_data[3*id+2];
			image_return[im_size-1-3*id] = (b1 << 4) + (b0 >> 4);
			image_return[im_size-2-3*id] = (b0 << 4) + (b2 >> 4);
			image_return[im_size-3-3*id] = (b2 << 4) + (b1 >> 4);
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

	im_width >>= size + 1;
	im_height >>= size;


	int g_xL;
	int g_xR;
	int g_yL;
	int g_yR;
	int gL;
	int gR;
	int thresh;
	if (size == 0) thresh = 10;
	else thresh = 15;

	int vertSumL;
	int vertSumR;
	int vertSums[4];
	int topPix[4];
	int botPix[4];

	alt_u8 *top;
	alt_u8 *mid;
	alt_u8 *bot;

	alt_u8 ret;

	for (int i = 0; i < im_height; i++){
		for (int j = 0; j < im_width; j++){
			g_xL = 0;
			g_xR = 0;
			g_yL = 0;
			g_yR = 0;

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

				vertSumL = (*top >> 4) + 2*(*mid >> 4) + (*bot >> 4);
				vertSumR = (*top & 0x0F) + 2*(*mid & 0x0F) + (*bot & 0x0F);
				vertSums[1] = vertSumL;
				vertSums[2] = vertSumL;
				vertSums[3] = vertSumR;
				topPix[2] = (*top >> 4);
				topPix[3] = (*top & 0x0F);
				topPix[1] = topPix[2];

				botPix[2] = (*bot >> 4);
				botPix[3] = (*bot & 0x0F);
				botPix[1] = botPix[2];
			}

			g_xL = vertSums[1] - vertSums[3];
			g_yL = topPix[1] + 2*topPix[2] + topPix[3] - (botPix[1] + 2*botPix[2] + botPix[3]);
			vertSums[0] = vertSums[2];
			vertSums[1] = vertSums[3];
			topPix[0] = topPix[2];
			topPix[1] = topPix[3];
			botPix[0] = botPix[2];
			botPix[1] = botPix[3];

			if (j != im_width-1) {
				top++;
				mid++;
				bot++;
				vertSumL = (*top >> 4) + 2*(*mid >> 4) + (*bot >> 4);
				vertSumR = (*top & 0x0F) + 2*(*mid & 0x0F) + (*bot & 0x0F);
				vertSums[2] = vertSumL;
				vertSums[3] = vertSumR;
				topPix[2] = (*top >> 4);
				topPix[3] = (*top & 0x0F);
				botPix[2] = (*bot >> 4);
				botPix[3] = (*bot & 0x0F);

			} else {
				vertSums[2] = vertSums[3];
				topPix[2] = topPix[3];
				botPix[2] = botPix[3];
			}

			g_xR = vertSums[0] - vertSums[2];
			g_yR = topPix[0] + 2*topPix[1] + topPix[2] - (botPix[0] + 2*botPix[1] + botPix[2]);

			gL = abs(g_xL) + abs(g_yL);
			gR = abs(g_xR) + abs(g_yR);
			ret = 0;

			if (gL > thresh) ret += 0xF0;
			if (gR > thresh) ret += 0x0F;
			image_return[i*im_width+j] = ret;

		}
	}

	return;
}
