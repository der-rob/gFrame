//
//  CaligraphyStyle.h
//  StyleTest
//
//  Created by Julian Adenauer on 14.10.14.
//
//

#ifndef __StyleTest__CaligraphyStyle__
#define __StyleTest__CaligraphyStyle__

#include <iostream>

#include "ofMain.h"
#include "GPoint.h"

#include "MSACore.h"
#include "MSAInterpolator.h"


class CaligraphyStyle {
    
public:
    CaligraphyStyle();
    void render(vector<GPoint> &points, int width, int height);
    void setFill(bool fill);
    void toggleFill();
    void setStrokeWidth(int _width) { stroke_width = _width; }
    int getStrokeWidth() { return stroke_width; }
    
private:
    ofPath path;
    int stroke_width = 20;
    bool fill;
    msa::InterpolatorT<ofPoint> interpolator1, interpolator2;
};

#endif