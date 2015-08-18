//
//  LEDFrame.h
//  gFrame
//
//  Created by Robert Albert on 17.08.15.
//
// This class encapsulates all the functionality to drive the GFrame'S LEDS in the aluminium frame
//

#ifndef __gFrame__LEDFrame__
#define __gFrame__LEDFrame__

#include <stdio.h>
#include "ofMain.h"
#include "ofxDmx.h"

class LEDFrame {
public:
    ofParameterGroup parameters;

private:
    ofParameter<bool> enabled;
    ofxDmx dmx;
    ofColor LEDstripColor;
    ofParameter<float> upper_pulsing_limit, lower_pulsing_limit;
    ofParameter<int> pulsing_time;
    ofParameter<float> level;
    ofParameter<float> brightness;
    
    float last_points_time;

    //color as int values
    unsigned char ired, igreen, iblue;
    
    bool pulsing;
    
public:
    void setup();
    void disconnect();
    void updateLevel();
    void startPulsing();
    void stopPulsing();
    void setColor(ofColor _color);
    void update();
    
    bool getEnabled() { return enabled;}
    void setEnabled(bool _enabled) { enabled = _enabled;}
    void toggleEnabled() {enabled = !enabled;}
    
    void updateLastPointsTime() {last_points_time = ofGetElapsedTimef();}
    float getLastPointsTime() {return last_points_time;}
};

#endif /* defined(__gFrame__LEDFrame__) */
