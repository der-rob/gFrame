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


class CaligraphyStyle {
    
public:
    CaligraphyStyle();
    void render(vector<GPoint>& points);
    void setFill(bool fill);
    void toggleFill();
    
private:
    ofPath path;
    bool fill;
};

#endif