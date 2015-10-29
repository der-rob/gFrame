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

#ifndef __gFrame__StrokeList__
#define __gFrame__StrokeList__

#include <iostream>
#include "ofMain.h"
#include "GPoint.h"
#include "Network.h"

#define TYPE_LOCAL 0
#define TYPE_REMOTE 1

class StrokeList{

public:
    void update();
    
    void setupSync(Network* network);
    
    void addToNewStroke(GPoint point);
    void add(GPoint point);
    void clear();
    
    // SETTERS
    void setLifetime(float lifetime){ this->lifetime = lifetime; }
    
    // GETTERS
    float getLifetime(){return lifetime;}
    vector<vector<GPoint> >* getAllStrokes(){return &strokes; };

    
private:
    vector<vector<GPoint> > strokes;
    float lifetime = 60000;
    int stroke_count = 0;
    
    Network *network;
    bool enableSync = false;
};

#endif /* defined(__gFrame__PointList__) */
