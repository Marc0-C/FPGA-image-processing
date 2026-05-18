// Details about the file and for what purpose it was built for
// Created By : Team A06
// Created Date: 05/05/26
// version ='2.0'

#include "io.h"

/*
 * flip_image is used to flip an image along both the horizontal and vertical directions.
 *
 * @param alt_u8 imageData[] - pointer to start of array holing the image data
 * @param alt_u8 imageReturn[] - pointer to start of array to write the processed data
 * @param int size - size of input image
 * 					 	0: full size
 * 					 	1: quarter size
 *
 * @return - None
 */
void flip_image(alt_u8 imageData[], alt_u8 imageReturn[], int size){
	//calculate image dimensions
	int imWidth = 320;
	int imHeight = 240;

	imWidth >>= size+1;
	imHeight >>= size;
	int imSize = 3*imWidth*imHeight;

	//iterate through pixels
	alt_u8 b0;
	alt_u8 b1;
	alt_u8 b2;
	int id;
	for (int i = 0; i < imHeight; i++){
		for (int j = 0; j < imWidth; j++){
			//read 2 pixels data
			id = i*imWidth+j;
			b0 = imageData[3*id];
			b1 = imageData[3*id+1];
			b2 = imageData[3*id+2];
			//write to flipped location
			imageReturn[imSize-3-3*id] = (b1 << 4) + (b2 >> 4);
			imageReturn[imSize-2-3*id] = (b2 << 4) + (b0 >> 4);
			imageReturn[imSize-1-3*id] = (b0 << 4) + (b1 >> 4);
		}
	}
	return;
}

/*
 * blur is used to blur an image using a 3x3 kernel.
 *
 * @param alt_u8 imageData[] - pointer to start of array holing the image data
 * @param alt_u8 imageReturn[] - pointer to start of array to write the processed data
 * @param int size - size of input image
 * 					 	0: full size
 * 					 	1: quater size
 *
 * @return - None
 */
void blur(alt_u8 imageData[], alt_u8 imageReturn[], int size){
	//calculate image dimensions
	int imWidth = 320;
	int imHeight = 240;

	imWidth >>= size + 1;
	imHeight >>= size;

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

	//iterate through pixels
	for (int i = 0; i < imHeight; i++){
		for (int j = 0; j < imWidth; j++){
			//find pointer to start of each row
			if (j == 0){
				mid = &imageData[3*(i*imWidth)];
				//image padding
				if (i == 0){
					top = mid;
					bot = &imageData[3*((i+1)*imWidth)];
				} else if (i == imHeight - 1) {
					top = &imageData[3*((i-1)*imWidth)];
					bot = mid;
				} else {
					top = &imageData[3*((i-1)*imWidth)];
					bot = &imageData[3*((i+1)*imWidth)];
				}

				//finds the vertical sum of each pixel colour and stores
				sumCentR = (top[0] >> 4) + (mid[0] >> 4) + (bot[0] >> 4);
				sumCentL = (top[1] & 0x0F) + (mid[1] & 0x0F) + (bot[1] & 0x0F);
				sumsR[1] = sumCentL;
				sumsR[2] = sumCentL;
				sumsR[3] = sumCentR;

				sumCentR = (top[0] & 0x0F) + (mid[0] & 0x0F) + (bot[0] & 0x0F);
				sumCentL = (top[2] >> 4) + (mid[2] >> 4) + (bot[2] >> 4);
				sumsG[1] = sumCentL;
				sumsG[2] = sumCentL;
				sumsG[3] = sumCentR;

				sumCentR = (top[1] >> 4) + (mid[1] >> 4) + (bot[1] >> 4);
				sumCentL = (bot[0] & 0x0F) + (bot[0] & 0x0F) + (bot[0] & 0x0F);
				sumsB[1] = sumCentL;
				sumsB[2] = sumCentL;
				sumsB[3] = sumCentR;
			}

			//sums 3 columns
			sumL[0] = sumsR[1] + sumsR[2] + sumsR[3];
			sumsR[0] = sumsR[2];
			sumsR[1] = sumsR[3];

			sumL[1] = sumsG[1] + sumsG[2] + sumsG[3];
			sumsG[0] = sumsG[2];
			sumsG[1] = sumsG[3];

			sumL[2] = sumsB[1] + sumsB[2] + sumsB[3];
			sumsB[0] = sumsB[2];
			sumsB[1] = sumsB[3];

			//gets the next pixel
			if (j != imWidth-1) {
				//increments row pointers
				top += 3;
				mid += 3;
				bot += 3;

				//finds the vertical sum of each pixel colour and stores
				sumCentR = (top[0] >> 4) + (mid[0] >> 4) + (bot[0] >> 4);
				sumCentL = (top[1] & 0x0F) + (mid[1] & 0x0F) + (bot[1] & 0x0F);
				sumsR[2] = sumCentL;
				sumsR[3] = sumCentR;

				sumCentR = (top[0] & 0x0F) + (mid[0] & 0x0F) + (bot[0] & 0x0F);
				sumCentL = (top[2] >> 4) + (mid[2] >> 4) + (bot[2] >> 4);
				sumsG[2] = sumCentL;
				sumsG[3] = sumCentR;

				sumCentR = (top[1] >> 4) + (mid[1] >> 4) + (bot[1] >> 4);
				sumCentL = (bot[0] & 0x0F) + (bot[0] & 0x0F) + (bot[0] & 0x0F);
				sumsB[2] = sumCentL;
				sumsB[3] = sumCentR;
			} else {
				//edge case for image padding
				sumsR[2] = sumsR[3];
				sumsG[2] = sumsG[3];
				sumsB[2] = sumsB[3];
			}
			//sums coulumns of the nexxt pixel
			sumR[0] = sumsR[0] + sumsR[1] + sumsR[2];
			sumR[1] = sumsG[0] + sumsG[1] + sumsG[2];
			sumR[2] = sumsB[0] + sumsB[1] + sumsB[2];

			//writes RGB values of 2 pixels to imageReturn
			imageReturn[3*(i*imWidth+j)] = (((sumR[0] * 57) >> 9) << 4) + (((sumR[1] * 57) >> 9) & 0x0F);
			imageReturn[3*(i*imWidth+j)+1] = (((sumR[2] * 57) >> 9) << 4) + (((sumL[0] * 57) >> 9) & 0x0F);
			imageReturn[3*(i*imWidth+j)+2] = (((sumL[1] * 57) >> 9) << 4) + (((sumL[2] * 57) >> 9) & 0x0F);
		}
	}


	return;
}

/*
 * edge_detector applies sobel filters in the x and y directions to locate the edges in an image
 *
 * @param alt_u8 imageData[] - pointer to start of array holing the image data
 * @param alt_u8 imageReturn[] - pointer to start of array to write the processed data
 * @param int size - size of input image
 * 					 	0: full size
 * 					 	1: quater size
 *
 * @return - None
 */
void edge_detector(alt_u8 imageData[], alt_u8 imageReturn[], int size){
	//calculate image dimensions
	int imWidth = 320;
	int imHeight = 240;

	imWidth >>= size + 1;
	imHeight >>= size;

	int sobelXL;
	int sobelXR;
	int sobelYL;
	int sobelYR;
	int gradL;
	int gradR;
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
	//iterate through pixels
	for (int i = 0; i < imHeight; i++){
		for (int j = 0; j < imWidth; j++){
			sobelXL = 0;
			sobelXR = 0;
			sobelYL = 0;
			sobelYR = 0;

			//find pointer to start of each row
			if (j == 0){
				mid = &imageData[3*i*imWidth];
				//image padding
				if (i == 0){
					top = mid;
					bot = &imageData[3*((i+1)*imWidth)];
				} else if (i == imHeight - 1) {
					top = &imageData[3*(i-1)*imWidth];
					bot = mid;
				} else {
					top = &imageData[3*(i-1)*imWidth];
					bot = &imageData[3*(i+1)*imWidth];
				}

				//converts each loaded pixel into grayscale
				topR = (77*(top[0] >> 4) + 150*(top[0] & 0x0F) + 29*(top[1] >> 4)) >> 8;
				topL = (77*(top[1] & 0x0F)+150*(top[2] >> 4)+29*(top[2] & 0x0F)) >> 8;
				midR = (77*(mid[0] >> 4)+150*(mid[0] & 0x0F)+29*(mid[1] >> 4)) >> 8;
				midL = (77*(mid[1] & 0x0F)+150*(mid[2] >> 4)+29*(mid[2] & 0x0F)) >> 8;
				botR = (77*(bot[0] >> 4)+150*(bot[0] & 0x0F)+29*(bot[1] >> 4)) >> 8;
				botL = (77*(bot[1] & 0x0F)+150*(bot[2] >> 4)+29*(bot[2] & 0x0F)) >> 8;

				//finds sums for sobel x and
				vertSumL = topL + 2*midL + botL;
				vertSumR = topR + 2*midR + botR;
				vertSums[1] = vertSumL;
				vertSums[2] = vertSumL;
				vertSums[3] = vertSumR;

				//stores top and bottom row for sobel y
				topPix[2] = topL;
				topPix[3] = topR;
				topPix[1] = topPix[2];

				botPix[2] = botL;
				botPix[3] = botR;
				botPix[1] = botPix[2];
			}
			//calculates sobel filters
			sobelXL = vertSums[1] - vertSums[3];
			sobelYL = topPix[1] + 2*topPix[2] + topPix[3] - (botPix[1] + 2*botPix[2] + botPix[3]);

			//shifts pixels in arrays
			vertSums[0] = vertSums[2];
			vertSums[1] = vertSums[3];
			topPix[0] = topPix[2];
			topPix[1] = topPix[3];
			botPix[0] = botPix[2];
			botPix[1] = botPix[3];

			//gets next pixels
			if (j != imWidth - 1) {
				//increments pointers
				top += 3;
				mid += 3;
				bot += 3;

				//converts each loaded pixel into grayscale
				topR = (77*(top[0] >> 4) + 150*(top[0] & 0x0F) + 29*(top[1] >> 4)) >> 8;
				topL = (77*(top[1] & 0x0F)+150*(top[2] >> 4)+29*(top[2] & 0x0F)) >> 8;
				midR = (77*(mid[0] >> 4)+150*(mid[0] & 0x0F)+29*(mid[1] >> 4)) >> 8;
				midL = (77*(mid[1] & 0x0F)+150*(mid[2] >> 4)+29*(mid[2] & 0x0F)) >> 8;
				botR = (77*(bot[0] >> 4)+150*(bot[0] & 0x0F)+29*(bot[1] >> 4)) >> 8;
				botL = (77*(bot[1] & 0x0F)+150*(bot[2] >> 4)+29*(bot[2] & 0x0F)) >> 8;

				//finds sums for sobel x and
				vertSumL = topL + 2*midL + botL;
				vertSumR = topR + 2*midR + botR;
				vertSums[2] = vertSumL;
				vertSums[3] = vertSumR;

				//stores top and bottom row for sobel y
				topPix[2] = topL;
				topPix[3] = topR;
				botPix[2] = botL;
				botPix[3] = botR;

			} else {
				//image padding
				vertSums[2] = vertSums[3];
				topPix[2] = topPix[3];
				botPix[2] = botPix[3];
			}

			//calculates sobel filters for next pixel
			sobelXR = vertSums[0] - vertSums[2];
			sobelYR = topPix[0] + 2*topPix[1] + topPix[2] - (botPix[0] + 2*botPix[1] + botPix[2]);

			//retrieves gradient value for each pixel
			gradL = abs(sobelXL) + abs(sobelYL);
			gradR = abs(sobelXR) + abs(sobelYR);


			imageReturn[0] = 0;
			imageReturn[1] = 0;
			imageReturn[2] = 0;

			//writes white to each pixel in return if gradient is above threshold
			if (gradR > thresh) {
				imageReturn[0] += 0xFF;
				imageReturn[1] += 0xF0;
			}
			if (gradL > thresh) {
				imageReturn[1] += 0x0F;
				imageReturn[2] += 0xFF;
			}
			imageReturn += 3;

		}
	}

	return;
}
