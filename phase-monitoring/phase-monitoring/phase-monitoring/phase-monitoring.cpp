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
    Mat background = imread("./background.jpg", IMREAD_GRAYSCALE);
    Mat frame = imread("./frame.jpg", IMREAD_GRAYSCALE);
    Mat foregroundMask, imgDiff, stabilized;
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
    goodFeaturesToTrack(background, backPts, 200, 0.3, 7);
    calcOpticalFlowPyrLK(background, frame, backPts, framePts, status, err);
    Mat transformationMatrix = estimateAffinePartial2D(backPts, framePts);
    warpAffine(frame, stabilized, transformationMatrix, background.size(),
               INTER_NEAREST | WARP_INVERSE_MAP);
    
    // Crop images
    int widthShift = round(background.size().width * 0.1);
    int heightShift = round(background.size().height * 0.1);
    Rect2f roi = Rect2f(widthShift, heightShift,
                        background.size().width - widthShift*2,
                        background.size().height - heightShift*2);
    background = background(roi);
    stabilized = stabilized(roi);

    // Make a mask containing objects as white areas
    absdiff(background, stabilized, imgDiff);
    threshold(imgDiff, foregroundMask, 150, 255, THRESH_BINARY);
    erode(foregroundMask, foregroundMask, Mat());

    namedWindow("Background", WINDOW_NORMAL);
    imshow("Background", background);

    namedWindow("Stabilized", WINDOW_NORMAL);
    imshow("Stabilized", stabilized);

    namedWindow("imgDiff", WINDOW_NORMAL);
    imshow("imgDiff", imgDiff);

    namedWindow("foregroundMask", WINDOW_NORMAL);
    imshow("foregroundMask", foregroundMask);

    waitKey(0);
    return 0;
}