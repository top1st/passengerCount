// PassengerCount.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include "opencv2\opencv.hpp"
using namespace std;
using namespace cv;

cv::Mat structuringElement3x3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
cv::Mat structuringElement5x5 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
cv::Mat structuringElement7x7 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
cv::Mat structuringElement15x15 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15));


std::vector<std::vector<cv::Point> > contours;
std::vector<std::vector<cv::Point> > convexHulls;


// global variables ///////////////////////////////////////////////////////////////////////////////
const cv::Scalar SCALAR_BLACK = cv::Scalar(0.0, 0.0, 0.0);
const cv::Scalar SCALAR_WHITE = cv::Scalar(255.0, 255.0, 255.0);
const cv::Scalar SCALAR_YELLOW = cv::Scalar(0.0, 255.0, 255.0);
const cv::Scalar SCALAR_GREEN = cv::Scalar(0.0, 200.0, 0.0);
const cv::Scalar SCALAR_RED = cv::Scalar(0.0, 0.0, 255.0);


void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point>> cnts, std::string strImageName)
{
	cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

	cv::drawContours(image, cnts, -1, SCALAR_WHITE, -1);

	cv::imshow(strImageName, image);

}

int start() 
{
	VideoCapture cap;
	bool update_bg_model = true;
	bool smoothMask = true;

	char* videoName = "MyVideo2.avi";
	string videoPath = "./video/";
	videoPath.append(videoName);

	if (cap.open(videoPath) == false) {
		cout << "Cannot open video file\n" << endl;
	}

	int history = 25;
	double varThreshold = 16;
	bool detectShadows = true;

	
	Ptr<BackgroundSubtractor> bg_modelMog25 = createBackgroundSubtractorMOG2(history, varThreshold, detectShadows).dynamicCast<BackgroundSubtractor>();
	Ptr<BackgroundSubtractor> bg_modelMog500 = createBackgroundSubtractorMOG2(500, varThreshold, detectShadows).dynamicCast<BackgroundSubtractor>();

	double dist2Threshold = 400.0;

	Ptr<BackgroundSubtractor> bg_modelKNN25 = createBackgroundSubtractorKNN(history, dist2Threshold, detectShadows).dynamicCast<BackgroundSubtractor>();
	Ptr<BackgroundSubtractor> bg_modelKNN500 = createBackgroundSubtractorKNN(500, dist2Threshold, detectShadows).dynamicCast<BackgroundSubtractor>();

	Mat img0, img;
	Mat fgmaskMog25, fgmaskMog500, fgmaskKNN25, fgmaskKNN500;
	Mat	fgimgMog25,  fgimgMog500, fgimgKNN25, fgimgKNN500;

	int width = cap.get(CAP_PROP_FRAME_WIDTH);
	int height = cap.get(CAP_PROP_FRAME_HEIGHT);

	double formatvalue = cap.get(CAP_PROP_FORMAT);

	//fgmaskKNN25.create(Size(width, height), )


	for (int frameNum = 0; ; frameNum++)
	{
		cout << frameNum << endl;
		cap >> img0;
		
		if (img0.empty())
			break;

		resize(img0, img, Size(320, 240), INTER_LINEAR);

		if (fgimgMog25.empty())
			fgimgMog25.create(img.size(), img.type());

		bg_modelMog25->apply(img, fgmaskMog25, update_bg_model ? -1 : 0);

		if (smoothMask)
		{
			GaussianBlur(fgmaskMog25, fgmaskMog25, Size(11, 11), 3.5, 3.5);
			threshold(fgmaskMog25, fgmaskMog25, 10, 255, THRESH_BINARY);
		}

		if (!fgmaskMog25.empty()) {

			// Removing Noise using deliat and erode.

			morphologyEx(fgmaskMog25, fgmaskMog25, MORPH_OPEN, structuringElement3x3);
			morphologyEx(fgmaskMog25, fgmaskMog25, MORPH_CLOSE, structuringElement5x5);
			morphologyEx(fgmaskMog25, fgmaskMog25, MORPH_OPEN, structuringElement5x5);
			morphologyEx(fgmaskMog25, fgmaskMog25, MORPH_CLOSE, structuringElement7x7);
			morphologyEx(fgmaskMog25, fgmaskMog25, MORPH_OPEN, structuringElement7x7);
			morphologyEx(fgmaskMog25, fgmaskMog25, MORPH_CLOSE, structuringElement15x15);


			cv::findContours(fgmaskMog25, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			for (int i = 0; i < contours.size(); i++)
			{
				double cntArea = contourArea(contours[i]);
				if (cntArea < 100) 
				{
					contours.
				}
			}

			drawAndShowContours(fgmaskMog25.size(), contours, "contours");

			convexHulls.resize(contours.size());
			for (int i = 0; i < contours.size(); i++)
			{
				convexHull(Mat(contours[i]), convexHulls[i]);
			}

			drawAndShowContours(fgmaskMog25.size(), convexHulls, "imgConvexHulls");

			


		}
		

		fgimgMog25 = Scalar::all(0);
		img.copyTo(fgimgMog25, fgmaskMog25);

		Mat bgimg;
		bg_modelMog25->getBackgroundImage(bgimg);

		imshow("image", img);
		imshow("foreground mask", fgmaskMog25);
		imshow("foreground image", fgimgMog25);
		if (!bgimg.empty())
			imshow("mean background image", bgimg);

		char k = (char)waitKey(30);
		if (k == 27) break;
		if (k == ' ')
		{
			update_bg_model = !update_bg_model;
			if (update_bg_model)
				printf("Background update is on\n");
			else
				printf("Background update is off\n");
		}

	}

	cap.release();

	return 1;
	
}

int main()
{
	start();
    return 0;
}

