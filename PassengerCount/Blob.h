#pragma once
#ifndef MY_BLOB
#define MY_BLOB
#include "opencv2\opencv.hpp"
#include <vector>
#include <iostream>

using namespace cv;
using namespace std;

class Blob
{

public:
	// member variables
	std::vector<cv::Point> currentContour;
	cv::Rect currentBoundingRect;
	std::vector<cv::Point> centerPositions;
	double dbSize;

	//
public:
	Blob(std::vector<cv::Point> _contour);
	void predictNextPosition(void);
	~Blob();
};
#endif    // MY_BLOB
