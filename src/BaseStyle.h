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
    virtual void render(vector<GPoint> &points);
};