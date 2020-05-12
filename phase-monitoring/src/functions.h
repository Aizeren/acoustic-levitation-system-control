#pragma once
#include <iostream>
#include <sstream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/calib3d.hpp>
#include <string>

using namespace std;
using namespace cv;

// Find top and bottom coordinates of emitters.
// Input: black-white image with emitters labels
// Output: points with edges coordinates
vector<Point2i> findEmittersEdges(Mat, vector<int>);

// Calculate coordinates of nodes
// Input: emittersEdges - vector with 4 points-emitter edges coords
// Output: vector with coords of nodes
vector<Point2i> calcNodesCoords(vector<Point2i>, float, float, uchar, uchar);

// Calculate x coordinate of emmiters axis
vector<int> findSidesOfEmitters(Mat);

vector<bool> findBusyNodes(vector<Point2i>, Mat);

void stabilizeImage();