#include "functions.h"
#include <fstream>
#include <chrono>

int main()
{
    // wavelenght and height of emitters label in mm
    float waveLen = 8.575, labelHeight = 10;
    uchar phaseNum = 0, maxPhaseNum = 23;

    ofstream nodesStatesFile("../../resources/nodesStates.txt");
    ifstream curPhaseFile("../../resources/curPhase.txt");    

    //VideoCapture cap("../../resources/video_final_2.mp4");
    VideoCapture cap(0);
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
    Mat objectsMask, imgDiff, labelMask, stabilizedFrameGray, stabilizedFrame, stabilizedFrameHSV,
        backgroundCropped, backgroundGrayCropped;
    vector<Point2f> backPts, framePts;
    vector<uchar> status;
    vector<float> err;
    vector<Point2i> emittersEdges, nodesCoordinates, objCoordinates;
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

    namedWindow("stabilizedFrameWithNodes", WINDOW_NORMAL);
    namedWindow("labelMask", WINDOW_NORMAL);

    goodFeaturesToTrack(backgroundGray, backPts, 200, 0.3, 7);
    uint64 init_time = chrono::duration_cast<chrono::milliseconds>(
        chrono::system_clock::now().time_since_epoch()).count();
    cout << init_time << endl;

    while (true) {
        if (!cap.read(frame)) {
            cout << "Video frame is empty!" << endl;
            return -1;
        }
        cvtColor(frame, frameGray, COLOR_BGR2GRAY);
        
        // Stabilize image
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
        backgroundGrayCropped = backgroundGray(stabilizedFrameROI);
        stabilizedFrameGray = stabilizedFrameGray(stabilizedFrameROI);
        stabilizedFrame = stabilizedFrame(stabilizedFrameROI);

        // Detect labels of emitters
        cvtColor(stabilizedFrame, stabilizedFrameHSV, COLOR_RGB2HSV);
        inRange(stabilizedFrameHSV, Scalar(120, 100, 100), Scalar(130, 255, 255), labelMask);

        erode(labelMask, labelMask, getStructuringElement(MORPH_RECT, Size(5, 5)));
        dilate(labelMask, labelMask, getStructuringElement(MORPH_RECT, Size(7, 7)));
        erode(labelMask, labelMask, Mat());

        emittersSides = findSidesOfEmitters(labelMask);
        if (emittersSides.size() != 0) {
            emittersEdges = findEmittersEdges(labelMask, emittersSides);
            if (emittersEdges.size() != 0) {
                emittersROI = Rect2i(Point(emittersSides.at(0), emittersEdges.at(1).y),
                    Point(emittersSides.at(1), emittersEdges.at(2).y));
                // Make a mask containing objects as white areas
                absdiff(backgroundGrayCropped, stabilizedFrameGray, imgDiff);
                threshold(imgDiff, objectsMask, 254, 255, THRESH_BINARY);
                threshold(imgDiff(emittersROI), objectsMask(emittersROI), 80, 255, THRESH_BINARY);

                curPhaseFile.open("../../resources/curPhase.txt");
                curPhaseFile >> tmp;
                if (tmp != "")
                    phaseNum = stoi(tmp);
                else phaseNum = 0;
                curPhaseFile.close();

                nodesCoordinates = calcNodesCoords(emittersEdges, waveLen, labelHeight, phaseNum, maxPhaseNum);

                objCoordinates = calcObjCoordantes(objectsMask, emittersROI);

                isNodeBusy = findBusyNodes(nodesCoordinates, objectsMask);

                nodesStatesFile.open("../../resources/nodesStates.txt", ofstream::out | ofstream::trunc);
                for (int i = 0; i < isNodeBusy.size(); i++) {
                    nodesStatesFile << to_string(isNodeBusy.at(i));
                }
                nodesStatesFile.close();
            } else {
                nodesStatesFile.open("../../resources/nodesStates.txt", ofstream::out | ofstream::trunc);
                nodesStatesFile << to_string(2);
                nodesStatesFile.close();
            }
        }

        // !--------------- ONLY OUTPUTS -----------------!

        /*for (int i = 0; i < nodesCoordinates.size(); i++) {
            if(isNodeBusy.at(i) == 0)
                circle(stabilizedFrame, nodesCoordinates.at(i), 2, Scalar(0, 0, 255), 1);
            else
                circle(stabilizedFrame, nodesCoordinates.at(i), 2, Scalar(0, 255, 0), 1);
        }
        cvtColor(labelMask, labelMask, COLOR_GRAY2BGR);
        for (int i = 0; i < emittersEdges.size(); i++) {
            circle(stabilizedFrame, emittersEdges.at(i), 2, Scalar(255, 0, 0), 1);
            circle(labelMask, emittersEdges.at(i), 2, Scalar(0, 0, 255), 1);
        }
        imshow("stabilizedFrameWithNodes", stabilizedFrame);
        imshow("labelMask", labelMask);*/
        
        waitKey(2);
    }

    return 0;
}