#include "functions.h"

vector<int> findSidesOfEmitters(Mat mask) {
	int leftX = -1, rightX = -1;
	Mat col;

	for (int x = 0; x < mask.size().width-1; x++) {
		col = mask(Range::all(), Range(x, x+1)).clone();
		for (int y = 0; y < mask.size().height; y++) {
			if (col.at<uchar>(Point(0, y)) != 0)
				rightX = x;
			if (col.at<uchar>(Point(0, y)) != 0 && leftX == -1)
				leftX = x;
		}
	}

	return vector<int>{leftX, rightX};
}

vector<Point2i> findEmittersEdges(Mat mask, vector<int> emitterSides) {
	vector<Point2i> res(4, Point2i(0, 0));
	uchar edgeCounter = 0, curColor, nextColor;
	Mat col;
	uint xCenter = round(emitterSides.at(0) + float(emitterSides.at(1) - emitterSides.at(0)) / 2);
	for (int x = xCenter; x < mask.size().width - 1; x++) {
		col = mask(Range::all(), Range(x, x + 1)).clone();
		edgeCounter = 0;
		for (int y = 0; y < mask.size().height - 1; y++) {
			curColor = col.at<uchar>(Point(0, y));
			nextColor = col.at<uchar>(Point(0, y + 1));
			if (curColor != nextColor) {
				edgeCounter++;
				if (edgeCounter > 4)
					break;
				res.at(edgeCounter - 1) = Point(x, y);
			}
		}
		if (edgeCounter == 4)
			break;
	}

	return res;
}

vector<Point2i> calcNodesCoords(vector<Point2i> emittersEdges, float waveLen, float labelLen, uchar phaseNum, uchar maxPhaseNum) {
	vector<Point2i> res;
	int xcoord = emittersEdges.at(0).x;
	int ytop = emittersEdges.at(1).y;
	int ybottom = emittersEdges.at(2).y;
	char isPhase = (phaseNum == 0) ? 0 : 1;
	float phaseShift = (phaseNum == maxPhaseNum) ? 0 : (phaseNum / maxPhaseNum + 0.5);
	uint emitter1Height = abs(emittersEdges.at(0).y - emittersEdges.at(1).y);
	uint emitter2Height = abs(emittersEdges.at(2).y - emittersEdges.at(3).y);
	float emitterHeight = (emitter1Height + emitter2Height)/2;

	float pixelsInHalfWave = (emitterHeight * (waveLen/2)) / labelLen;

	uint workAreaHeight = waveLen * round(abs(emittersEdges.at(1).y - emittersEdges.at(2).y) / waveLen);
	uint numOfNodes = round(workAreaHeight / pixelsInHalfWave) - isPhase;

	for (int i = isPhase; i < numOfNodes + isPhase; i++) {
		res.push_back(Point2i(xcoord, round(ytop + i * pixelsInHalfWave + phaseShift * pixelsInHalfWave)));
	}

	return res;
}

vector<bool> findBusyNodes(vector<Point2i> nodesCoordinates, Mat objectsMask) {
	vector<bool> res(nodesCoordinates.size(), false);
	short delta = 2;
	Rect2i nodeRoi;
	Mat maskCropped;

	for (int i = 0; i < nodesCoordinates.size(); i++) {
		nodeRoi = Rect2i(Point(0, nodesCoordinates.at(i).y - delta),
						 Point(objectsMask.size().width, nodesCoordinates.at(i).y + delta));
		maskCropped = objectsMask(nodeRoi);
		for (int x = 0; x < maskCropped.size().width; x++) {
			for (int y = 0; y < maskCropped.size().height; y++)
				if (maskCropped.at<uchar>(Point(x, y)) != 0) {
					res.at(i) = true;
					break;
				}
			if (res.at(i) == true)
				break;
		}
	}

	return res;
}