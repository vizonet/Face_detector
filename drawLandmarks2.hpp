#define _USE_MATH_DEFINES

#include <iostream>
#include "curlSend.hpp"
#include <cmath>

#ifndef _renderFace_H_
#define _renderFace_H_

using namespace cv; 
using namespace std; 

#define COLOR Scalar(255, 200,0)
#define RED Scalar(0, 200, 255)

#define LOWER_LIMIT 135
#define UPPER_LIMIT 205
#define NOSE_LIMIT 200

//#define UPPER_LIMIT5 215 

const double M_PI = acos(-1); // 3.14

// drawPolyLine draws a poly line by joining 
// successive points between the start and end indices. 
void drawPolyline
(
  Mat &im,
  const vector<Point2f> &landmarks,
  const int start,
  const int end,
  bool isClosed = false
)
{
    // Gather all points between the start and end indices
    vector <Point> points;
    for (int i = start; i <= end; i++)
    {
        points.push_back(cv::Point(landmarks[i].x, landmarks[i].y));
    }
    // Draw polylines. 
    polylines(im, points, isClosed, COLOR, 2, 16);
    
}

int segmentDetect(float angle)
{
  float normal_angle = angle - LOWER_LIMIT;
  float range = UPPER_LIMIT - LOWER_LIMIT;

   if(normal_angle < range/3) {
     return 0;
   }  else if(normal_angle < range*2/3) {
     return 1;
   }  else  {
     return 2;
   }
}


//detect headtilt
int tiltRatio
(
  const vector<Point2f> &landmarks,
  const int jawStart,
  const int jawEnd,
  const int lipStart,
  const int lipEnd
)
{

  float minJ, maxJ, minL, maxL;
  float midJ, midL;
  float angle;

  minJ = landmarks[jawStart].x;
  maxJ = landmarks[jawStart].x;

  for(int i = jawStart; i <= jawEnd; i++) {
    if(landmarks[i].x < minJ) {
      minJ = landmarks[i].x;
    }

    if(landmarks[i].x > maxJ) {
      maxJ = landmarks[i].x;
    }
  }

  midJ = (minJ + maxJ)/2.0;

  minL = landmarks[lipStart].x;
  maxL = landmarks[lipStart].x;

  for(int i = lipStart; i <= lipEnd; i++) {
    if(landmarks[i].x < minL) {
      minL = landmarks[i].x;
    }

    if(landmarks[i].x > maxL) {
      maxL = landmarks[i].x;
    }
  }

  midL = (minL + maxL)/2.0;



  angle = (midJ - midL) / ((minJ - maxJ)/2);
  angle = acos(angle);
  angle = angle * (360/M_PI);

  //std::cout << "angle: " + to_string(angle) << endl;
  //std::cout << "segment: " + to_string(segmentDetect(angle)) << endl;

  return segmentDetect(angle);

}

int pitchRatio(
  const vector<Point2f> &landmarks,
  const int leftEdge,
  const int tip,
  const int rightEdge
)
{
  //using law of cosines
  //c2 = a2 + b2 - 2ab(cosC)
  float leftEdge_square = pow(landmarks[leftEdge].x - landmarks[tip].x, 2) + pow(landmarks[leftEdge].y - landmarks[tip].y, 2);
  float rightEdge_square = pow(landmarks[rightEdge].x - landmarks[tip].x, 2) + pow(landmarks[rightEdge].y - landmarks[tip].y, 2);
  float base_square = pow(landmarks[rightEdge].x - landmarks[leftEdge].x, 2) + pow(landmarks[rightEdge].y - landmarks[leftEdge].y, 2);

  float angle = (leftEdge_square + rightEdge_square - base_square) / (2*sqrt(leftEdge_square)*sqrt(rightEdge_square));
  angle = acos(angle) * (360/M_PI);

  if(angle > 200) {
    return 1;
  } else {
    return 0;
  }
}

float oldEar = 0.0;
float earHistory[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
bool blinkHistory[] = {false, false, false, false, false, false};

float eyeEAR (int eyeStart, const vector<Point2f> &landmarks) {
    float p2p6 = sqrt(pow(landmarks[eyeStart + 2].x - landmarks[eyeStart + 6].x, 2) + pow(landmarks[eyeStart + 2].y - landmarks[eyeStart + 6].y, 2));
    float p3p5 = sqrt(pow(landmarks[eyeStart + 3].x - landmarks[eyeStart + 5].x, 2) + pow(landmarks[eyeStart + 3].y - landmarks[eyeStart + 5].y, 2));
    float p1p4 = sqrt(pow(landmarks[eyeStart + 1].x - landmarks[eyeStart + 4].x, 2) + pow(landmarks[eyeStart + 1].y - landmarks[eyeStart + 4].y, 2));

    return (p2p6 + p3p5)/(2*p1p4);
}

//https://www.pyimagesearch.com/2017/04/24/eye-blink-detection-opencv-python-dlib/
float detectBlink(
  const vector<Point2f> &landmarks
) {
  /*
    Left eye
    p1-6: 37 - 42
    Right eye
    p1-6: 43 - 48
    ear = (||p2-p6|| + ||p3-p5||)/(2*(||p1-p4||))
  */

  int leftEye = 36;
  int rightEye = 42;
  
  float newEar = (eyeEAR(leftEye, landmarks) + eyeEAR(rightEye, landmarks))/2;

  // float perChange = ((oldEar - newEar)*100)/oldEar;
  // oldEar = newEar;

  for(int i = 0; i < 5; i++) {
    earHistory[i] = earHistory[i + 1];
    blinkHistory[i] = blinkHistory[i+1];
  }
  earHistory[5] = newEar;

  float lowestEar = 10.0;

  for(int i = 0; i < 5; i++) {
    if(earHistory[i] < lowestEar) {
      lowestEar = earHistory[i];
    }
  }

  float perChange = ((newEar - lowestEar)/lowestEar)*100;
  bool recentBlink = blinkHistory[0] || blinkHistory[1] || blinkHistory[2] || blinkHistory[3] || blinkHistory[4] || blinkHistory[5];

  if(perChange > 15 && !recentBlink) {
    blinkHistory[5] = true;
    return true;
  } else {
    blinkHistory[5] = false;
    return false;
  }
  //std::cout << "EAR %change: " + to_string(perChange) << endl;

}

int choiceHistory[] = {0,0,1,0,0,0,0};
int choiceHistoryLength = (sizeof(choiceHistory)/sizeof(*choiceHistory));
int lastStare = 99;
void drawLandmarks(Mat &im, vector<Point2f> &landmarks)
{
    // Draw face for the 68-point model.
    if (landmarks.size() == 68)
    {
      drawPolyline(im, landmarks, 0, 16);           // Jaw line
      drawPolyline(im, landmarks, 17, 21);          // Left eyebrow
      drawPolyline(im, landmarks, 22, 26);          // Right eyebrow
      drawPolyline(im, landmarks, 27, 30);          // Nose bridge
      drawPolyline(im, landmarks, 30, 35, true);    // Lower nose
      drawPolyline(im, landmarks, 36, 41, true);    // Left eye
      drawPolyline(im, landmarks, 42, 47, true);    // Right Eye
      drawPolyline(im, landmarks, 48, 59, true);    // Outer lip
      drawPolyline(im, landmarks, 60, 67, true);    // Inner lip

      int tilt = tiltRatio(landmarks, 0, 16, 48, 59);
      int pitch = pitchRatio(landmarks, 31, 30, 35);
      //bool blink = detectBlink(landmarks);

      int choice = 3*pitch + tilt;

      std::cout << "Choice " + to_string(choice) << endl;
      //std::cout << "Blink " + to_string(blink) << endl;

      std::string action = "lookAt" + to_string(choice);

      bool stare = true;

      for(int i=0; i<(choiceHistoryLength - 1); i++) {
        stare = stare && (choiceHistory[i] == choiceHistory[i+1]);
        choiceHistory[i] = choiceHistory[i+1];
      }
      choiceHistory[choiceHistoryLength - 1] = choice;
      stare = stare && (choiceHistory[choiceHistoryLength - 2] == choiceHistory[choiceHistoryLength - 1]);
      if(stare) {
        stare = (choice != lastStare);
      }

      if(stare) {
        lastStare = choice;
      }

      sendHTTP(choice, false, stare);
    }
    else 
    { // If the number of points is not 68, we do not know which 
      // points correspond to which facial features. So, we draw 
      // one dot per landamrk. 
      for(int i = 0; i < landmarks.size(); i++)
      {
        circle(im,landmarks[i],3, COLOR, FILLED);
      }
    }
    
}

float getFaceArea(vector<Point2f> &landmarks) {
  float lowX = 0.0;
  float highX = 0.0;
  float lowY = 0.0;
  float highY = 0.0;

  if(landmarks.size() == 68) {
    lowX = landmarks[0].x;
    highX = landmarks[0].x;
    lowY = landmarks[0].y;
    highY = landmarks[0].y;

    for (int i = 0; i < landmarks.size(); i++) {
      if(landmarks[i].x < lowX) {
        lowX = landmarks[i].x;
      }

      if(landmarks[i].y < lowY) {
        lowY = landmarks[i].y;
      }

      if(landmarks[i].x > highX) {
        highX = landmarks[i].x;
      }

      if(landmarks[i].y > highY) {
        highY = landmarks[i].y;
      }
    }

    float area = pow(highX - lowX, 2) + pow(highY - lowY, 2);
    return area;
  } else {
    return 0.0;
  }
}

#endif // _renderFace_H_