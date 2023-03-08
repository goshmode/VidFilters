//James Marcel
//filter library

#include <cstdio>
#include <cstring>
#include <opencv2/opencv.hpp>
#include "filter.h"


//apply a 3x3 filter - datatype will be CV_16SC3
//[-1 0 1]
//[-2 0 2]
//[-1 0 1]
int gradX(cv::Mat &src, cv::Mat &dst) {

	//allocate dst image - init to 0s
	dst = cv::Mat::zeros(src.size(), CV_16SC3); //signed short data type

	//loop over src and apply a 3x3 filter
	for (int i = 1; i < src.rows - 1; i++) {

		//src needs ptrs to rows above and below
		cv::Vec3b *rptrm1 = src.ptr<cv::Vec3b>(i - 1);
		cv::Vec3b *rptr = src.ptr<cv::Vec3b>(i);
		cv::Vec3b *rptrp1 = src.ptr<cv::Vec3b>(i + 1);

		//dst is short so we need to use Vec3s
		cv::Vec3s *dptr = dst.ptr<cv::Vec3s>(i);

		for (int j = 1; j < src.cols - 1; j++) {

			//for each color channel
			for (int c = 0; c < 3; c++) {
				//summing filtered surrounding values
				dptr[j][c] = ((-1 * rptrm1[j - 1][c]) + rptrp1[j + 1][c] +
					(-2 * rptr[j - 1][c]) + (2 * rptr[j + 1][c]) +
					(-1 * rptrp1[j - 1][c]) + rptrp1[j + 1][c]) / 4;
			}

		}
	}

	//return
	return 0;
}

//grayScale averages the RGB values of each pixel and sets result in destination array as uchar
int grayScale(cv::Mat& src, cv::Mat& dst) {

	//allocate dst image - init to 0s
	dst = cv::Mat::zeros(src.size(), CV_8U); //using 8 bit chars since we just need one color

	//loop over source and avg the 3 color values to get a grayscale value
	for (int i = 0; i < src.rows; i++) {

		//src row pointer
		cv::Vec3b* rptr = src.ptr<cv::Vec3b>(i);

		//dst is short so we need to use Vec3s
		uchar* dptr = dst.ptr<uchar>(i);

		//going through each column of 
		for (int j = 0; j < src.cols; j++) {

			//averaging each vec3b into a single value to store in the dst
			dptr[j] = (rptr[j][0] + rptr[j][1] + rptr[j][2]) / 3;

		}

	}


	//return
	return 0;
}



//blur filter is separable 1x5 and 5x1 filters ([1 2 4 2 1]) to approximate a 5x5 gaussian blur in the destination
int blur5x5(cv::Mat& src, cv::Mat& dst) {


	//allocate dst image - init to 0s
	//dst2 is the first convolution, then calculations are written to dst for final result
	cv::Mat dst2;
	dst2 = cv::Mat::zeros(src.size(), CV_16FC3); //this space is for the first convolution
	//allocating the destination image:
	dst = cv::Mat::zeros(src.size(), src.type()); 



	
	//loop over src and apply the horizontal 
	for (int i = 3; i < (src.rows - 3); i++) {

		cv::Vec3b* rptr = src.ptr<cv::Vec3b>(i);

		//dst is short so we need to use Vec3s
		cv::Vec3s* dptr2 = dst2.ptr<cv::Vec3s>(i);


		for (int j = 3; j < (src.cols - 3); j++) {


				//for each color channel
				for (int c = 0; c < 3; c++) {

					//summing filtered surrounding values for horizontal [1 2 4 2 1] 1x5 filter
					dptr2[j][c] = (rptr[j - 2][c] + (2 * rptr[j - 1][c]) +
						(4 * rptr[j][c]) + (2 * rptr[j + 1][c]) + rptr[j + 2][c]);

				}


		}

	}



	// 2nd loop applies the 5x1 filter [1 2 4 2 1] and writes to final result destination
	for (int i = 3; i < dst2.rows - 3; i++) {

		//col pointers for updated source
		cv::Vec3s* nrptrm2 = dst2.ptr<cv::Vec3s>(i - 2);
		cv::Vec3s* nrptrm1 = dst2.ptr<cv::Vec3s>(i - 1);
		cv::Vec3s* nrptr = dst2.ptr<cv::Vec3s>(i);
		cv::Vec3s* nrptrp1 = dst2.ptr<cv::Vec3s>(i + 1);
		cv::Vec3s* nrptrp2 = dst2.ptr<cv::Vec3s>(i + 2);

		//col pointer for final destination
		cv::Vec3b* dptr = dst.ptr<cv::Vec3b>(i);

		//for each row in this column
		for (int j = 3; j < dst2.cols - 3; j++) {
			for (int c = 0; c < 3; c++) {

				//writing result to destination array (result divided by sum of gaussian array (100))
				dptr[j][c] = (nrptrm2[j][c] + (2 * nrptrm1[j][c]) + (4 * nrptr[j][c]) +
					(2 * nrptrp1[j][c]) + nrptrp2[j][c]) / 100;

			}
		}
	}

	//handling edge cases that can't be properly calculated by filter
	for (int i = 0; i < src.rows; i++) {
		cv::Vec3b* rptr = src.ptr<cv::Vec3b>(i);
		cv::Vec3b* dptr = dst.ptr<cv::Vec3b>(i);

		//case for left and right edges
		if ((i >= 6) && (i <= src.rows - 6)) {
			for (int c = 0; c < 3; c++) {
				dptr[0][c] = rptr[0][c];
				dptr[1][c] = rptr[1][c];
				dptr[2][c] = rptr[2][c];
				dptr[src.cols - 3][c] = rptr[src.cols - 3][c];
				dptr[src.cols - 2][c] = rptr[src.cols - 2][c];
				dptr[src.cols - 1][c] = rptr[src.cols - 1][c];

			}
		}
		//case for top and bottom edges
		else {
			for (int j = 0; j < src.cols; j++) {
				for (int c = 0; c < 3; c++) {
					dptr[j][c] = rptr[j][c];
				}
			}

		}

	}

return 0;
}


//implements X sobel 3x3 filter convolving [-1 0 1]horizontal and [1 2 1] vertical (positive right)
int sobelX3x3(cv::Mat &src, cv::Mat &dst) {

	//allocate dst image - init to 0s
	//dst2 is the first convolution, then calculations are written to dst for final result
	cv::Mat dst2;
	dst2 = cv::Mat::zeros(src.size(), CV_16SC3); //this space is for the first convolution
	//allocating the destination image:
	dst = cv::Mat::zeros(src.size(), CV_16SC3); //signed short data type for passed destination

	//loop over src and apply the horizontal 
	for (int i = 2; i < (src.rows - 2); i++) {

		cv::Vec3b* rptr = src.ptr<cv::Vec3b>(i);

		//dst is short so we need to use Vec3s
		cv::Vec3s* dptr2 = dst2.ptr<cv::Vec3s>(i);

		for (int j = 2; j < (src.cols - 2); j++) {

			//for each color channel
			for (int c = 0; c < 3; c++) {

				//summing filtered surrounding values for horizontal [-1 0 1] 1x3 filter
				dptr2[j][c] = (-1 * rptr[j - 1][c]) + rptr[j + 1][c];

			}
		}
	}

	// 2nd loop applies the 3x1 filter [1 2 1]
	for (int i = 2; i < dst2.rows - 2; i++) {

		//col pointer for updated source
		cv::Vec3s* nrptrm1 = dst2.ptr<cv::Vec3s>(i - 1);
		cv::Vec3s* nrptr = dst2.ptr<cv::Vec3s>(i);
		cv::Vec3s* nrptrp1 = dst2.ptr<cv::Vec3s>(i + 1);

		//col pointer for final destination
		cv::Vec3s* dptr = dst.ptr<cv::Vec3s>(i);

		//for each row in this column
		for (int j = 2; j < dst2.cols - 2; j++) {
			for (int c = 0; c < 3; c++) {

				dptr[j][c] = ( nrptrm1[j][c] + (2 * nrptr[j][c]) + nrptrp1[j][c] );

			}
		}
	}

	return 0;
}


//implements Y sobel 3x3 filter convolving [-1 0 1]vertical and [1 2 1] horizontal
//works off same logic as X3x3 but positive down
int sobelY3x3(cv::Mat &src, cv::Mat &dst) {

	//allocate dst image - init to 0s
	//dst2 is the first convolution, then calculations are written to dst for final result
	cv::Mat dst2;
	dst2 = cv::Mat::zeros(src.size(), CV_16SC3); //this space is for the first convolution
	//allocating the destination image:
	dst = cv::Mat::zeros(src.size(), CV_16SC3); //signed short data type for passed destination

	//loop over src and apply the horizontal 
	for (int i = 2; i < (src.rows - 2); i++) {

		cv::Vec3b* rptr = src.ptr<cv::Vec3b>(i);

		//dst is short so we need to use Vec3s
		cv::Vec3s* dptr2 = dst2.ptr<cv::Vec3s>(i);

		for (int j = 2; j < (src.cols - 2); j++) {

			//for each color channel
			for (int c = 0; c < 3; c++) {

				//summing filtered surrounding values for horizontal [1 2 1] 1x3 filter
				dptr2[j][c] =  rptr[j - 1][c] + (2 * rptr[j][c]) +  rptr[j + 1][c];

			}
		}
	}

	// 2nd loop applies the 3x1 filter [-1 0 1]
	for (int i = 2; i < dst2.rows - 2; i++) {

		//col pointer for updated source
		cv::Vec3s* nrptrm1 = dst2.ptr<cv::Vec3s>(i - 1);
		cv::Vec3s* nrptr = dst2.ptr<cv::Vec3s>(i);
		cv::Vec3s* nrptrp1 = dst2.ptr<cv::Vec3s>(i + 1);

		//col pointer for final destination
		cv::Vec3s* dptr = dst.ptr<cv::Vec3s>(i);

		//for each row in this column
		for (int j = 2; j < dst2.cols - 2; j++) {
			for (int c = 0; c < 3; c++) {

				dptr[j][c] = ( (- 1 * nrptrm1[j][c]) + nrptrp1[j][c] );

			}
		}
	}

	return 0;
}

//combines sobelx and sobely arrays to determine gradient magnitude of each pixel.
//NOTE: I divided each result by 3 to give a less noisy image. Could be due to my webcam.
int magnitude(cv::Mat &sx, cv::Mat &sy, cv::Mat &dst) {

	dst = cv::Mat::zeros(sx.size(), CV_8UC3);

	for (int i = 0; i < sx.rows; i++) {

		//row pointers for sx and sy
		cv::Vec3s* xptr = sx.ptr<cv::Vec3s>(i);
		cv::Vec3s* yptr = sy.ptr<cv::Vec3s>(i);

		//row pointer for destination
		cv::Vec3b* dptr = dst.ptr<cv::Vec3b>(i);

		for (int j = 0; j < sx.cols; j++) {

			for (int c = 0; c < 3; c++) {

				//NOTE: Initially the formula sqrt(sx*sx + sy*sy) was too sensitive on my camera, so i divided results by 3 for a more practical image
				dptr[j][c] = sqrt((xptr[j][c] * xptr[j][c]) + (yptr[j][c] * yptr[j][c]))/3;

			}
		}

	}

	return 0;
}

//blurs the image but chooses one of 'levels' pixel values to quantize color
//Uses the same framework as gaussian blur, but added the bucket steps into the second full frame iteration to save
//cpu from computing another full iteration
int blurQuantize(cv::Mat& src, cv::Mat& dst, int levels) {

	float buckets = static_cast<float>(255) / levels;

	//allocate dst image - init to 0s
	//dst2 is the first convolution, then calculations are written to dst for final result
	cv::Mat dst2;
	dst2 = cv::Mat::zeros(src.size(), CV_16FC3); //this space is for the first convolution
	//allocating the destination image:
	dst = cv::Mat::zeros(src.size(), src.type());

	//loop over src and apply the horizontal 
	for (int i = 3; i < (src.rows - 3); i++) {

		cv::Vec3b* rptr = src.ptr<cv::Vec3b>(i);

		//dst is short so we need to use Vec3s
		cv::Vec3s* dptr2 = dst2.ptr<cv::Vec3s>(i);

		for (int j = 3; j < (src.cols - 3); j++) {

			//for each color channel
			for (int c = 0; c < 3; c++) {

				//summing filtered surrounding values for horizontal [1 2 4 2 1] 1x5 filter
				dptr2[j][c] = (rptr[j - 2][c] + (2 * rptr[j - 1][c]) +
					(4 * rptr[j][c]) + (2 * rptr[j + 1][c]) + rptr[j + 2][c]);

			}
		}
	}



	// 2nd loop applies the 5x1 filter [1 2 4 2 1]
	for (int i = 3; i < dst2.rows - 3; i++) {

		//col pointer for updated source
		cv::Vec3s* nrptrm2 = dst2.ptr<cv::Vec3s>(i - 2);
		cv::Vec3s* nrptrm1 = dst2.ptr<cv::Vec3s>(i - 1);
		cv::Vec3s* nrptr = dst2.ptr<cv::Vec3s>(i);
		cv::Vec3s* nrptrp1 = dst2.ptr<cv::Vec3s>(i + 1);
		cv::Vec3s* nrptrp2 = dst2.ptr<cv::Vec3s>(i + 2);

		//col pointer for final destination
		cv::Vec3b* dptr = dst.ptr<cv::Vec3b>(i);

		//for each row in this column
		for (int j = 3; j < dst2.cols - 3; j++) {
			for (int c = 0; c < 3; c++) {

				short blurValue = (nrptrm2[j][c] + (2 * nrptrm1[j][c]) + (4 * nrptr[j][c]) +
					(2 * nrptrp1[j][c]) + nrptrp2[j][c]) / 100;
				int zone = blurValue / buckets;
				dptr[j][c] = zone * buckets;


				/*
				if ((j == 100) && (i == 100)) {
					printf("value here is %d \n", dptr[j][c]);
				}
				*/
			}
		}
	}

	//handling edge cases that can't be properly calculated by filter
	for (int i = 0; i < src.rows; i++) {
		cv::Vec3b* rptr = src.ptr<cv::Vec3b>(i);
		cv::Vec3b* dptr = dst.ptr<cv::Vec3b>(i);

		//case for left and right edges
		if ((i >= 6) && (i <= src.rows - 6)) {
			for (int c = 0; c < 3; c++) {
				dptr[0][c] = rptr[0][c];
				dptr[1][c] = rptr[1][c];
				dptr[2][c] = rptr[2][c];
				dptr[src.cols - 3][c] = rptr[src.cols - 3][c];
				dptr[src.cols - 2][c] = rptr[src.cols - 2][c];
				dptr[src.cols - 1][c] = rptr[src.cols - 1][c];

			}
		}
		//case for top and bottom edges
		else {
			for (int j = 0; j < src.cols; j++) {
				for (int c = 0; c < 3; c++) {
					dptr[j][c] = rptr[j][c];
				}
			}

		}

	}


	return 0;
}

//cartoon filter generates a color quantized frame and checks sobel magnitude for each pixel
//In my implementation, I zeroed the destination image to black, then only filled in pixels with a magnitude under the threshold
int cartoon(cv::Mat& src, cv::Mat& dst, int levels, int magThreshold) {

	cv::Mat xsobelsrc;
	cv::Mat ysobelsrc;
	cv::Mat magdisp;
	cv::Mat quantsrc;
	sobelX3x3(src, xsobelsrc);
	sobelY3x3(src, ysobelsrc);
	//generating magnitude and quantize frames of 'levels' levels
	//magnitude(xsobelsrc, ysobelsrc, magdisp);
	blurQuantize(src, quantsrc, levels);

	dst = cv::Mat::zeros(src.size(), src.type());


	//iterate through all rows. If magnitude has value higher than magThreshold, fill in black on quantized image
	for (int i = 0; i < dst.rows; i++) {

		//col pointer for x and y sobel
		cv::Vec3s* xptr = xsobelsrc.ptr<cv::Vec3s>(i);
		cv::Vec3s* yptr = ysobelsrc.ptr<cv::Vec3s>(i);

		//col pointer for updated source
		cv::Vec3b* rptr = quantsrc.ptr<cv::Vec3b>(i);

		//col pointer for final destination
		cv::Vec3b* dptr = dst.ptr<cv::Vec3b>(i);

		//for each row in this column
		for (int j = 0; j < dst.cols; j++) {
			int count = 0;
			for (int c = 0; c < 3; c++){
				if (sqrt( (xptr[j][c] * xptr[j][c]) + (yptr[j][c] * yptr[j][c]) ) / 3 > magThreshold ) {
					
					count++;
				}
			}
			
			if (count == 0) {
				dptr[j][0] = rptr[j][0];
				dptr[j][1] = rptr[j][1];
				dptr[j][2] = rptr[j][2];
			}

			/*
			if ((j == 100) && (i == 100)) {
				printf("value here is %d \n", dptr[j][c]);
			}
			*/
		}
	}


	return 0;
}
	


//This filter chooses a pixel and gives an adjacent scale x scale area the same values
int pixelate(cv::Mat &src, cv::Mat &dst, int scale) {

	dst = cv::Mat::zeros(src.size(), src.type());

	int baseRow = 0;
	int baseCol = 0;
	//pixel iteration.
	for (int i = 0; i < src.rows; i++) {

		//determines whether to shift the basePixel value
		if (i > 0) {
			if (i % scale == 0) {
				baseRow = i;
			}
		}

		//source row pointer only needs to keep track of 'scale' row
		cv::Vec3b* rptr = src.ptr<cv::Vec3b>(baseRow);

		//destination pointer needs to fill in every row
		cv::Vec3b* dptr = dst.ptr<cv::Vec3b>(i);

		for (int j = 0; j < src.cols; j++) {

			//setting up baseCol based on scale. edge case to avoid divide by 0
			if (j == 0) {
				baseCol = 0;
			}
			else if (j % scale == 0) {
				baseCol = j;
			}

			//each color value is copied to the adjacent 2x2 block of pixels
			for (int c = 0; c < 3; c++) {

				// color all destination pixels based on source data at [baseRow][baseCol][c]
				dptr[j][c] = rptr[baseCol][c];
				

			}

		}
	}


	return 0;
}



//This filter takes the current frame and the last frame, 
//then determines (based on sens) whether to show the new frame or the old one.
//This gives the resulting frame a trail of the last frame, emphasizing movement.
int movement(cv::Mat &src,cv::Mat &last, cv::Mat &dst, int sens) {

	dst = cv::Mat::zeros(src.size(), src.type());

	for (int i = 0; i < src.rows; i++) {

		//pointer for source
		cv::Vec3b* rptr = src.ptr<cv::Vec3b>(i);

		//pointer for last frame
		cv::Vec3b* lptr = last.ptr<cv::Vec3b>(i);

		//pointer for destination
		cv::Vec3b* dptr = dst.ptr<cv::Vec3b>(i);

		for (int j = 0; j < src.cols;j++) {
			//if sum of BGR in src is different enough from lastFrame, set dest to src
			int newsum = rptr[j][0] + rptr[j][1] + rptr[j][2];
				int oldsum = lptr[j][0] + lptr[j][1] + lptr[j][2];
			//only fill color values if source pixel is different enough from last frame
			if ( abs(newsum - oldsum) > sens) {
				for (int c = 0; c < 3; c++) {
					dptr[j][c] = rptr[j][c];
				}
			}
			//otherwise write it from the last frame
			else {
				for (int c = 0; c < 3; c++) {
					dptr[j][c] = lptr[j][c];
				}
			}

		}
	}

	return 0;
}



//this filter will adjust two color channels by a designated amount 'shift'
int colorshift(cv::Mat &src, cv::Mat &dst, int shift) {

	dst = cv::Mat::zeros(src.size(), src.type());

	for (int i = 0; i < src.rows; i++) {

		//pointer for source
		cv::Vec3b* rptr = src.ptr<cv::Vec3b>(i);

		//pointer for destination
		cv::Vec3b* dptr = dst.ptr<cv::Vec3b>(i);

		for (int j = 0; j < src.cols; j++) {

			for (int c = 0; c < 3; c++) {

				//blue channels add shift
				if (c == 0) {
					//checking if value + shift goes out of bounds in either direction
					if (rptr[j][c] + shift > 255) {
						dptr[j][c] = 255;
					}
					else if (rptr[j][c] + shift < 0) {
						dptr[j][c] = 0;
					}
					//otherwise add shift to source's value for this color
					else {
						dptr[j][c] = rptr[j][c] + shift;
					}
				}
				//green values are shifted in the opposite direction
				else if (c == 1) {
					//checking if value + shift goes out of bounds in either direction
					if (rptr[j][c] - shift > 255) {
						dptr[j][c] = 255;
					}
					else if (rptr[j][c] - shift < 0) {
						dptr[j][c] = 0;
					}
					else {
						dptr[j][c] = rptr[j][c] - shift;
					}
				}
				//red stays the same in this implementation
				else {
					dptr[j][c] = rptr[j][c];
				}
			}
		}
	}

	return 0;
}


//attempt to make an hdr image through histogram equalization
//The algorithm comes from https://cromwell-intl.com/3d/histogram/
//but the implementation is my own
int hdrEQ(cv::Mat& src, cv::Mat& dst) {

	//making histogram counting each instance of each 'value'
	int histo[256] = { 0 };
	//initializing the cumulative density function array
	int cdf[256] = { 0 };

	dst = cv::Mat::zeros(src.size(), src.type());

	//looping through each pixel to count values for the histogram
	for (int i = 0; i < src.rows; i++) {

		cv::Vec3b* rptr = src.ptr<cv::Vec3b>(i);

		for (int j = 0; j < src.cols; j++) {
			//increment the histogram element for this pixel's value
			histo[rptr[j][2]] += 1;
		}
		
	}

	//calculating cumulative density function (sum of all values in histo
	//lower than any value and including that value)
	for (int x = 0; x < 256;x++) {
		//first case, then the rest can be dynamic programming
		if (x == 0) {
			cdf[x] = histo[0];
		}
		else {
			cdf[x] = histo[x] + cdf[x - 1];
		}
	}

	//n is total number of pixels
	int n = src.rows * src.cols;
	//now going through each pixel and updating the value based on 
	//cdf and total num of pixels to equalize
	for (int i = 0; i < src.rows; i++) {

		cv::Vec3b* rptr = src.ptr<cv::Vec3b>(i);
		cv::Vec3b* dptr = dst.ptr<cv::Vec3b>(i);

		for (int j = 0; j < src.cols; j++) {

			//calculating the equalized value based on our histogram and cdf results
			dptr[j][2] = floor(255 * ( cdf[rptr[j][2]] - cdf[0]) / (n - cdf[0]) );
			//copying the hue/sat from source too
			dptr[j][0] = rptr[j][0];
			dptr[j][1] = rptr[j][1];
			
		}
	}

	return 0;
}