/*
	James Marcel

	Establishes webcam feed, filters the frame based on input,
	then displays the frame.
*/

#include <cstdio>
#include <cstring>
#include <opencv2/opencv.hpp>
#include "filter.h"

//global used for screenshot numbering
int screenNum = 0;


//screenshot saves the passed Mat as an jpg image. 
//Warning: There is no handling for filename conflicts. Writes to exe directory
int screenshot(cv::Mat &frame) {

	std::string filename = "screenshot_";

	filename += (std::to_string(screenNum) + ".jpg");
	cv::imwrite(filename, frame);
	printf("Screenshot taken.");
	screenNum++;
	return 0;
}

int main(int argc, char* argv[]) {

	cv::VideoCapture* capdev;

	//open the video device
	capdev = new cv::VideoCapture(0);
	if (!capdev->isOpened()) {
		printf("Unable to open video device\n");
		return -1;

	}

	//get some properties of the image
	cv::Size refS((int)capdev->get(cv::CAP_PROP_FRAME_WIDTH),
		(int)capdev->get(cv::CAP_PROP_FRAME_HEIGHT));
	printf("Expected size: %d %d \n", refS.width, refS.height);

	cv::namedWindow("Video", 1); //identifies a window
	cv::Mat frame;

	//keypress variables
	char button = 'n';
	bool screen = false;

	//init for movement filter
	cv::Mat lastFrame;

	//variables for color shift filter
	int shift = 0;
	int shiftAmt = 5;

	for (;;) {
		*capdev >> frame; //get a new frame from the cam, treat as a stream
		if (frame.empty()) {
			printf("frame is empty\n");
			break;
		}

		//see if there is a keystroke
		char key = cv::waitKey(10);
		if (key == 'q') {
			break;
		}

		//changes button state on these keystrokes
		switch (key) {
		case 'n':
		case 'e':
		case 'g':
		case 'h':
		case 'b':
		case 'x':
		case 'y':
		case 'm':
		case 'l':
		case 'c':
		case 'p':
		case 'r':
		case 'f':
		case 'u':
		case 'a':
			button = key;
			break;
		//case i works on the last frame, so it starts by copying the current frame
		case 'i':
			button = key;
			frame.copyTo(lastFrame);
			break;
		}
		
		//screenshot handler
		if (key == 's'){
			screen = true;
		}
		else {
			screen = false;
		}

		//show normal frame with no filter
		if (button == 'n') {
			cv::imshow("Video", frame);
			button = 'n';
			if (screen == true) {
				screenshot(frame);
			}
		}

		else {

			// a keypress switches to histogram EQ  aka fake HDR
			if (button == 'a') {
				cv::Mat final;
				cv::Mat frameHSV;
				cv::Mat newHSV;
				//converting to hue/sat/val
				cv::cvtColor(frame, frameHSV, cv::COLOR_BGR2HSV);

				//run histogram
				hdrEQ(frameHSV,newHSV);

				//convert back to BGR and display
				cv::cvtColor(newHSV, final, cv::COLOR_HSV2BGR);
				cv::imshow("Video", final);
				if (screen == true) {
					screenshot(final);
				}
			}

			// u keypress switches to color shifting filter
			if (button == 'u') {
				cv::Mat colordisp;
				colorshift(frame, colordisp, shift);
				cv::imshow("Video", colordisp);
				if (screen == true) {
					screenshot(colordisp);
				}
				if (shift + shiftAmt > 200){
					shiftAmt = -shiftAmt;
				}
				else if (shift + shiftAmt < 0) {
					shiftAmt = abs(shiftAmt);
				}
				shift += shiftAmt;
			}


			//i key press switch to movement filter
			if (button == 'i') {
				cv::Mat movedisp;

				movement(frame, lastFrame, movedisp, 150);
				cv::Mat blurdisp;
				cv::imshow("Video", movedisp);
				button = 'i';
				if (screen == true) {
					screenshot(movedisp);
				}
				//save this frame as last frame
				frame.copyTo(lastFrame);

			}

			// p key press switch to pixelation filter
			if (button == 'p') {
				cv::Mat pixeldisp;
				int pixelSize = 10;
				pixelate(frame, pixeldisp,pixelSize);
				cv::imshow("Video", pixeldisp);
				button = 'p';
				if (screen == true) {
					screenshot(pixeldisp);
				}

			}



			// m key press switch to combined sobel gradient magnitude filter
			if (button == 'm'){
				cv::Mat xsobelsrc;
				cv::Mat ysobelsrc;
				cv::Mat magdisp;
				//need to generate x and y sobel src first, then call magnitude with those as params
				sobelX3x3(frame, xsobelsrc);
				sobelY3x3(frame, ysobelsrc);
				//generating magnitude
				magnitude(xsobelsrc, ysobelsrc, magdisp);
				cv::imshow("Video", magdisp);
				button = 'm';
				if (screen == true) {
					screenshot(magdisp);
				}
			}



			// c key press switch to combined sobel gradient magnitude
			else if (button == 'c') {
				cv::Mat cartdsp;
				//variables for black line sensitivity (lower for more edges)
				//and quantized layers
				int sensitivity = 50;
				int layers = 5;
				cartoon(frame, cartdsp, layers, sensitivity);
				cv::imshow("Video", cartdsp);
				button = 'c';
				if (screen == true) {
					screenshot(cartdsp);
				}

			}

			// l key press switch blur/quantize filter
			else if (button == 'l') {
				cv::Mat quantsrc;
				blurQuantize(frame, quantsrc, 4);
				cv::imshow("Video", quantsrc);
				button = 'l';
				if (screen == true) {
					screenshot(quantsrc);
				}

			}

			// x key press switch to x sobel using a 3x3 filter
			else if (button == 'x') {
				cv::Mat xsobelsrc;
				cv::Mat xsobeldisp;
				sobelX3x3(frame, xsobelsrc);
				cv::convertScaleAbs(xsobelsrc, xsobeldisp);
				cv::imshow("Video", xsobeldisp);
				button = 'x';
				if (screen == true) {
					screenshot(xsobeldisp);
				}
				
			}

			// y key press switch to y sobel using a 3x3 filter
			else if (button == 'y') {
				cv::Mat ysobelsrc;
				cv::Mat ysobeldisp;
				sobelY3x3(frame, ysobelsrc);
				cv::convertScaleAbs(ysobelsrc, ysobeldisp);
				cv::imshow("Video", ysobeldisp);
				button = 'y';
				if (screen == true) {
					screenshot(ysobeldisp);
				}

			}

			// b key press switches to gaussian blur using blur5x5 with convolution
			else if (button == 'b') {
				cv::Mat blursrc;
				blur5x5(frame, blursrc);
				cv::imshow("Video", blursrc);
				button = 'b';		
				if (screen == true) {
					screenshot(blursrc);
				}

			}


			//  e key press switches to grayscale using cvtColor
			else if (button == 'e') {

				cv::Mat graysrc;
				cvtColor(frame, graysrc, CV_16F);
				cv::imshow("Video", graysrc);
				button = 'e';
				if (screen == true) {
					screenshot(graysrc);
				}
			}


			//  h key press switches to grayscale using the average of b,g,r placed in a uchar matrix
			else if (button == 'h') {

				cv::Mat graysrc;
				grayScale(frame, graysrc);
				cv::imshow("Video", graysrc);
				button = 'h';
				if (screen == true) {
					screenshot(graysrc);
				}
			}


			//g key press switches to gradX filter (edge detection from tutorial)
			else if (button == 'g') {

				cv::Mat gradsrc;
				gradX(frame, gradsrc);
				cv::Mat displaysrc;
				cv::convertScaleAbs(gradsrc, displaysrc);
				cv::imshow("Video", displaysrc);
				button = 'g';
				if (screen == true) {
					screenshot(displaysrc);
				}
			}
		}


	}

	delete capdev;
	return 0;

}