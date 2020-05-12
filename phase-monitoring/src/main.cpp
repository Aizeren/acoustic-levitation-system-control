#include "functions.h"
#include <fstream>
int main()
{
    // initial position - in phase
    // length of wave and length of emitters label in mm
    float waveLen = 8.575, labelLen = 10;
    uchar phaseNum = 0, maxPhaseNum = 23;

    ofstream nodesStatesFile("../../resources/nodesStates.txt");
    ifstream curPhaseFile("../../resources/curPhase.txt");
    VideoCapture cap("../../resources/video.mp4");
    if (!cap.isOpened()) {
        cout << "Error opening video stream" << endl;
        return -1;
    }

    Mat background, backgroundGray, frameGray, frame;
    cap >> background;
    if (background.empty()) {
        cout << "Video initial frame is empty!" << endl;
        return -1;
    }
    cvtColor(background, backgroundGray, COLOR_BGR2GRAY);
    Mat objectsMask, imgDiff, labelMask, stabilizedFrameGray, stabilizedFrame, backgroundHSV,
        backgroundCropped, backgroundGrayCropped;
    vector<Point2f> backPts, framePts;
    vector<uchar> status;
    vector<float> err;
    vector<Point2i> emittersEdges, nodesCoordinates;
    vector<bool> isNodeBusy;
    Rect2i emittersROI, stabilizedFrameROI;
    // X coordinates of left and right sides of emitters
    vector<int> emittersSides;
    string tmp;

    if (!nodesStatesFile.is_open()) {
        cout << "Couldn't open or find nodesStates.txt file." << endl;
        return -1;
    }
    if (!curPhaseFile.is_open()) {
        cout << "Couldn't open or find curPhase.txt file." << endl;
        return -1;
    }
    nodesStatesFile.close();
    curPhaseFile.close();

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            cout << "Video frame is empty!" << endl;
            return -1;
        }
        cvtColor(frame, frameGray, COLOR_BGR2GRAY);
        curPhaseFile.open("../../resources/curPhase.txt");
        curPhaseFile >> tmp;
        phaseNum = stoi(tmp);
        curPhaseFile.close();
        // Stabilize image
        goodFeaturesToTrack(backgroundGray, backPts, 200, 0.3, 7);
        calcOpticalFlowPyrLK(backgroundGray, frameGray, backPts, framePts, status, err);
        Mat transformationMatrix = estimateAffinePartial2D(backPts, framePts);
        warpAffine(frameGray, stabilizedFrameGray, transformationMatrix, backgroundGray.size(),
            WARP_INVERSE_MAP);
        warpAffine(frame, stabilizedFrame, transformationMatrix, backgroundGray.size(),
            WARP_INVERSE_MAP);

        // Crop images
        int widthShift = round(backgroundGray.size().width * 0.05);
        int heightShift = round(backgroundGray.size().height * 0.05);
        stabilizedFrameROI = Rect2i(widthShift, heightShift,
            backgroundGray.size().width - widthShift * 2,
            backgroundGray.size().height - heightShift * 2);
        backgroundCropped = background(stabilizedFrameROI);
        backgroundGrayCropped = backgroundGray(stabilizedFrameROI);
        stabilizedFrameGray = stabilizedFrameGray(stabilizedFrameROI);
        stabilizedFrame = stabilizedFrame(stabilizedFrameROI);

        // Detect labels of emitters
        cvtColor(stabilizedFrame, backgroundHSV, COLOR_RGB2HSV);
        inRange(backgroundHSV, Scalar(120, 100, 100), Scalar(130, 255, 255), labelMask);

        erode(labelMask, labelMask, getStructuringElement(MORPH_RECT, Size(5, 5)));
        dilate(labelMask, labelMask, getStructuringElement(MORPH_RECT, Size(7, 7)));
        erode(labelMask, labelMask, Mat());

        emittersSides = findSidesOfEmitters(labelMask);
        if (emittersSides.size() != 0) {
            emittersEdges = findEmittersEdges(labelMask, emittersSides);
            emittersROI = Rect2i(Point(emittersSides.at(0), emittersEdges.at(1).y),
                Point(emittersSides.at(1), emittersEdges.at(2).y));
            // Make a mask containing objects as white areas
            absdiff(backgroundGrayCropped, stabilizedFrameGray, imgDiff);
            threshold(imgDiff, objectsMask, 254, 255, THRESH_BINARY);
            threshold(imgDiff(emittersROI), objectsMask(emittersROI), 80, 255, THRESH_BINARY);

            nodesCoordinates = calcNodesCoords(emittersEdges, waveLen, labelLen, phaseNum, maxPhaseNum);

            isNodeBusy = findBusyNodes(nodesCoordinates, objectsMask);

            nodesStatesFile.open("../../resources/nodesStates.txt", ofstream::out | ofstream::trunc);
            for (int i = 0; i < isNodeBusy.size(); i++) {
                nodesStatesFile << to_string(isNodeBusy.at(i));
            }
            nodesStatesFile.close();
            cout << int(phaseNum) << endl;
        }
        else {
            nodesStatesFile.open("../../resources/nodesStates.txt", ofstream::out | ofstream::trunc);
            nodesStatesFile << to_string(2);
            nodesStatesFile.close();
        }

        // !--------------- ONLY OUTPUTS -----------------!

        for (int i = 0; i < nodesCoordinates.size(); i++) {
            if (isNodeBusy.at(i) == true)
                circle(stabilizedFrameGray, nodesCoordinates.at(i), 3, Scalar(255), 2);
            else
                circle(stabilizedFrameGray, nodesCoordinates.at(i), 3, Scalar(0), 2);
        }

        namedWindow("stabilizedFrame", WINDOW_NORMAL);
        imshow("stabilizedFrame", stabilizedFrameGray);
        namedWindow("labelMask", WINDOW_NORMAL);
        imshow("labelMask", labelMask);
        namedWindow("objectsMask", WINDOW_NORMAL);
        imshow("objectsMask", objectsMask);

        waitKey(10);
    }

    return 0;
}