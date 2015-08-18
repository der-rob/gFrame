//
//  LEDFrame.cpp
//  gFrame
//
//  Created by Robert Albert on 17.08.15.
//
//

#include "LEDFrame.h"

//--------------------------------------------------------------
void LEDFrame::setup() {
    
    enabled = true;
    //setup parameters
    parameters.setName("dmx settings");
    parameters.add(enabled.set("DMX on", false));
    parameters.add(upper_pulsing_limit.set("upper PL",0,0,1));
    parameters.add(lower_pulsing_limit.set("lower PL", 0,0,1));
    parameters.add(brightness.set("LED brightness",1.0,0.0,1));
    parameters.add(pulsing_time.set("LED pulsing time",2000,500,5000));
    
    dmx.connect(0);
    
    setColor(ofColor::fromHsb(0,255,10));
    update();
    
    pulsing = true;
    pulsing_time = 2000; //in milliseconds
    level = 0.0;
    upper_pulsing_limit = 0.6;
    lower_pulsing_limit = 0.05;
}

//--------------------------------------------------------------
void LEDFrame::disconnect() {
    setColor(ofColor::black);
    update();
    dmx.disconnect();
}

//--------------------------------------------------------------
void LEDFrame::startPulsing() {
    pulsing = true;
}

//--------------------------------------------------------------
void LEDFrame::stopPulsing() {
    pulsing = false;
    level = 1.0;
}

//--------------------------------------------------------------
void LEDFrame::updateLevel() {
    //create triangle wave for pulsing led lights
    int time = abs(((int)ofGetElapsedTimeMillis() % (pulsing_time*2)) - pulsing_time);
    
    //check how long no point has been added
    if (ofGetElapsedTimeMillis() - last_points_time > 500 && !pulsing)
    {
        level -= 0.01;
        float new_level = ofMap(time, 0, pulsing_time, lower_pulsing_limit, upper_pulsing_limit);
        if (level - new_level < 0)
            startPulsing();
    }
    float ledlevel2;
    if (pulsing) {
        //int time = abs(((int)ofGetElapsedTimeMillis() % (LED_pulsing_time*2)) - LED_pulsing_time);
        level = ofMap(time, 0, pulsing_time, lower_pulsing_limit, upper_pulsing_limit);
    }
    
}

//--------------------------------------------------------------
void LEDFrame::setColor(ofColor _color) {
    
    ired = (int)((float)_color.r*level*brightness);
    igreen = (int)((float)_color.g*level*brightness);
    iblue = (int)((float)_color.b*level*brightness);
}

//--------------------------------------------------------------
void LEDFrame::update() {
    dmx.setLevel(2, ired);
    dmx.setLevel(3, igreen);
    dmx.setLevel(4, iblue);
    dmx.setLevel(5, 0);
    dmx.update();
}
