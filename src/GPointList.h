//
//  PointList.h
//  gFrame
//
//  Created by Julian Adenauer on 24.10.14.
//
/*
 
    This class is memory for all the GPoints
    Because of the multitouch thing, this class has to keep track of the current stroke for each id
 
 */

#ifndef __gFrame__PointList__
#define __gFrame__PointList__

#include <iostream>
#include "ofMain.h"
#include "GPoint.h"

class GPointList{

public:
    void update();
    
    void addToNewStroke(GPoint point);
    void add(GPoint point);
    vector<vector<GPoint> >* getAllStrokes(){return &strokes; };
    void setLifetime(float lifetime){ this->lifetime = lifetime; }
    
private:
    vector<vector<GPoint> > strokes;
    map<int, int> currentStrokeForId;
    float lifetime = 500;

};

#endif /* defined(__gFrame__PointList__) */
