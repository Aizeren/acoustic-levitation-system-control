#include "functions.h"
#include <fstream>
#include <chrono>

int main()
{
    // initial position - in phase
    // length of wave and length of emitters label in mm
    float waveLen = 8.575, labelLen = 10;
    uchar phaseNum = 0, maxPhaseNum = 23;

    ofstream nodesStatesFile("../../resources/nodesStates.txt");
    ofstream logFile("../../resources/nodesCoordinates.csv");
    logFile.open("../../resources/nodesCoordinates.csv", ofstream::out | ofstream::trunc);
    logFile.close();
    logFile.open("../../resources/nodesCoordinates.csv", ofstream::out | ofstream::app);
    ifstream curPhaseFile("../../resources/curPhase.txt");

    ofstream logObjFile("../../resources/objCoordinates.csv");
    logObjFile.open("../../resources/objCoordinates.csv", ofstream::out | ofstream::trunc);
    logObjFile.close();
    

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

    if (!logFile.is_open()) {
        cout << "Couldn't open or find nodesCoordinates.csv file." << endl;
        return -1;
    }

    //namedWindow("stabilizedFrame", WINDOW_NORMAL);
    namedWindow("stabilizedFrameWithNodes", WINDOW_NORMAL);
    namedWindow("labelMask", WINDOW_NORMAL);
   // namedWindow("objectsMask", WINDOW_NORMAL);
    /*namedWindow("background", WINDOW_NORMAL);
    namedWindow("frame", WINDOW_NORMAL);
    namedWindow("imgDiff", WINDOW_NORMAL);*/

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
            //cout << "sides size: " << emittersSides.size() << endl;
            if (emittersEdges.size() != 0) {
                //cout << "eges size: " << emittersEdges.size() << endl;
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

                nodesCoordinates = calcNodesCoords(emittersEdges, waveLen, labelLen, phaseNum, maxPhaseNum);

                /*logFile << to_string(chrono::duration_cast<chrono::milliseconds>(
                    chrono::system_clock::now().time_since_epoch()).count() - init_time) << ";";
                //cout << to_string(chrono::duration_cast<chrono::milliseconds>(
                //    chrono::system_clock::now().time_since_epoch()).count() - init_time) << endl;
                for (int i = 0; i < nodesCoordinates.size(); i++) {
                    logFile << to_string(nodesCoordinates.at(i).y) << ";";
                    //cout << to_string(nodesCoordinates.at(i).y) << endl;
                }
                logFile << "\n";*/

                objCoordinates = calcObjCoordantes(objectsMask, emittersROI);
                

                isNodeBusy = findBusyNodes(nodesCoordinates, objectsMask);

                nodesStatesFile.open("../../resources/nodesStates.txt", ofstream::out | ofstream::trunc);
                //nodesStatesFile << to_string(1);
                for (int i = 0; i < isNodeBusy.size(); i++) {
                    nodesStatesFile << to_string(isNodeBusy.at(i));
                }
                //nodesStatesFile << to_string(1);
                nodesStatesFile.close();

                logObjFile.open("../../resources/objCoordinates.csv", ofstream::out | ofstream::app);
                logObjFile << to_string(chrono::duration_cast<chrono::milliseconds>(
                    chrono::system_clock::now().time_since_epoch()).count() - init_time) << ";";
                for (int i = 0; i < objCoordinates.size(); i++) {
                    logObjFile << to_string(objCoordinates.at(i).x) << ";";
                    logObjFile << to_string(objCoordinates.at(i).y) << ";";
                    //cout << "x: " << objCoordinates.at(i).x;
                    //cout << "y: " << objCoordinates.at(i).y;
                }
                logObjFile << "\n"; logObjFile.close();
            } else {
                nodesStatesFile.open("../../resources/nodesStates.txt", ofstream::out | ofstream::trunc);
                nodesStatesFile << to_string(2);
                nodesStatesFile.close();
            }
        }
        

        // !--------------- ONLY OUTPUTS -----------------!

        /*imshow("background", background);
        imshow("frame", frame);
        imshow("stabilizedFrame", stabilizedFrame);*/
        for (int i = 0; i < nodesCoordinates.size(); i++) {
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
        imshow("labelMask", labelMask);
        //imshow("objectsMask", objectsMask);
        /*imshow("imgDiff", imgDiff);*/
        
        waitKey(2);
    }

    
    return 0;
}