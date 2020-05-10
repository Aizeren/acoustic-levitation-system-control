#include "functions.h"

int main()
{
    // initial position - in phase
    // length of wave and length of emitters label in mm
    float waveLen = 8.575, labelLen = 10;
    uchar phaseNum = 0, maxPhaseNum = 24;
    Mat background = imread("./Resources/background.jpg", IMREAD_COLOR);
    Mat frame = imread("./Resources/frame.jpg", IMREAD_COLOR);
    Mat backgroundGray = imread("./Resources/background.jpg", IMREAD_GRAYSCALE);
    Mat frameGray = imread("./Resources/frame.jpg", IMREAD_GRAYSCALE);

    /*Mat backgroundHSV, labelMask;
    cvtColor(background, backgroundHSV, COLOR_RGB2HSV);
    namedWindow("window", WINDOW_NORMAL);
    for (int h = 60; h < 130; h += 5) {
        inRange(backgroundHSV, Scalar(h, 100, 100), Scalar(h+10, 255, 255), labelMask);
        imshow("window", labelMask);
        waitKey(0);
    }*/
    
    Mat objectsMask, imgDiff, stabilizedFrame, labelMask, stabilizedFrameGray, backgroundHSV,
        backgroundCropped, backgroundGrayCropped;
    vector<Point2f> backPts, framePts;
    vector<uchar> status;
    vector<float> err;
    vector<Point2i> emittersEdges, nodesCoordinates;
    vector<bool> isNodeBusy;
    Rect2i emittersROI, stabilizedFrameROI;
    // X coordinates of left and right sides of emitters
    vector<int> emittersSides;

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
    warpAffine(frameGray, stabilizedFrameGray, transformationMatrix, backgroundGray.size(),
        WARP_INVERSE_MAP);
    warpAffine(frame, stabilizedFrame, transformationMatrix, background.size(),
        WARP_INVERSE_MAP);

    // Crop images
    int widthShift = round(backgroundGray.size().width * 0.05);
    int heightShift = round(backgroundGray.size().height * 0.05);
    stabilizedFrameROI = Rect2i(widthShift, heightShift,
        backgroundGray.size().width - widthShift * 2,
        backgroundGray.size().height - heightShift * 2);
    backgroundCropped = background(stabilizedFrameROI);
    backgroundGrayCropped = backgroundGray(stabilizedFrameROI);
    stabilizedFrame = stabilizedFrame(stabilizedFrameROI);
    stabilizedFrameGray = stabilizedFrameGray(stabilizedFrameROI);

    // Detect labels of emitters
    cvtColor(backgroundCropped, backgroundHSV, COLOR_RGB2HSV);
    inRange(backgroundHSV, Scalar(120,100, 100), Scalar(130, 255, 255), labelMask);

    dilate(labelMask, labelMask, Mat());
    erode(labelMask, labelMask, Mat());

    emittersSides = findSidesOfEmitters(labelMask);
    emittersEdges = findEmittersEdges(labelMask, emittersSides);
    emittersROI = Rect2i(Point(emittersSides.at(0), emittersEdges.at(1).y),
                         Point(emittersSides.at(1), emittersEdges.at(2).y));
    // Make a mask containing objects as white areas
    absdiff(backgroundGrayCropped, stabilizedFrameGray, imgDiff);
    threshold(imgDiff, objectsMask, 254, 255, THRESH_BINARY);
    threshold(imgDiff(emittersROI), objectsMask(emittersROI), 70, 255, THRESH_BINARY);

    nodesCoordinates = calcNodesCoords(emittersEdges, waveLen, labelLen, phaseNum, maxPhaseNum);

    isNodeBusy = findBusyNodes(nodesCoordinates, objectsMask);

    // !--------------- ONLY OUTPUTS -----------------!

    for (int i = 0; i < isNodeBusy.size(); i++)
        cout << "isNodeBusy[" << i << "] = " << isNodeBusy.at(i) << endl;

    rectangle(stabilizedFrame, emittersROI, Scalar(0, 0, 255), 3);
    for (int i = 0; i < nodesCoordinates.size(); i++) {
        circle(stabilizedFrame, nodesCoordinates.at(i), 4, Scalar(255, 0, 0), 2);
    }

    cvtColor(labelMask, labelMask, COLOR_GRAY2BGR);
    for (int i = 0; i < emittersEdges.size(); i++) {
        circle(labelMask, emittersEdges.at(i), 2, Scalar(255, 0, 0), 2);
    }

    /*namedWindow("Background", WINDOW_NORMAL);
    imshow("Background", backgroundCropped);*/

    namedWindow("stabilizedFrame", WINDOW_NORMAL);
    imshow("stabilizedFrame", stabilizedFrame);

    namedWindow("imgDiff", WINDOW_NORMAL);
    imshow("imgDiff", imgDiff);

    namedWindow("objectsMask", WINDOW_NORMAL);
    imshow("objectsMask", objectsMask);

    namedWindow("labelMask", WINDOW_NORMAL);
    imshow("labelMask", labelMask);

    waitKey(0);
    return 0;
}