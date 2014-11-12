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
    void render(vector<GPoint> &points);
    
    void setFill(bool fill);
    void toggleFill();
    
    void setLineWidth(int width){ this->width = width; }
    void setLineDepth(int depth){ this->depth = depth; }
    void setZSpeed(int speed){this->z_speed = speed; }
    void setTwist(int twist){this->twist = twist;}
    
private:
    bool fill = true;
    ofPoint lastPoint;
    int depth = 10;
    int width = 10;
    int z_speed = 1;
    int twist = 5;
};


#endif /* defined(__StyleTest__ProfileStyle__) */
