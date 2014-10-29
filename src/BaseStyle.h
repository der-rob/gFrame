//
//  BaseStyle.h
//  brushtest_02
//
//  Created by Robert Albert on 24.10.14.
//
//
#include <iostream.h>
#include "ofMain.h"
#include "GPoint.h"

class BaseStyle {

private:
    int stroke_id;
    ofPoint lastPoint;

public:
    BaseStyle();
    
    virtual void render(vector<GPoint> &points);
    
    //int getStrokeID() {return stroke_id;}
    //void setStrokeID(int _id) {stroke_id = _id;}
    //ofPoint getLastPoint() {return lastPoint;}
    //void setLastPoint(ofPoint new_lastPoint) {lastPoint = new_lastPoint;}
};