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
	char isAntiphase = (phaseNum == 0) ? 0 : 1; // 0 - yes
	float phaseShift = float(phaseNum) / float(maxPhaseNum) + 0.5;
	uint emitter1Height = abs(emittersEdges.at(0).y - emittersEdges.at(1).y);
	uint emitter2Height = abs(emittersEdges.at(2).y - emittersEdges.at(3).y);
	float emitterHeight = (emitter1Height + emitter2Height)/2;

	float pixelsInHalfWave = (emitterHeight * (waveLen/2)) / labelLen;

	uint workAreaHeight = abs(emittersEdges.at(1).y - emittersEdges.at(2).y);
	uint numOfNodes = round(workAreaHeight / pixelsInHalfWave) - isAntiphase;

	for (int i = 0; i < numOfNodes; i++) {
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

vector<Point2i> calcObjCoordantes(Mat objMask, Rect2i roi) {
	vector<Point2i> res;
	int counter = 0;
	int x_sum = 0;
	int y_sum = 0;

	for (int x_i = roi.x+3; x_i < roi.x + roi.width-3; x_i++) {
		for (int y_i = roi.y + 10; y_i < roi.y + roi.height - 5; y_i++) {
			if (objMask.at<uchar>(Point(x_i, y_i)) != 0) {
				counter++;
				x_sum += x_i;
				y_sum += y_i;
			}
		}
	}
	if (counter != 0)
		res.push_back(Point(x_sum / counter, y_sum / counter));
	return res;
}