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
    
    int new_point_distance = ofGetWidth()/50.0;


public:
    virtual void render(vector<GPoint> &points);
    
    void setNewPointDistance(int _distance) {new_point_distance = _distance;}
    int getNewPointDistance() { return new_point_distance;}

};