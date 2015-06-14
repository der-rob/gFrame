//
//  SimpleFlowField.h
//  flowtest
//
//  Created by Robert Albert on 13.06.15.
//
//

#ifndef __flowtest__SimpleFlowField__
#define __flowtest__SimpleFlowField__

#include <stdio.h>
#include "ofxFlowTools.h"

using namespace flowTools;

class SimpleFlowField {
public:
    void setup(int _width, int _height);
    void update();
    void draw();
    void inputUpdate(int x, int y);
    void inputUpdate(float x, float y, int ID);
    void addObstacle(ofTexture &obstacle);
    
    // Time
    float				lastTime;
    float               deltaTime;
    
    
    
    //input
    ofVec2f				lastMouse;
    ofVec2f             last_touch_points[12];
    
    // FlowTools
    int					flowWidth;
    int					flowHeight;
    int					drawWidth;
    int					drawHeight;
    
    ftFluidSimulation	fluid;
    
    int					numDrawForces;
    ftDrawForce*		flexDrawForces;
    
    ofColor             color;
    ofParameter<int>    brightness;
    ofParameter<int>    alpha;
};

#endif /* defined(__flowtest__SimpleFlowField__) */
