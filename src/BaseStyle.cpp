//
//  BaseStyle.cpp
//  brushtest_02
//
//  Created by Robert Albert on 24.10.14.
//
//

#include "BaseStyle.h"


void BaseStyle::render(vector<GPoint> &points) {
    ofSetColor(255);
    for (int i = 1; i< points.size(); i++)
    {
        ofCircle(points[i].getLocation(), 5);
    }
}