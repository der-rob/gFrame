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
    
private:
    bool fill = true;
    ofPoint lastPoint;
};


#endif /* defined(__StyleTest__ProfileStyle__) */
