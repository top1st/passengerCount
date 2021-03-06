// PassengerCount.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <vector>
#include "opencv2\opencv.hpp"
#include "Blob.h"

using namespace std;
using namespace cv;

std::vector<Blob> blobs;

cv::Mat structuringElement3x3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
cv::Mat structuringElement5x5 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
cv::Mat structuringElement7x7 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
cv::Mat structuringElement9x9 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(9, 9));
cv::Mat structuringElement15x15 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15));


std::vector<std::vector<cv::Point> > contours;
std::vector<std::vector<cv::Point> > convexHulls;


// global variables ///////////////////////////////////////////////////////////////////////////////
const cv::Scalar SCALAR_BLACK = cv::Scalar(0.0, 0.0, 0.0);
const cv::Scalar SCALAR_WHITE = cv::Scalar(255.0, 255.0, 255.0);
const cv::Scalar SCALAR_YELLOW = cv::Scalar(0.0, 255.0, 255.0);
const cv::Scalar SCALAR_GREEN = cv::Scalar(0.0, 200.0, 0.0);
const cv::Scalar SCALAR_RED = cv::Scalar(0.0, 0.0, 255.0);

const int INT_FONTFACE= CV_FONT_HERSHEY_SIMPLEX;
const double FONTSCALE = 0.5;// (240 * 320) / 300000.0;
const int FONTTHINKNESS = (int)std::round(FONTSCALE * 2.0);

bool blnFirstFrame = true;

int totalPass = 0;
int outPass = 0;
int inPass = 0;

void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point>> cnts, std::string strImageName, Mat &retMat = Mat())
{
		cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

		cv::drawContours(image, cnts, -1, SCALAR_WHITE, -1);

		cv::imshow(strImageName, image);

		retMat = image;
	

}

int openAndClose(Mat &fgmask, char* fgName = "default", bool showfg = false)
{
	morphologyEx(fgmask, fgmask, MORPH_OPEN, structuringElement3x3);
	morphologyEx(fgmask, fgmask, MORPH_CLOSE, structuringElement5x5);
	morphologyEx(fgmask, fgmask, MORPH_OPEN, structuringElement5x5);
	morphologyEx(fgmask, fgmask, MORPH_CLOSE, structuringElement7x7);
	morphologyEx(fgmask, fgmask, MORPH_OPEN, structuringElement7x7);
	morphologyEx(fgmask, fgmask, MORPH_CLOSE, structuringElement15x15);

	if (showfg == true) {
		imshow(fgName, fgmask);
	}

	return 1;
}

int tryAddCurBlob(Blob blob, std::vector<Blob> &curBlobs)
{
	for (auto &curBlob : curBlobs)
	{
		if (std::abs(curBlob.centerPositions.back().y - blob.centerPositions.back().y) < 20) {
			curBlob.combineWith(blob);
			return 0;
		}
	}

	curBlobs.push_back(blob);
	return 1;
}
void checkLifeCycleOfBlobs(std::vector<Blob> &existingBlobs)
{
	for (int i = 0; i < existingBlobs.size(); i++)
	{
		if (existingBlobs[i].isDead()) {
			cv::Point blobFront = existingBlobs[i].centerPositions.front();
			cv::Point blobBack = existingBlobs[i].centerPositions.back();
			if (blobFront.y > 160 && blobBack.y < 100)
			{
				cout << "Out : " << ++outPass << endl;
				//i--;
			}
			if (blobFront.y < 80 && blobBack.y > 140)
			{
				cout << "In  : " << ++inPass << endl;
				//existingBlobs.erase(existingBlobs.begin() + i);
				//i--;
			}
			existingBlobs.erase(existingBlobs.begin() + i);
			i--;
		}
	}
}

int matchCurrentFrameBlobsToExistingBlobs(std::vector<Blob> &existingBlobs, std::vector<Blob> &currentFrameBlobs)
{

	for (auto &existingBlob : existingBlobs) {

		existingBlob.blnCurrentMatchFoundOrNewBlob = false;

		existingBlob.predictNextPosition();
	}

	for (auto &blob : currentFrameBlobs)
	{
		bool isAdded = false;
		for (auto &existingBlob : existingBlobs)
		{
			if (existingBlob.addIfDetectIntersects(blob))
			{
				isAdded = true;
				break;
			}
		}
		if (isAdded == false)
		{
			cv::Point blobcenter = blob.centerPositions.front();
			if(blobcenter.y < 80 || blobcenter.y > 160)
				existingBlobs.push_back(blob);
		}
	}

	checkLifeCycleOfBlobs(existingBlobs);

	return 1;
}

int drawBlobPosition(Mat &img, Blob blob)
{
	circle(img, blob.centerPositions.back(), 3, SCALAR_RED, 3);
	rectangle(img, blob.currentBoundingRect, SCALAR_YELLOW, 3);
	
	cv::Point blobFront = blob.centerPositions.front();
	cv::Point blobBack = blob.centerPositions.back();
	if (blobFront.y > 160 && blobBack.y < 100)
	{
		polylines(img, blob.centerPositions, false, SCALAR_GREEN, 1);
	}
	if (blobFront.y < 80 && blobBack.y > 140)
	{
		polylines(img, blob.centerPositions, false, SCALAR_YELLOW, 1);
	}

	return 1;
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

	int history = 250;
	double varThreshold = 20;
	bool detectShadows = true;

	
	Ptr<BackgroundSubtractor> bg_modelMog25 = createBackgroundSubtractorMOG2(history, varThreshold, detectShadows).dynamicCast<BackgroundSubtractor>();
	Ptr<BackgroundSubtractor> bg_modelMog500 = createBackgroundSubtractorMOG2(500, varThreshold, detectShadows).dynamicCast<BackgroundSubtractor>();

	double dist2Threshold = 400.0;

	Ptr<BackgroundSubtractor> bg_modelKNN25 = createBackgroundSubtractorKNN(25, dist2Threshold, detectShadows).dynamicCast<BackgroundSubtractor>();
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
		//cout << "FrameNumber : " << frameNum << endl;
		cap >> img0;
		
		if (img0.empty())
			break;

		resize(img0, img, Size(320, 240), INTER_LINEAR);

		if (fgimgMog25.empty())
			fgimgMog25.create(img.size(), img.type());

		if (fgimgKNN25.empty())
			fgimgKNN25.create(img.size(), img.type());

		bg_modelMog25->apply(img, fgmaskMog25, update_bg_model ? -1 : 0);
		bg_modelKNN25->apply(img, fgmaskKNN25, update_bg_model ? -1 : 0);

		imshow("bgMaskStp1", fgmaskMog25);
		imshow("bgMaskKNNStep1", fgmaskKNN25);
		bitwise_and(fgmaskKNN25, fgmaskMog25, fgmaskMog25);

		if (smoothMask)
		{
			GaussianBlur(fgmaskMog25, fgmaskMog25, Size(71, 71), 3.5, 3.5);
			imshow("bgMaskSep2Blur", fgmaskMog25);
			threshold(fgmaskMog25, fgmaskMog25, 130, 255, THRESH_BINARY);
			imshow("bgMaskSep3thresh", fgmaskMog25);

			/*GaussianBlur(fgmaskKNN25, fgmaskKNN25, Size(71, 71), 3.5, 3.5);
			imshow("BlurfgmaskKNN25", fgmaskKNN25);
			threshold(fgmaskKNN25, fgmaskKNN25, 130, 255, THRESH_BINARY);
			imshow("threshfgmaskKNN25", fgmaskKNN25);*/
		}

		if (!fgmaskMog25.empty()) {

			// Removing Noise using deliat and erode.

			openAndClose(fgmaskMog25, "OPNECLOSEfgmaskMog25", true);
			
			//openAndClose(fgmaskKNN25, "OPNECLOSEfgmaskFNN25", true);


			


			cv::findContours(fgmaskMog25, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			for (int i = 0; i < contours.size(); i++)
			{
				double cntArea = contourArea(contours[i]);
				if (cntArea < 2000) 
				{
					contours.erase(contours.begin() + i);
					i--;
				}
				else
				{
					//cout << cntArea << endl;
				}
			}

			drawAndShowContours(fgmaskMog25.size(), contours, "contours");

			convexHulls.resize(contours.size());
			for (int i = 0; i < contours.size(); i++)
			{
				convexHull(Mat(contours[i]), convexHulls[i]);
			}

			std::vector<Blob> currentFrameBlobs;

			for (auto &convexH : convexHulls)
			{
				Blob possibleBlob(convexH);

				tryAddCurBlob(possibleBlob, currentFrameBlobs);

				//rrentFrameBlobs.push_back(possibleBlob);
				
				
			}

			if (blnFirstFrame == true)
			{
				blobs.clear();
				for (auto &currentFrameBlob : currentFrameBlobs)
				{
					blobs.push_back(currentFrameBlob);
				}
				blnFirstFrame = false;
			}
			else
			{
				matchCurrentFrameBlobsToExistingBlobs(blobs, currentFrameBlobs);
			}

			for( auto &possibleBlob: blobs)
				drawBlobPosition(img, possibleBlob);

			Mat convexMat;
			drawAndShowContours(fgmaskMog25.size(), convexHulls, "imgConvexHulls", convexMat);
			convexMat.copyTo(fgmaskMog25);


		}
		

		fgimgMog25 = Scalar::all(0);
		img.copyTo(fgimgMog25, fgmaskMog25);

		Mat bgimg;
		bg_modelMog25->getBackgroundImage(bgimg);

		putText(img, "In : ", Point(10, 30), INT_FONTFACE, FONTSCALE, SCALAR_RED, 2);
		putText(img, std::to_string(inPass), Point(50, 30), INT_FONTFACE, FONTSCALE, SCALAR_RED, 2);
		putText(img, "Out: ", Point(10, 50), INT_FONTFACE, FONTSCALE, SCALAR_RED, 2);
		putText(img, std::to_string(outPass), Point(50, 50), INT_FONTFACE, FONTSCALE, SCALAR_RED, 2);

		imshow("image", img);

		imshow("foreground mask", fgmaskMog25);
		imshow("foreground image", fgimgMog25);
		if (!bgimg.empty())
			imshow("mean background image", bgimg);

		char k = (char)waitKey(1);
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

