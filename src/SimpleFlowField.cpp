//
//  SimpleFlowField.cpp
//  flowtest
//
//  Created by Robert Albert on 13.06.15.
//
//

#include "SimpleFlowField.h"

void SimpleFlowField::setup(int _width, int _height, int flow_sclale)
{
    drawWidth = _width;
    drawHeight = _height;
    // process all but the density on 16th resolution
    flowWidth = drawWidth/flow_sclale;
    flowHeight = drawHeight/flow_sclale;
    pressureFieldScale.addListener(this, &SimpleFlowField::setPressureFieldScale);
    
    // Fluid
    fluid.setup(flowWidth, flowHeight, drawWidth, drawHeight, false);
    pressureField.setup(flowWidth / 4, flowHeight / 4);
    
    // Draw Forces
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
    lastMouse.set(0,0);
    
    for (ofVec2f vec : last_touch_points)
    {
        vec.set(0,0);
    }
    
    options.setName("other options");
    options.add(draw_pressure.set("draw pressure", false));
    options.add(pressureFieldScale.set("Pressure field scale",0.5,0.01,1.0));
    options.add(use_seperate_fluid_color.set("sep. fluid color",false));
    options.add(fluid_color.set("Fluid Color", ofColor(255,255,255,255), ofColor(0,0,0,0), ofColor(255,255,255,255)));
    
    color = ofColor::white;
}

void SimpleFlowField::update()
{
    deltaTime = ofGetElapsedTimef() - lastTime;
    lastTime = ofGetElapsedTimef();
}

void SimpleFlowField::draw() {
    
    for (int i=0; i<numDrawForces; i++) {
        flexDrawForces[i].update();
        if (flexDrawForces[i].didChange()) {
            // if a force is constant multiply by deltaTime
            float strength = flexDrawForces[i].getStrength();
            if (!flexDrawForces[i].getIsTemporary())
                strength *=deltaTime;
            switch (flexDrawForces[i].getType())
            {
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
    
    if (draw_pressure) {
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        pressureField.setPressure(fluid.getPressure());
        pressureField.draw(0, 0, ofGetWidth(), ofGetHeight());
    }
    fluid.update();
    
    int windowWidth = ofGetWindowWidth();
    int windowHeight = ofGetWindowHeight();
    
    if(use_seperate_fluid_color) {
        color = fluid_color;
    }
//    else {
//        ofColor temp_color = fluid_color;
//        color.a = temp_color.a;
//    }
    
    ofSetColor(color);
    fluid.draw(0, 0, windowWidth, windowHeight);
}

void SimpleFlowField::inputUpdate(int x, int y)
{
    ofVec2f mouse;
    
    mouse.set(x / (float)ofGetWindowWidth(), y / (float)ofGetWindowHeight());
    ofVec2f velocity = mouse - lastMouse;
    
    for (int i=0; i<3; i++) {
        if (flexDrawForces[i].getType() == FT_VELOCITY)
            flexDrawForces[i].setForce(velocity);
        flexDrawForces[i].applyForce(mouse);
    }
    
    /*
     for (int i=3; i<numDrawForces; i++) {
     if (flexDrawForces[i].getType() == FT_VELOCITY)
     flexDrawForces[i].setForce(velocity);
     flexDrawForces[i].applyForce(mouse);
     }
     */
    lastMouse.set(mouse.x, mouse.y);
}

void SimpleFlowField::inputUpdate(float x, float y, int ID)
{
    ofVec2f this_point;
    
    this_point.set(x, y);
    ofVec2f velocity = this_point - last_touch_points[ID];
    
    for (int i=0; i<3; i++) {
        if (flexDrawForces[i].getType() == FT_VELOCITY)
            flexDrawForces[i].setForce(velocity);
        flexDrawForces[i].applyForce(this_point);
    }
    /*
     for (int i=3; i<numDrawForces; i++) {
     if (flexDrawForces[i].getType() == FT_VELOCITY)
     flexDrawForces[i].setForce(velocity);
     flexDrawForces[i].applyForce(mouse);
     }
     */
    last_touch_points[ID].set(this_point.x, this_point.y);
}

void SimpleFlowField::addObstacle(ofTexture &obstacle) {
    fluid.reset_obstacle();
    fluid.addObstacle(obstacle);
}

void SimpleFlowField::resetObstacle() {
    fluid.reset_obstacle();
}