#include "Blob.h"


Blob::Blob(std::vector<cv::Point> _contour)
{
	currentContour = _contour;
	dbSize = contourArea(currentContour);
	currentBoundingRect = cv::boundingRect(currentContour);
	cv::Point currentCenter;
	currentCenter.x = (currentBoundingRect.x + currentBoundingRect.x + currentBoundingRect.width) / 2;
	currentCenter.y = (currentBoundingRect.y + currentBoundingRect.y + currentBoundingRect.height) / 2;

	frameLife = 1;
	deadTime = 0;
	blnCurrentMatchFoundOrNewBlob = false;

	centerPositions.push_back(currentCenter);
}



Blob::~Blob()
{
}



bool Blob::combineWith(Blob blob)
{
	currentContour.insert(currentContour.end(), blob.currentContour.begin(), blob.currentContour.end());

	dbSize = contourArea(currentContour);
	currentBoundingRect = cv::boundingRect(currentContour);
	cv::Point currentCenter;
	currentCenter.x = (currentBoundingRect.x + currentBoundingRect.x + currentBoundingRect.width) / 2;
	currentCenter.y = (currentBoundingRect.y + currentBoundingRect.y + currentBoundingRect.height) / 2;

	frameLife = 1;
	deadTime = 0;
	blnCurrentMatchFoundOrNewBlob = false;

	centerPositions.push_back(currentCenter);


	return true;
}

int Blob::updateCurrentBlobWithNew(Blob blob)
{
	currentContour = blob.currentContour;
	dbSize = contourArea(currentContour);
	currentBoundingRect = cv::boundingRect(currentContour);
	cv::Point currentCenter;
	currentCenter.x = (currentBoundingRect.x + currentBoundingRect.x + currentBoundingRect.width) / 2;
	currentCenter.y = (currentBoundingRect.y + currentBoundingRect.y + currentBoundingRect.height) / 2;

	frameLife = 1;
	deadTime = 0;
	blnCurrentMatchFoundOrNewBlob = false;

	centerPositions.push_back(currentCenter);
	return 1;
}

bool Blob::addIfDetectIntersects(Blob blob)
{
	Rect intersectsRect = currentBoundingRect & blob.currentBoundingRect;
	cv::Point frontPos = centerPositions.front();
	cv::Point backPos = centerPositions.back();
	
	bool updateFlag = false;

	if (intersectsRect.area() * 4 > currentBoundingRect.area()) 
	{
		if (frontPos.y < 80)
		{
			if (blob.centerPositions.back().y > backPos.y)
			{
				updateFlag = true;
			}
		}
		else if (frontPos.y > 160)
		{
			if (blob.centerPositions.back().y < backPos.y)
			{
				updateFlag = true;
			}
		}
		if (updateFlag == true)
		{
			updateCurrentBlobWithNew(blob);
		}
		return true;
		
	}

	
	return updateFlag;
}

bool Blob::isDead()
{
	if (++deadTime > 8) {
		return true;
	}
	else {
		return false;
	}
}

void Blob::predictNextPosition()
{
	int postLength = centerPositions.size();

}
