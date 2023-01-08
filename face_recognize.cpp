#include <iostream>
#include <sstream>
#include <conio.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#include "opencv2/face.hpp"
#include "drawLandmarks.hpp"

using namespace std;
using namespace cv;
using namespace cv::face;

// �������� ������������� �������� 
float precision = 0.1;

// ��������� ��� �������� ���� 
struct FaceData {
    string name;        // ��� ���������� ���� 
    float mark_27_8;    // �������� �������� ����� 27 � 8 2D-�������
};

// ������ �������� ��� (���� ��� � ���������)
vector<FaceData> facesdata;

// ��������� ������: �������������, ���������
int mode;

int chMode(int mode)
    /* 
        ����� ������
    */
{
    if (mode == 1)
        cout << "-> ����� ������� �� 1 - �������������!\n\n";
    else
        cout << "-> ����� ������� �� 2 - ���� ���� � ����!\n";

    return mode;
}

// ���� ������
int ESC = 27;   // ����� �� ���������
int TAB = 9;    // ����� ������

bool isFaceStored(float mark_27_8)
{   
    // ���� ����������� ���� � ����
    bool isFinded = false; 
    
    for (int i = 0; i < facesdata.size(); i++)
    {
        // ��������� ������� �������� � ������ ��������
        if (abs(mark_27_8 - facesdata[i].mark_27_8) < precision)
        {
            isFinded = true;
        }
    }
    return isFinded;
}

FaceData* getFace(float mark_27_8=0, string name="")
    /* 
        ���������� ��������� �� ������ ���� �� ����
    */
{
    // ����� ������
    int mode = NULL; 

    if (mark_27_8)
        // ����� ���� �� ��������
        mode = 1;
    else if (name.size())
        // ����� ���� �� �����
        mode = 2;

    // ������������ ������
    FaceData *face = nullptr;

    switch (mode)
    {
        case 1: 
            // ����� �� �������� ����
            for (int i = 0; i < facesdata.size(); i++)
            {
                if (mark_27_8 == facesdata[i].mark_27_8)
                {
                    face = &facesdata[i];
                }
            }
            break;

        case 2:
            // ����� ���� �� ����� 
            for (int i = 0; i < facesdata.size(); i++)
            {
                if (name == facesdata[i].name)
                {
                    face = &facesdata[i];
                }
            }
            break;
    }
    return face;
}

FaceData* faceIdentified(float mark_27_8, float precision)
/*
    ���������� ��������� ���� �� ����
*/
{
    FaceData* face = nullptr;

    for (int i = 0; i < facesdata.size(); i++)
    {
        if (abs(mark_27_8 - facesdata[i].mark_27_8) < precision)
        {
            face = &facesdata[i];
        }
    }
    return face;
}


int main(int argc, char **argv)
{
    // ��������� � �������
    setlocale(LC_ALL, ""); 
  
    cout << "�������� ����� - ������� 'TAB' \n";
    cout << "����� �� ��������� - ������� 'ESC' \n\n";
    
    // ����� ������ ���������
    do {
        string str;
        stringstream ss;
        cout << "1. ������������� ���� \n";
        cout << "2. ��������� �������� ���� � ����������� � ���� \n\n";        
        cout << "������� ����� ������ ���������: ";
        cin >> str;
        // � ������ ���� ������� �����
        ss << str;
        ss >> mode;
    } while (mode != 1 && mode != 2);

    // Load Face Detector
    CascadeClassifier faceDetector("haarcascade_frontalface_alt2.xml");
    // Create an instance of Facemark
    Ptr<Facemark> facemark = FacemarkLBF::create();
    // Load landmark detector
    facemark->loadModel("lbfmodel.yaml");
    // Set up webcam for video capture
    VideoCapture cam(0);
    // Variable to store a video frame and its grayscale
    Mat frame, gray;
    // Read a frame
    while (cam.read(frame))
    {
        // Find face
        vector<Rect> faces;
        // Convert frame to grayscale because
        // faceDetector requires grayscale image.
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        // Detect faces
        faceDetector.detectMultiScale(gray, faces);
        // Variable for landmarks.
        // Landmarks for one face is a vector of points
        // There can be more than one face in the image. Hence, we
        // use a vector of vector of points.
        vector<vector<Point2f>> landmarks;
        // Run landmark detector
        bool success = facemark->fit(frame, faces, landmarks);

        if (success)
        {
            // If successful, render the landmarks on the face
            for (size_t i = 0; i < faces.size(); i++)
            {
                cv::rectangle(frame, faces[i], Scalar(0, 255, 0), 3);
            }

            for (int i = 0; i < landmarks.size(); i++)
            {
                // if((i >=30)&&(i <= 35))
                drawLandmarks(frame, landmarks[i]);

                for (size_t j = 0; j < landmarks[i].size(); j++)
                {
                    circle(frame, Point(landmarks[i][j].x, landmarks[i][j].y), 1, Scalar(255, 0, 0), 2);
                }
                line(frame, Point(landmarks[i][27].x, landmarks[i][27].y), Point(landmarks[i][8].x, landmarks[i][8].y), Scalar(0, 0, 255), 2);
                float XL = (landmarks[i][45].x + landmarks[i][42].x) / 2;
                float YL = (landmarks[i][45].y + landmarks[i][42].y) / 2;
                float XR = (landmarks[i][39].x + landmarks[i][36].x) / 2;
                float YR = (landmarks[i][39].y + landmarks[i][36].y) / 2;
                line(frame, Point(XL, YL), Point(XR, YR), Scalar(0, 0, 255), 2);
                float DX = XR - XL;
                float DY = YR - YL;
                float L = sqrt(DX * DX + DY * DY);
                float X1 = (landmarks[i][27].x);
                float Y1 = (landmarks[i][27].y);
                float X2 = (landmarks[i][8].x);
                float Y2 = (landmarks[i][8].y);
                float DX1 = abs(X1 - X2);
                float DY1 = abs(Y1 - Y2);
                float L1 = sqrt(DX1 * DX1 + DY1 * DY1);
                float X0 = (XL + XR) / 2;
                float Y0 = (YL + YR) / 2;
                float sin_AL = DY / L;
                float cos_AL = DX / L;

                float X_User_0 = (landmarks[i][27].x - X0) / L;
                float Y_User_0 = -(landmarks[i][27].y - Y0) / L;
                float X_User27 = X_User_0 * cos_AL - Y_User_0 * sin_AL;
                float Y_User27 = X_User_0 * sin_AL + Y_User_0 * cos_AL;

                X_User_0 = (landmarks[i][30].x - X0) / L;
                Y_User_0 = -(landmarks[i][30].y - Y0) / L;
                float X_User30 = X_User_0 * cos_AL - Y_User_0 * sin_AL;
                float Y_User30 = X_User_0 * sin_AL + Y_User_0 * cos_AL;

                if (abs(X_User27 - X_User30) <= 0.1)
                {
                    // �������� �������� ������������� ����
                    float mark_27_8 = abs(L1 / L);
                            
                    // ����� ������������� ����
                    string text;

                    // ������ ����
                    FaceData *face = nullptr, member;
                    
                    switch (mode) {
                        case 1:
                            // ��������� ��������� � ��������� ��� ���������� �� ����
                            face = faceIdentified(mark_27_8, precision);
                            if (face)
                            {
                                text = face->name;
                            }
                            else
                            {
                                text = "Unknown";
                            }
                            putText(
                                frame,
                                text,
                                Point(landmarks[i][27].x, landmarks[i][27].y),
                                1, 2, Scalar(0, 0, 255), 2
                            );                           
                            /*
                            if (abs((L1 / L) - 1.6) < 0.1)
                            {
                                putText(frame, "Roman", Point(landmarks[i][27].x, landmarks[i][27].y), 1, 2, Scalar(0, 0, 255), 2);
                            }
                            if (abs((L1 / L) - 1.87) < 0.1)
                            {
                                putText(frame, "Eugeny", Point(landmarks[i][27].x, landmarks[i][27].y), 1, 2, Scalar(0, 0, 255), 2);
                            }
                            if (abs((L1 / L) - 2.1) < 0.1)
                            {
                                putText(frame, "Svirnesvkiy", Point(landmarks[i][27].x, landmarks[i][27].y), 1, 2, Scalar(0, 0, 255), 2);
                            }
                            */ 
                            
                            // ����� ������
                            if (waitKey(1) == TAB)
                            {
                                mode = chMode(2);
                            }
                            break;

                        case 2:
                            // ���������� �������� ��������, ���� ������ ��� � ���� 
                            // �������� ���� �� ������� ����
                            putText(
                                frame, 
                                to_string(mark_27_8),
                                Point(landmarks[i][27].x, landmarks[i][27].y), 
                                1, 2, Scalar(0, 0, 255), 2
                            );
               
                            // ������������� ������ ���� � ����                   
                            cout << "\n" << "������� ��� ��� ������ ���� (�� ���������): ";
                            cin >> member.name;

                            face = getFace(0, member.name);
                            if (face)
                            {
                                cout << "\n���� ��� ���� � ����! �������: " << face->mark_27_8 << "\n\n";
                                mode = chMode(1);
                            }
                            else
                            {
                                member.mark_27_8 = mark_27_8;
                                facesdata.push_back(member);

                                cout << "���� ���������.. �������: " << member.mark_27_8 << "\n\n";                               
                                mode = chMode(1);
                            }

                            break;
                    }
                    // ����: ��������� ��� 
                }
                // ����: ���� �� ������� 
            }
        }

        // Display results
        imshow("Facial Landmark Detection", frame);

        // Exit loop if ESC is pressed
        if (waitKey(1) == ESC)
            break;
    }
    return 0;
}