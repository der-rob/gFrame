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

using namespace flowTools;

class FlowField {
public:
    FlowField();
    ~FlowField();
    void setup(int _width, int _height);
    void inputUpdate(float x, float y);
    void inputUpdate(float x, float y, int id);
    void render();
    void update(ofTexture &tex);
    void updateObstacle(ofTexture &obstacle);
    ofParameterGroup *getFluidParameters() { return &fluid.parameters;}
    ofParameterGroup *getFluidParameters_2() { return &fluid2.parameters;}
    void setColor(ofColor _color) { this->color = _color;}
    
private:
    float				lastTime;
    float				deltaTime;
    int					flowWidth;
    int					flowHeight;
    int					drawWidth;
    int					drawHeight;
    ofColor             color;
    ofColor             color2;

    ftOpticalFlow		opticalFlow;
    ftVelocityMask		velocityMask;
    ftFluidSimulation	fluid;
    ftFluidSimulation   fluid2;
    ftParticleFlow		particleFlow;
    
    ftDisplayScalar		displayScalar;
    ftVelocityField		velocityField;
    ftTemperatureField	temperatureField;
    
    int					numDrawForces;
    ftDrawForce*		flexDrawForces;
    
    ofVec2f             last_touch_points[12];
    ofVec2f				last_mouse;

    

};
#endif /* defined(__gFrame__FlowStroke__) */
