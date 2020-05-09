#include "functions.h"

#include <ctime>

int main()
{
    // initial position - in phase
    // length of wave and length of emitters label in mm
    float waveLen = 8.575, labelLen = 10;
    uchar phaseNum = 0, maxPhaseNum = 24;
    Mat background = imread("./background.jpg", IMREAD_COLOR);
    Mat frame = imread("./frame.jpg", IMREAD_COLOR);
    Mat backgroundGray = imread("./background.jpg", IMREAD_GRAYSCALE);
    Mat frameGray = imread("./frame.jpg", IMREAD_GRAYSCALE);
    Mat objectsMask, imgDiff, stabilized, labelMask, stabilizedGray, backgroundHSV;
    vector<Point2f> backPts, framePts;
    vector<uchar> status;
    vector<float> err;
    vector<Point2i> emittersEdges, nodesCoordinates;

    if (!background.data) {
        cout << "Couldn't open or find background image." << endl;
        return -1;
    }
    if (!frame.data) {
        cout << "Couldn't open or find frame image." << endl;
        return -1;
    }

    for (int o = 0; o < 10; o++) {
        int startTime = clock();
        // Stabilize image
        goodFeaturesToTrack(backgroundGray, backPts, 200, 0.3, 7);
        calcOpticalFlowPyrLK(backgroundGray, frameGray, backPts, framePts, status, err);
        Mat transformationMatrix = estimateAffinePartial2D(backPts, framePts);
        warpAffine(frameGray, stabilizedGray, transformationMatrix, backgroundGray.size(),
            WARP_INVERSE_MAP);
        warpAffine(frame, stabilized, transformationMatrix, background.size(),
            WARP_INVERSE_MAP);

        // Crop images
        int widthShift = round(backgroundGray.size().width * 0.05);
        int heightShift = round(backgroundGray.size().height * 0.05);
        Rect2f roi = Rect2f(widthShift, heightShift,
            backgroundGray.size().width - widthShift * 2,
            backgroundGray.size().height - heightShift * 2);
        /*background = background(roi);
        backgroundGray = backgroundGray(roi);
        stabilized = stabilized(roi);
        stabilizedGray = stabilizedGray(roi);*/

        // Detect labels of emitters
        cvtColor(background, backgroundHSV, COLOR_RGB2HSV);
        inRange(backgroundHSV, Scalar(30, 170, 160), Scalar(60, 255, 255), labelMask);

        dilate(labelMask, labelMask, Mat());
        erode(labelMask, labelMask, Mat());

        // Make a mask containing objects as white areas
        absdiff(backgroundGray, stabilizedGray, imgDiff);
        threshold(imgDiff, objectsMask, 100, 255, THRESH_BINARY);
        erode(objectsMask, objectsMask, getStructuringElement(MORPH_RECT, Size(4, 4)));

        emittersEdges = findEmittersEdges(labelMask);

        nodesCoordinates = calcNodesCoords(emittersEdges, waveLen, labelLen, phaseNum, maxPhaseNum);
        cout << clock() - startTime << endl;
    }

// !--------------- ONLY OUTPUTS -----------------!
    for (int i = 0; i < nodesCoordinates.size(); i++) {
        circle(stabilized, nodesCoordinates.at(i), 4, Scalar(255, 0, 0), 2);
    }

    cvtColor(labelMask, labelMask, COLOR_GRAY2BGR);
    for (int i = 0; i < emittersEdges.size(); i++) {
        circle(labelMask, emittersEdges.at(i), 2, Scalar(255, 0, 0), 2);
    }

    /*namedWindow("Background", WINDOW_NORMAL);
    imshow("Background", background);*/

    namedWindow("Stabilized", WINDOW_NORMAL);
    imshow("Stabilized", stabilized);

   /* namedWindow("imgDiff", WINDOW_NORMAL);
    imshow("imgDiff", imgDiff);

    namedWindow("objectsMask", WINDOW_NORMAL);
    imshow("objectsMask", objectsMask);

    namedWindow("labelMask", WINDOW_NORMAL);
    imshow("labelMask", labelMask);*/

    waitKey(0);
    return 0;
}