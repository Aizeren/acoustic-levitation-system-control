#include <iostream>
#include <sstream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/calib3d.hpp>
#include "phase-monitoring.h"

using namespace std;
using namespace cv;

int main()
{
    Mat background = imread("./background.jpg", IMREAD_COLOR);
    Mat frame = imread("./frame.jpg", IMREAD_COLOR);
    Mat backgroundGray = imread("./background.jpg", IMREAD_GRAYSCALE);
    Mat frameGray = imread("./frame.jpg", IMREAD_GRAYSCALE);
    Mat objectsMask, imgDiff, stabilized, labelMask, stabilizedGray, backgroundHSV;
    vector<Point2f> backPts, framePts;
    vector<uchar> status;
    vector<float> err;

    if (!background.data) {
        cout << "Couldn't open or find background image." << endl;
        return -1;
    }
    if (!frame.data) {
        cout << "Couldn't open or find frame image." << endl;
        return -1;
    }

    // Stabilize image
    goodFeaturesToTrack(backgroundGray, backPts, 200, 0.3, 7);
    calcOpticalFlowPyrLK(backgroundGray, frameGray, backPts, framePts, status, err);
    Mat transformationMatrix = estimateAffinePartial2D(backPts, framePts);
    warpAffine(frameGray, stabilizedGray, transformationMatrix, backgroundGray.size(),
               INTER_NEAREST | WARP_INVERSE_MAP);
    warpAffine(frame, stabilized, transformationMatrix, background.size(),
        INTER_NEAREST | WARP_INVERSE_MAP);

    // Crop images
    int widthShift = round(backgroundGray.size().width * 0.1);
    int heightShift = round(backgroundGray.size().height * 0.1);
    Rect2f roi = Rect2f(widthShift, heightShift,
                        backgroundGray.size().width - widthShift*2,
                        backgroundGray.size().height - heightShift*2);
    background = background(roi);
    backgroundGray = backgroundGray(roi);
    stabilized = stabilized(roi);
    stabilizedGray = stabilizedGray(roi);

    // Detect labels of emmiters
    cvtColor(background, backgroundHSV, COLOR_RGB2HSV);
    inRange(backgroundHSV, Scalar(30, 150, 150), Scalar(60, 255, 255), labelMask);

    // Make a mask containing objects as white areas
    absdiff(backgroundGray, stabilizedGray, imgDiff);
    threshold(imgDiff, objectsMask, 100, 255, THRESH_BINARY);
    erode(objectsMask, objectsMask, Mat());

    /*namedWindow("Background", WINDOW_NORMAL);
    imshow("Background", background);

    namedWindow("Stabilized", WINDOW_NORMAL);
    imshow("Stabilized", stabilized);

    namedWindow("imgDiff", WINDOW_NORMAL);
    imshow("imgDiff", imgDiff);*/

    namedWindow("objectsMask", WINDOW_NORMAL);
    imshow("objectsMask", objectsMask);

    namedWindow("labelMask", WINDOW_NORMAL);
    imshow("labelMask", labelMask);

    waitKey(0);
    return 0;
}