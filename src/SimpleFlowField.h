#ifndef __flowtest__SimpleFlowField__
#define __flowtest__SimpleFlowField__

#include <stdio.h>
#include "ofxFlowTools.h"

using namespace flowTools;

class SimpleFlowField {
public:
    void setup(int _width, int _height, int flow_scale = 4);
    void update();
    void draw();
    void inputUpdate(int x, int y);
    void inputUpdate(float x, float y, int ID);
    void addObstacle(ofTexture &obstacle);
    void resetObstacle();
    
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
    ftPressureField     pressureField;
    
    int					numDrawForces;
    ftDrawForce*		flexDrawForces;
    
    //color helper
    ofColor color;
    
    //the actual fluid color
    ofParameter<ofColor> fluid_color;
    ofParameter<bool>    use_seperate_fluid_color;
    ofParameter<bool>   draw_pressure;
    ofParameter<float>	pressureFieldScale;
    void				setPressureFieldScale(float& _value) { pressureField.setPressureScale(_value); }
    
    ofParameterGroup options;

};

#endif /* defined(__flowtest__SimpleFlowField__) */