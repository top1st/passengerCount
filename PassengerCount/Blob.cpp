#include "Blob.h"


Blob::Blob(std::vector<cv::Point> _contour)
{
	currentContour = _contour;
	dbSize = contourArea(currentContour);
	currentBoundingRect = cv::boundingRect(currentContour);
	cv::Point currentCenter;
	currentCenter.x = (currentBoundingRect.x + currentBoundingRect.x + currentBoundingRect.width) / 2;
	currentCenter.y = (currentBoundingRect.y + currentBoundingRect.y + currentBoundingRect.height) / 2;

	centerPositions.push_back(currentCenter);
}


Blob::~Blob()
{
}

void Blob::predictNextPosition()
{

}
