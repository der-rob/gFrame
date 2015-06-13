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

#ifndef Included_BaseStyle_H
#define Included_BaseStyle_H
#include "BaseStyle.h"
#endif

class CaligraphyStyle : public BaseStyle {
    
public:
    CaligraphyStyle();
    void render(vector<GPoint> &points, int width, int height);
    void setFill(bool fill);
    void toggleFill();
    void setWidth(int min, int max){min_width = min; max_width = max;}
    void setFadeOutTime(float _fadeouttime, float _fadeduration) { endFadeAge = _fadeouttime; startFadeAge = _fadeouttime - _fadeduration; }
    
private:
    ofPath path;
    int stroke_width = 20;
    bool fill;
    int min_width = 1;
    int max_width = 20;
    int startFadeAge = 5000;
    int endFadeAge = 10000;
    msa::InterpolatorT<ofPoint> interpolator1, interpolator2;
};

#endif