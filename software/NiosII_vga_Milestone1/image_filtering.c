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

	int sumL[3];
	int sumR[3];
	int sumCentL;
	int sumCentR;
	int sumsR[4];
	int sumsG[4];
	int sumsB[4];

	alt_u8 *top;
	alt_u8 *mid;
	alt_u8 *bot;


	for (int i = 0; i < im_height; i++){
		for (int j = 0; j < im_width; j++){

			if (j == 0){
				mid = &image_data[3*(i*im_width)];
				if (i == 0){
					top = mid;
					bot = &image_data[3*((i+1)*im_width)];
				} else if (i == im_height - 1) {
					top = &image_data[3*((i-1)*im_width)];
					bot = mid;
				} else {
					top = &image_data[3*((i-1)*im_width)];
					bot = &image_data[3*((i+1)*im_width)];
				}

				sumCentL = (top[0] & 0x0F) + (mid[0] & 0x0F) + (bot[0] & 0x0F);
				sumCentR = (top[1] >> 4) + (mid[1] >> 4) + (bot[1] >> 4);
				sumsR[1] = sumCentL;
				sumsR[2] = sumCentL;
				sumsR[3] = sumCentR;

				sumCentL = (top[0] >> 4) + (mid[0] >> 4) + (bot[0] >> 4);
				sumCentR = (top[2] & 0x0F) + (mid[2] & 0x0F) + (bot[2] & 0x0F);
				sumsG[1] = sumCentL;
				sumsG[2] = sumCentL;
				sumsG[3] = sumCentR;

				sumCentL = (top[1] & 0x0F) + (mid[1] & 0x0F) + (bot[1] & 0x0F);
				sumCentR = (top[2] >> 4) + (mid[2] >> 4) + (bot[2] >> 4);
				sumsB[1] = sumCentL;
				sumsB[2] = sumCentL;
				sumsB[3] = sumCentR;
			}

			sumL[0] = sumsR[1] + sumsR[2] + sumsR[3];
			sumsR[0] = sumsR[2];
			sumsR[1] = sumsR[3];

			sumL[1] = sumsG[1] + sumsG[2] + sumsG[3];
			sumsG[0] = sumsG[2];
			sumsG[1] = sumsG[3];

			sumL[2] = sumsB[1] + sumsB[2] + sumsB[3];
			sumsB[0] = sumsB[2];
			sumsB[1] = sumsB[3];

			if (j != im_width-1) {
				top += 3;
				mid += 3;
				bot += 3;
				sumCentL = (top[0] & 0x0F) + (mid[0] & 0x0F) + (bot[0] & 0x0F);
				sumCentR = (top[1] >> 4) + (mid[1] >> 4) + (bot[1] >> 4);
				sumsR[2] = sumCentL;
				sumsR[3] = sumCentR;

				sumCentL = (top[0] >> 4) + (mid[0] >> 4) + (bot[0] >> 4);
				sumCentR = (top[2] & 0x0F) + (mid[2] & 0x0F) + (bot[2] & 0x0F);
				sumsG[2] = sumCentL;
				sumsG[3] = sumCentR;

				sumCentL = (top[1] & 0x0F) + (mid[1] & 0x0F) + (bot[1] & 0x0F);
				sumCentR = (top[2] >> 4) + (mid[2] >> 4) + (bot[2] >> 4);
				sumsB[2] = sumCentL;
				sumsB[3] = sumCentR;
			} else {
				sumsR[2] = sumsR[3];
				sumsG[2] = sumsG[3];
				sumsB[2] = sumsB[3];
			}

			sumR[0] = sumsR[0] + sumsR[1] + sumsR[2];
			sumR[1] = sumsG[0] + sumsG[1] + sumsG[2];
			sumR[2] = sumsB[0] + sumsB[1] + sumsB[2];


			image_return[3*(i*im_width+j)] = ((sumL[1] / 9)<<4) + ((sumL[0] / 9) & 0x0F);
			image_return[3*(i*im_width+j)+1] = ((sumR[0] / 9)<<4) + ((sumL[2] / 9) & 0x0F);
			image_return[3*(i*im_width+j)+2] = ((sumL[2] / 9)<<4) + ((sumL[1] / 9) & 0x0F);
		}
	}


	return;
}

void edge_detector(alt_u8 image_data[], alt_u8 image_return[], int size){
	int im_width = 320;
	int im_height = 240;

	im_width >>= size + 1;
	im_height >>= size;


	float pixL, pixR;

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

	alt_u8 topL, midL, botL;
	alt_u8 topR, midR, botR;

	alt_u8 *ret = image_return;

	for (int i = 0; i < im_height; i++){
		for (int j = 0; j < im_width; j++){
			g_xL = 0;
			g_xR = 0;
			g_yL = 0;
			g_yR = 0;

			if (j == 0){
				mid = &image_data[3*i*im_width];
				if (i == 0){
					top = mid;
					bot = &image_data[3*((i+1)*im_width)];
				} else if (i == im_height - 1) {
					top = &image_data[3*(i-1)*im_width];
					bot = mid;
				} else {
					top = &image_data[3*(i-1)*im_width];
					bot = &image_data[3*(i+1)*im_width];
				}

				topL = (77*(top[0] & 0x0F)+150*(top[0] >> 4)+29*(top[1] & 0x0F) >> 8);
				topR = (77*(top[1] >> 4)+150*(top[2] & 0x0F)+29*(top[2] >> 4)>> 8);
				midL = (77*(mid[0] & 0x0F)+150*(mid[0] >> 4)+29*(mid[1] & 0x0F)>> 8);
				midR = (77*(mid[1] >> 4)+150*(mid[2] & 0x0F)+29*(mid[2] >> 4)>> 8);
				botL = (77*(bot[0] & 0x0F)+150*(bot[0] >> 4)+29*(bot[1] & 0x0F)>> 8);
				botR = (77*(bot[1] >> 4)+150*(bot[2] & 0x0F)+29*(bot[2] >> 4)>> 8);


				vertSumL = topL + 2*midL + botL;
				vertSumR = topR + 2*midR + botR;
				vertSums[1] = vertSumL;
				vertSums[2] = vertSumL;
				vertSums[3] = vertSumR;
				topPix[2] = topL;
				topPix[3] = topR;
				topPix[1] = topPix[2];

				botPix[2] = botL;
				botPix[3] = botR;
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
				top += 3;
				mid += 3;
				bot += 3;

				topL = (77*(top[0] & 0x0F)+150*(top[0] >> 4)+29*(top[1] & 0x0F) >> 8);
				topR = (77*(top[1] >> 4)+150*(top[2] & 0x0F)+29*(top[2] >> 4)>> 8);
				midL = (77*(mid[0] & 0x0F)+150*(mid[0] >> 4)+29*(mid[1] & 0x0F)>> 8);
				midR = (77*(mid[1] >> 4)+150*(mid[2] & 0x0F)+29*(mid[2] >> 4)>> 8);
				botL = (77*(bot[0] & 0x0F)+150*(bot[0] >> 4)+29*(bot[1] & 0x0F)>> 8);
				botR = (77*(bot[1] >> 4)+150*(bot[2] & 0x0F)+29*(bot[2] >> 4)>> 8);

				vertSumL = topL + 2*midL + botL;
				vertSumR = topR + 2*midR + botR;
				vertSums[2] = vertSumL;
				vertSums[3] = vertSumR;
				topPix[2] = topL;
				topPix[3] = topR;
				botPix[2] = botL;
				botPix[3] = botR;

			} else {
				vertSums[2] = vertSums[3];
				topPix[2] = topPix[3];
				botPix[2] = botPix[3];
			}

			g_xR = vertSums[0] - vertSums[2];
			g_yR = topPix[0] + 2*topPix[1] + topPix[2] - (botPix[0] + 2*botPix[1] + botPix[2]);

			gL = abs(g_xL) + abs(g_yL);
			gR = abs(g_xR) + abs(g_yR);


			ret[0] = 0;
			ret[1] = 0;
			ret[2] = 0;

			if (gL > thresh) {
				ret[0] += 0xFF;
				ret[1] += 0x0F;
			}
			if (gR > thresh) {
				ret[1] += 0xF0;
				ret[2] += 0xFF;
			}
			ret += 3;

		}
	}

	return;
}
