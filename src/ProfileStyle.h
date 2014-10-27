//
//  ProfileStyle.h
//  StyleTest
//
//  Created by Julian Adenauer on 14.10.14.
//
//

#ifndef __StyleTest__ProfileStyle__
#define __StyleTest__ProfileStyle__

#include <iostream>
#include "ofMain.h"
#include "GPoint.h"

class ProfileStyle {
    
public:
    ProfileStyle();
    void update();
    void draw();
    void dragging(int x, int y);
    void released(int x, int y);
    void render(vector<GPoint> &points);
    void setFill(bool fill);
    void toggleFill();
    void clear();
    void addPoint(GPoint p);
    int getStrokeId(){return stroke_id;}
    
    
private:
    bool fill;
    vector<vector<GPoint> > points;
    ofPoint lastPoint;
    int stroke_id;
};


#endif /* defined(__StyleTest__ProfileStyle__) */
