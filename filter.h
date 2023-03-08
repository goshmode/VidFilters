#pragma once
//James Marcel
//filter library header

int gradX(cv::Mat &src, cv::Mat &dst);
int grayScale(cv::Mat &src, cv::Mat &dst);
int blur5x5(cv::Mat &src, cv::Mat &dst);
int sobelX3x3(cv::Mat &src, cv::Mat &dst);
int sobelY3x3(cv::Mat &src, cv::Mat &dst);
int magnitude(cv::Mat &sx, cv::Mat &sy, cv::Mat& dst);
int blurQuantize(cv::Mat &src, cv::Mat &dst, int levels);
int cartoon(cv::Mat &src, cv::Mat &dst, int levels, int magThreshold);
int pixelate(cv::Mat& src, cv::Mat& dst, int scale);
int movement(cv::Mat& src, cv::Mat &last, cv::Mat& dst, int sens);
int colorshift(cv::Mat& src, cv::Mat& dst, int shift);
int hdrEQ(cv::Mat& src, cv::Mat& dst);