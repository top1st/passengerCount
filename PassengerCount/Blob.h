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
	bool blnCurrentMatchFoundOrNewBlob;
	int frameLife;
	int deadTime;
	//
public:
	Blob(std::vector<cv::Point> _contour);
	bool combineWith(Blob blob);
	bool addIfDetectIntersects(Blob blob);
	bool isDead();
	void predictNextPosition(void);
	int updateCurrentBlobWithNew(Blob blob);
	~Blob();
};
#endif    // MY_BLOB
