//
//  FlowStroke.h
//  gFrame
//
//  Created by Robert Albert on 27.05.15.
//
//

#ifndef __gFrame__FlowStroke__
#define __gFrame__FlowStroke__

#include <stdio.h>
#include "ofxFlowTools.h"

//#define USE_FASTER_INTERNAL_FORMATS			// about 15% faster but gives errors from ofGLUtils

using namespace flowTools;

class FlowField {
public:
    void setup(int _width, int _height);
    void inputUpdate(float x, float y);
    void inputUpdate(float x, float y, int id);
    void render();
    void update(ofTexture &tex);
    void updateObstacle(ofTexture &obstacle);
    ofParameterGroup *getFluidParameters() { return &fluid.parameters;}
    ofParameterGroup *getVelocityParameters() { return &velocityMask.parameters;}
    ofParameter<int> *getAlpha() { return &alpha; }
    ofParameter<int> *getBrightness() { return &brightness;}
    void setColor(ofColor _color) { this->color = _color;}
    
private:
    float				lastTime;
    float				deltaTime;
    int					flowWidth;
    int					flowHeight;
    int					drawWidth;
    int					drawHeight;
    ofColor             color;
    ofParameter<int>    alpha;
    ofParameter<int>    brightness;

    ftOpticalFlow		opticalFlow;
    ftVelocityMask		velocityMask;
    ftFluidSimulation	fluid;
    
    ftDisplayScalar		displayScalar;
    ftVelocityField		velocityField;
    ftTemperatureField	temperatureField;
    
    int					numDrawForces;
    ftDrawForce*		flexDrawForces;
    
    ofVec2f             last_touch_points[12];
    ofVec2f				last_mouse;
};
#endif /* defined(__gFrame__FlowStroke__) */
