//
//  FlowField.cpp
//  gFrame
//
//  Created by Robert Albert on 27.05.15.
//
//

#include "FlowField.h"

void FlowField::setup(int _width, int _height) {
    //flowtools
    drawWidth = _width;
    drawHeight = _height;
    
    // process all but the density on 16th resolution
    flowWidth = drawWidth/4;
    flowHeight = drawHeight/4;
    
    // Flow & Mask
    opticalFlow.setup(flowWidth, flowHeight);
    velocityMask.setup(drawWidth, drawHeight);
    
    //fluid
    fluid.setup(flowWidth, flowHeight, drawWidth, drawHeight, false);
    
    // Visualisation
    displayScalar.setup(flowWidth, flowHeight);
    velocityField.setup(flowWidth / 4, flowHeight / 4);
    temperatureField.setup(flowWidth / 4, flowHeight / 4);
    
    //Draw Forces
    numDrawForces = 6;
    flexDrawForces = new ftDrawForce[numDrawForces];
    flexDrawForces[0].setup(drawWidth, drawHeight, FT_DENSITY, true);
    flexDrawForces[0].setName("draw full res");
    flexDrawForces[1].setup(flowWidth, flowHeight, FT_VELOCITY, true);
    flexDrawForces[1].setName("draw flow res 1");
    flexDrawForces[2].setup(flowWidth, flowHeight, FT_TEMPERATURE, true);
    flexDrawForces[2].setName("draw flow res 2");
    flexDrawForces[3].setup(drawWidth, drawHeight, FT_DENSITY, false);
    flexDrawForces[3].setName("draw full res");
    flexDrawForces[4].setup(flowWidth, flowHeight, FT_VELOCITY, false);
    flexDrawForces[4].setName("draw flow res 1");
    flexDrawForces[5].setup(flowWidth, flowHeight, FT_TEMPERATURE, false);
    flexDrawForces[5].setName("draw flow res 2");
    
    lastTime = ofGetElapsedTimef();
    for (int i = 0; i < 12; i++)
        last_touch_points[i].set(0,0);
    
    color = ofColor(ofColor::orange);
    color.a = 128;
    
    brightness.set("fluid brightness", 255, 0, 255);
    alpha.set("fluid alpha", 255, 0, 255);
}

void FlowField::update(ofTexture &tex) {
    deltaTime = ofGetElapsedTimef() - lastTime;
    lastTime = ofGetElapsedTimef();
    
    opticalFlow.setSource(tex);
    opticalFlow.update(deltaTime);
    
    velocityMask.setDensity(tex);
    velocityMask.setVelocity(opticalFlow.getOpticalFlow());
    velocityMask.update();
    
    fluid.addVelocity(opticalFlow.getOpticalFlowDecay());
    fluid.addDensity(velocityMask.getColorMask());
    fluid.addTemperature(velocityMask.getLuminanceMask());
    
    for (int i=0; i<numDrawForces; i++) {
        flexDrawForces[i].update();
        if (flexDrawForces[i].didChange()) {
            // if a force is constant multiply by deltaTime
            float strength = flexDrawForces[i].getStrength();
            if (!flexDrawForces[i].getIsTemporary())
                strength *=deltaTime;
            switch (flexDrawForces[i].getType()) {
                case FT_DENSITY:
                    fluid.addDensity(flexDrawForces[i].getTexture(), strength);
                    break;
                case FT_VELOCITY:
                    fluid.addVelocity(flexDrawForces[i].getTexture(), strength);
                    break;
                case FT_TEMPERATURE:
                    fluid.addTemperature(flexDrawForces[i].getTexture(), strength);
                    break;
                case FT_PRESSURE:
                    fluid.addPressure(flexDrawForces[i].getTexture(), strength);
                    break;
                case FT_OBSTACLE:
                    fluid.addTempObstacle(flexDrawForces[i].getTexture());
                default:
                    break;
            }
        }
    }
    
    fluid.update();
}

void FlowField::inputUpdate(float x, float y, int id) {
    ofVec2f this_point = ofVec2f(x,y);
    ofVec2f velocity = this_point - last_touch_points[id];
    for (int i=0; i<3; i++) {
        if (flexDrawForces[i].getType() == FT_VELOCITY)
            flexDrawForces[i].setForce(velocity);
        flexDrawForces[i].applyForce(this_point);
    }
    
    
    last_touch_points[id%20].set(x,y);
}

void FlowField::inputUpdate(float x, float y) {
    
    ofVec2f this_point = ofVec2f(x,y);
    ofVec2f velocity = this_point - last_mouse;
    for (int i=0; i<3; i++)
    {
        if (flexDrawForces[i].getType() == FT_VELOCITY)
            flexDrawForces[i].setForce(velocity);
        flexDrawForces[i].applyForce(this_point);
    }
    /*
    for (int i=3; i<numDrawForces; i++) {
        if (flexDrawForces[i].getType() == FT_VELOCITY)
            flexDrawForces[i].setForce(velocity);
        flexDrawForces[i].applyForce(this_point);
    }*/
    last_mouse.set(x,y);
}

void FlowField::render() {
    int windowWidth = ofGetWindowWidth();
    int windowHeight = ofGetWindowHeight();
    
    ofSetColor(color.r, color.g, color.b, alpha);
    fluid.draw(0, 0, windowWidth, windowHeight);
}

void FlowField::updateObstacle(ofTexture &obstacle) {
    fluid.reset_obstacle();
    fluid.addObstacle(obstacle);
    fluid.addTempObstacle(obstacle);
}




