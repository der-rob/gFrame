#pragma once
#include "gFrameApp.h"

#define USE_PROGRAMMABLE_GL     // Maybe there is a reason you would want to


//--------------------------------------------------------------
void gFrameApp::setup(){
    
    //just set up the openFrameworks stuff
    ofSetFrameRate(30);
    ofSetVerticalSync(true);
    ofBackground(ofColor::black);
    ofSetWindowShape(1280, 720);
    
    //dimensions for final output
    outputRect = ofRectangle(0,0,1280, 720);

    //GUI setup
    guiSetup();
    styleGuiSetup();
    flowGuiSetup();
    ofAddListener(gui.loadPressedE, this, &gFrameApp::onSettingsReload);
    ofAddListener(gui.savePressedE, this, &gFrameApp::onSettingsSave);
    ofAddListener(style_gui.loadPressedE, this, &gFrameApp::onStyleSettingsreload);
    ofAddListener(style_gui.savePressedE, this, &gFrameApp::onStyleSettingsSave);
    ofAddListener(flow_gui.loadPressedE, this, &gFrameApp::onFlowSettingsReload);
    ofAddListener(flow_gui.savePressedE, this, &gFrameApp::onFlowSettingsSave);
    
    gui.loadFromFile("settings.xml");
    style_gui.loadFromFile("stylesettings.xml");
    flow_gui.loadFromFile("flow_settings.xml");
    
    //need to call this here, otherwise would get a BAD ACCESS FAULT
    gui.draw();
    style_gui.draw();
    
    //Syphon stuff
    syphonMainOut.setName("gFrame Main Out");
    
    //TUIO setup
    tuioClient.start(tuioPort);
    ofAddListener(tuioClient.cursorAdded,this,&gFrameApp::tuioAdded);
	ofAddListener(tuioClient.cursorUpdated,this,&gFrameApp::tuioUpdated);
    ofAddListener(tuioClient.cursorRemoved,this,&gFrameApp::tuioRemoved);
    
    //DMX for controlling RGB LED Strips
    dmx.connect(0);
    setLEDColor(ofColor::fromHsb(0,255,10));
    LED_pulsing = true;
    LED_pulsing_time = 2000; //in milliseconds
    LED_level = 0.0;
    upper_pulsing_limit = 0.6;
    lower_pulsing_limit = 0.05;
    
    //OSC
    receiver.setup(local_osc_port);
    try {
    sender.setup(ipad_ip,ipad_port);
    }
    catch (exception e) {
        ofLogError() << "unable to connect to ipad";
        use_ipad = false;
    }
    
    // SETUP LIGHT
    light.enable();
    light.setPointLight();
    light.setPosition(0,0,0);

#ifdef USE_NETWORK
    // NETWORK
    network.setup(host_port, remote_ip, remote_port);
    stroke_list.setupSync(&network);
#endif

    //brazil support
//  syphonFBO.allocate(1024, 768, GL_RGBA, 2);
    syphonFBO.allocate(outputRect.width, outputRect.height, GL_RGBA, 2);
    canvasFBO.allocate(outputRect.width, outputRect.height, GL_RGBA, 2);
    mCanvas.allocate(outputRect.width, outputRect.height, OF_IMAGE_COLOR);
    syphonFBO.begin(); ofClear(0); syphonFBO.end();
    canvasFBO.begin(); ofClear(0); canvasFBO.end();

    // initialize finger positions
    for(ofVec2f finger : finger_positions){
        finger = ofVec2f(0,0);
    }
    
    
    //flowtools
    drawWidth = outputRect.width;
    drawHeight = outputRect.height;
    // process all but the density on 16th resolution
    flowWidth = drawWidth/4;
    flowHeight = drawHeight/4;
    
    // Flow & Mask
    opticalFlow.setup(flowWidth, flowHeight);
    velocityMask.setup(drawWidth, drawHeight);
    //fluid
    fluid.setup(flowWidth, flowHeight, drawWidth, drawHeight, false);
    //particles
    particleFlow.setup(flowWidth, flowHeight, drawWidth, drawHeight);
    
    // Visualisation
    displayScalar.allocate(flowWidth, flowHeight);
    velocityField.allocate(flowWidth / 4, flowHeight / 4);
    temperatureField.allocate(flowWidth / 4, flowHeight / 4);
    
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
    lastMouse.set(0,0);
    for (int i = 0; i < 12; i++)
        last_touch_points[i].set(0,0);
    
    //stencil
    stencilText = "Applied Future!";
    stencilFont.loadFont("AkzidenzGrotesk-Cond.otf", 150);
    stencilFBO.allocate(outputRect.width, outputRect.height, GL_RGBA, 2);
    stencilFBO.begin();
    ofClear(0);
    ofRectangle stringBounds = stencilFont.getStringBoundingBox(stencilText, 0, 0);
    int text_x = (ofGetWidth() - stringBounds.width) / 2;
    int text_y = (ofGetHeight() + stringBounds.height) /2;
    stencilFont.drawString(stencilText, text_x, text_y);
    stencilFBO.end();
    
    fluid.addObstacle(stencilFBO.getTextureReference());
    fluid.addTempObstacle(stencilFBO.getTextureReference());


}
void gFrameApp::exit(){
    setLEDColor(ofColor::black);
    dmx.disconnect();
    network.disconnect();
}

//--------------------------------------------------------------
void gFrameApp::update(){
    
    stroke_list.update();
    if (use_ipad)
        oscUpdate();
    tuioClient.getMessage();
    
    // DMX UPDATE
    if (dmx_on)
        updateLEDpulsing();
    
    canvasFBO.begin();
    ofBackground(0);
    for(vector<GPoint> stroke : *stroke_list.getAllStrokes()){
        switch(stroke[0].getStyle()){
            case STYLE_PROFILE:
                profileStyle.render(stroke, (int)outputRect.width, (int)outputRect.height);
                break;
            case STYLE_SCRIZZLE:
                scrizzleStyle.render(stroke, (int)outputRect.width, (int)outputRect.height);
                break;
            case STYLE_CALIGRAPHY:
                caligraphyStyle.render(stroke, (int)outputRect.width, (int)outputRect.height);
                break;
            default:
                profileStyle.render(stroke, (int)outputRect.width, (int)outputRect.height);
                break;
        }
    }
  
    if(draw_finger_positions){
        drawFingerPositions((int)outputRect.width, (int)outputRect.height);
    }
    
    canvasFBO.end();
    
    canvasFBO.readToPixels(mCanvas.getPixelsRef());
    mCanvas.reloadTexture();
    
//    syphonFBO.begin();
//    
//    ofClear(0);
//
//    mCanvas.draw(outputRect.x, outputRect.y);
//    
//    syphonFBO.end();
//    syphonMainOut.publishTexture(&syphonFBO.getTextureReference());
    
    //update the brush settings
    //scrizzle style
    scrizzleStyle.setMainLineThickness(W_mainLine_thickness, W_byLine_thicknes);
    scrizzleStyle.setAmplitude(W_amplitude);
    scrizzleStyle.setLength(W_wavelength);
    scrizzleStyle.setNervousity(W_nervosity);
    scrizzleStyle.setFadeOutTime(W_fadeout_time*1000.0, W_fadeduration*1000.0);
    scrizzleStyle.setNewPointDistance(newPointDistance);
    
    //profile style
    profileStyle.setLineWidth(style_profile_width);
    profileStyle.setLineDepth(style_profile_depth);
    profileStyle.setZSpeed(style_profile_zspeed);
    profileStyle.setTwist(style_profile_twist);
    profileStyle.setNewPointDistance(newPointDistance);
    
    //caligraphy style
    caligraphyStyle.setWidth(C_width_min, C_width_max);
    caligraphyStyle.setFadeOutTime(C_fadeout_time*1000.0, C_fadeduration*1000.0);

    // lifetime
    stroke_list.setLifetime(point_lifetime * 1000);
    
    //flowtools
    deltaTime = ofGetElapsedTimef() - lastTime;
    lastTime = ofGetElapsedTimef();
}

//--------------------------------------------------------------
void gFrameApp::draw(){
    
    ofBackground(0);
    ofSetColor(255);
    
    
    syphonFBO.begin();
    ofClear(0);
    
    //flowtools
    opticalFlow.setSource(mCanvas.getTextureReference());
    opticalFlow.update(deltaTime);

    velocityMask.setDensity(mCanvas.getTextureReference());
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
                    fluid.addDensity(flexDrawForces[i].getTextureReference(), strength);
                    break;
                case FT_VELOCITY:
                    fluid.addVelocity(flexDrawForces[i].getTextureReference(), strength);
                    particleFlow.addFlowVelocity(flexDrawForces[i].getTextureReference(), strength);
                    break;
                case FT_TEMPERATURE:
                    fluid.addTemperature(flexDrawForces[i].getTextureReference(), strength);
                    break;
                case FT_PRESSURE:
                    fluid.addPressure(flexDrawForces[i].getTextureReference(), strength);
                    break;
                case FT_OBSTACLE:
                    fluid.addTempObstacle(flexDrawForces[i].getTextureReference());
                default:
                    break;
            }
        }
    }
    
    fluid.update();
    
    if (particleFlow.isActive()) {
        particleFlow.setSpeed(fluid.getSpeed());
        particleFlow.setCellSize(fluid.getCellSize());
        particleFlow.addFlowVelocity(opticalFlow.getOpticalFlow());
        particleFlow.addFluidVelocity(fluid.getVelocity());
        particleFlow.setObstacle(fluid.getObstacle());
    }
    particleFlow.update();
    
    
    int windowWidth = ofGetWindowWidth();
    int windowHeight = ofGetWindowHeight();
    ofClear(0,0);
    
    // Fluid Composite
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
    mCanvas.draw(0,0, windowWidth, windowHeight);
    
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofSetColor(localBrushColor);
    fluid.draw(0, 0, windowWidth, windowHeight);
    
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    //if (particleFlow.isActive())
        //particleFlow.draw(0, 0, windowWidth, windowHeight);
    
    ofPopStyle();
    syphonFBO.end();

    syphonMainOut.publishTexture(&syphonFBO.getTextureReference());

    //switching of the main screen might improve the performance
    if (draw_on_main_screen)
    {
        syphonFBO.draw(0,0,outputRect.width, outputRect.height);
    }
    
    //gui output here
    if(draw_gui){
        gui.draw();
        style_gui.draw();
        flow_gui.draw();
        ofSetColor(255);
        ofDrawBitmapString("clients: " + ofToString(network.isConnected()), ofGetWidth()-120, ofGetHeight()-85);
        ofDrawBitmapString("connected: " + ofToString(network.getNumClients()), ofGetWidth()-120, ofGetHeight()-70);
        ofDrawBitmapString("style: " + ofToString(current_style), ofGetWidth()-120, ofGetHeight()-55);
        ofDrawBitmapString("r: " + ofToString(network.getReceiveQueueLength()), ofGetWidth()-120, ofGetHeight()-40);
        ofDrawBitmapString("s: " + ofToString(network.getSendQueueLength()), ofGetWidth()-120, ofGetHeight()-25 );
        ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate(), 2), ofGetWidth()-120, ofGetHeight()-10 );
    }
    
    if(draw_flow_gui)
        draw_flow_gui = !draw_flow_gui;
    
    //stencilFBO.draw(0, 0, outputRect.width, outputRect.height);
}

void gFrameApp::drawFingerPositions(int _width, int _height){
    
    ofPushStyle();
    ofDisableDepthTest(); // disable depth test so the alpha blending works properly
    ofDisableLighting();
    
    ofColor outerColor = localBrushColor;
    outerColor.set(localBrushColor.get().r, localBrushColor.get().g, localBrushColor.get().b, 30);
    
    ofSetColor(localBrushColor);
    int i=0;
    for(ofVec2f finger : finger_positions){
        int px = finger.x * _width;
        int py = finger.y * _height;
        if(!(px == 0 && py == 0)){
            float incr = (float) ((2 * PI) / 32);
            
            glBegin(GL_TRIANGLE_FAN);
            ofSetColor(localBrushColor);
            glVertex2f(px, py);
            
            ofSetColor(outerColor);
            
            for(int i = 0; i < 32; i++){
                float angle = incr * i;
                float x = ((float) cos(angle) * finger_position_size) + px;
                float y = ((float) sin(angle) * finger_position_size) + py;
                glVertex2f(x, y);
            }
            
            glVertex2f(finger_position_size + px, py);
            glEnd();
#if DEBUG
            // draw finger id for debugging
            ofSetColor(255, 255, 255, 255);
            ofDrawBitmapString(ofToString(i), finger.x-5, finger.y+5);
#endif
        }
        i++;
    }
    ofPopStyle();
}

//--------------------------------------------------------------
void gFrameApp::keyPressed(int key){
    if (key == 'r') {
        localBrushColor = ofColor::red;
    }
    else if (key == 'b') {
        localBrushColor = ofColor::blue;
    }
    else if (key == 'g') {
        localBrushColor = ofColor::green;
    }
    else if(key == 'c') {
        stroke_list.clear();
    }
    else if (key == 'p')
        current_style = STYLE_PROFILE;
    else if (key == 's')
        current_style = STYLE_SCRIZZLE;
    else if (key == 'k')
        current_style = STYLE_CALIGRAPHY;
    else if(key == 'h')
        draw_gui = !draw_gui;
    else if (key == 'd')
        dmx_on = !dmx_on;
    else if(key == 'y')
        draw_on_main_screen = !draw_on_main_screen;
    else if(key == 'f')
        draw_flow_gui = !draw_flow_gui;
    else if (key == 'x') {
        fullscreen = !fullscreen;
        ofSetFullscreen(fullscreen);
        style_gui.setPosition(ofGetWidth() - 2*style_gui.getWidth() - 20, 10);
        gui.setPosition(ofGetWidth() - gui.getWidth() - 10, 10);
        outputRect.width = ofGetWidth();
        outputRect.height = ofGetHeight();
        canvasFBO.allocate(outputRect.width, outputRect.height);
        mCanvas.allocate(outputRect.width, outputRect.height, OF_IMAGE_COLOR);
        syphonFBO.allocate(outputRect.width, outputRect.height);

        
    }
    
    else if (key == 'm') {
        input_mouse = !input_mouse;
    }
    
    //switch between different output modes
    else if (key == '1') {
        //ofSetWindowShape(1024, 768);
        outputRect.width = 1024;
        outputRect.height =768;
        outputRect.x = 0;
        outputRect.y = 0;
        ofSetWindowShape(outputRect.width, outputRect.height);
        outputmode = PROJECTOR;
        orientation = LANDSCAPE;
        canvasFBO.allocate(outputRect.width, outputRect.height);
        mCanvas.allocate(outputRect.width, outputRect.height, OF_IMAGE_COLOR);
    }
    else if (key == '2') {
        outputRect.width = 512;
        outputRect.height = 768;
        outputRect.x = 0;
        outputRect.y = 0;
        ofSetWindowShape(outputRect.width, outputRect.height);
        outputmode = PROJECTOR_PORTRAIT;
        orientation = PORTRAIT;
        canvasFBO.allocate(outputRect.width, outputRect.height);
        mCanvas.allocate(outputRect.width, outputRect.height, OF_IMAGE_COLOR);
    }
    else if (key == 't') {
        changeStencilText(ofSystemTextBoxDialog("enter new obstacle text"));
    }
    

}

//--------------------------------------------------------------
void gFrameApp::mouseMoved(int x, int y){
    
    if(input_mouse){
        ofVec2f mouse;
        
        mouse.set(x / (float)ofGetWindowWidth(), y / (float)ofGetWindowHeight());
        
        
        GPoint the_point;
        //rescale mouse position
        //float x_norm = ofMap(x, 0, ofGetWidth(), 0.0, outputRect.width);
        float x_norm = ofMap(x, 0, outputRect.width, 0.0, 1.0);
        //float y_norm = ofMap(y, 0, ofGetHeight(), 0.0, outputRect.height);
        float y_norm = ofMap(y, 0, outputRect.height, 0.0, 1.0);
        
        the_point.setLocation(ofVec2f(mouse.x,mouse.y));
        the_point.setId(0);
        the_point.setStrokeId(0);
        the_point.setColor(localBrushColor);
        the_point.setStyle(current_style);
        //    the_point.lifetime = 0;
        //    all_points.push_back(the_point);
        stroke_list.add(the_point);
        
        stop_pulsing();
        last_points_time = ofGetElapsedTimeMillis();
        
        //flowtools
        ofVec2f velocity = mouse - lastMouse;
        for (int i=0; i<3; i++) {
            if (flexDrawForces[i].getType() == FT_VELOCITY)
                flexDrawForces[i].setForce(velocity);
            flexDrawForces[i].applyForce(mouse);
        }
        lastMouse.set(mouse.x, mouse.y);
    }
}

void gFrameApp::mouseDragged(int x, int y, int button) {
    ofVec2f mouse;
    
    mouse.set(x / (float)ofGetWindowWidth(), y / (float)ofGetWindowHeight());
    ofVec2f velocity = mouse - lastMouse;
    if (button == 0) {
        
        for (int i=0; i<3; i++) {
            if (flexDrawForces[i].getType() == FT_VELOCITY)
                flexDrawForces[i].setForce(velocity);
            flexDrawForces[i].applyForce(mouse);
        }
    }
    else {
        
        for (int i=3; i<numDrawForces; i++) {
            if (flexDrawForces[i].getType() == FT_VELOCITY)
                flexDrawForces[i].setForce(velocity);
            flexDrawForces[i].applyForce(mouse);
        }
    }
    lastMouse.set(mouse.x, mouse.y);
    
}


//--------------------------------------------------------------
void gFrameApp::tuioAdded(ofxTuioCursor &cursor) {
    if(input_tuio){
        GPoint the_point;
        float x,y;
        if (orientation == PORTRAIT) {
            //normalized value
            x = 1-cursor.getY();
            y = 1-cursor.getX();
        } else {
            x = cursor.getX();
            y = cursor.getY();
        }

        the_point.setLocation(ofVec2f(x, y));
        the_point.setId(cursor.getFingerId());
        the_point.setStrokeId(cursor.getSessionId());
        the_point.setColor(localBrushColor);
        the_point.setType(TUIO);
        the_point.setStyle(current_style);
        stroke_list.addToNewStroke(the_point);
        
        finger_positions[cursor.getFingerId()] = ofVec2f(x, y);
        
        stop_pulsing();
        last_points_time = ofGetElapsedTimeMillis();
        
        //flowtools
        ofVec2f this_point = ofVec2f(x,y);
        ofVec2f velocity = this_point - last_touch_points[cursor.getFingerId()];
        for (int i=0; i<3; i++) {
            if (flexDrawForces[i].getType() == FT_VELOCITY)
                flexDrawForces[i].setForce(velocity);
            flexDrawForces[i].applyForce(this_point);
        }
        last_touch_points[cursor.getFingerId()].set(x,y);

    }
    
}

//--------------------------------------------------------------
void gFrameApp::tuioUpdated(ofxTuioCursor &cursor) {
    if(input_tuio){
        GPoint the_point;
        float x,y;
        if (orientation == PORTRAIT) {
            //            x = outputRect.width-cursor.getY()*outputRect.width;
            //            y = outputRect.height-cursor.getX()*outputRect.height;
            //normalized value
            x = 1-cursor.getY();
            y = 1-cursor.getX();
        } else {
            //            x = cursor.getX()*outputRect.width;
            //            y = cursor.getY()*outputRect.height;
            x = cursor.getX();
            y = cursor.getY();
        }
        
        the_point.setLocation(ofVec2f(x, y));
        the_point.setId(cursor.getFingerId());
        the_point.setStrokeId(cursor.getSessionId());
        the_point.setColor(localBrushColor);
        the_point.setType(TUIO);
        the_point.setStyle(current_style);
        stroke_list.add(the_point);
        
        finger_positions[cursor.getFingerId()] = ofVec2f(x, y);
        
        stop_pulsing();
        last_points_time = ofGetElapsedTimeMillis();
        
        //flowtools
        ofVec2f this_point = ofVec2f(x,y);
        ofVec2f velocity = this_point - last_touch_points[cursor.getFingerId()];
        for (int i=0; i<3; i++) {
            if (flexDrawForces[i].getType() == FT_VELOCITY)
                flexDrawForces[i].setForce(velocity);
            flexDrawForces[i].applyForce(this_point);
        }
        last_touch_points[cursor.getFingerId()].set(x,y);

    }
}

void gFrameApp::tuioRemoved(ofxTuioCursor & cursor){
    if(input_tuio){
        finger_positions[cursor.getFingerId()] = ofVec2f(0, 0);
    }
}


//--------------------------------------------------------------
void gFrameApp::oscUpdate() {
    while (receiver.hasWaitingMessages())
    {
        ofxOscMessage m;
        receiver.getNextMessage(&m);
        cout << m.getAddress() << " " << m.getArgAsString(0) << endl;
         //brush style
        if (m.getAddress() == "/sty_wild") current_style = STYLE_SCRIZZLE;
        else if (m.getAddress() == "/sty_threedee") current_style = STYLE_PROFILE;
        else if (m.getAddress() == "/sty_brush") current_style = STYLE_CALIGRAPHY;
        //style color
        else if (m.getAddress() == "/col_red") localBrushColor = ofColor::red;
        else if (m.getAddress() == "/col_green") localBrushColor = ofColor::green;
        else if (m.getAddress() == "/col_blue") localBrushColor = ofColor::blue;
        else if (m.getAddress() == "/col_yellow") localBrushColor = ofColor::yellow;
        else if (m.getAddress() == "/col_orange") localBrushColor = ofColor::orange;
        else if (m.getAddress() == "/col_pink") localBrushColor = ofColor::pink;
        else if (m.getAddress() == "/clearcanvas") stroke_list.clear();

        //style settings tab
        //wild aka scrizzle
        else if (m.getAddress() =="/wild_amplitude") W_amplitude = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/wild_wavelength") W_wavelength = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/wild_mainline") W_mainLine_thickness = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/wild_bylines") W_byLine_thicknes = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/wild_nervosity") W_nervosity = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/wild_fadeouttime") W_fadeout_time = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/wild_fadeduration") W_fadeduration = ofToFloat(m.getArgAsString(0));
        //threedee aka profile
        else if (m.getAddress() == "/td_depth") style_profile_depth = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/td_width") style_profile_width = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/td_zspeed") style_profile_zspeed = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/td_twist") style_profile_twist = ofToFloat(m.getArgAsString(0));

        //caligraphy alias brush
        else if (m.getAddress() == "/br_min_stroke_width") C_width_min = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/br_max_stroke_width") C_width_max = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/br_fadeouttime") C_fadeout_time = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/br_fadeduration") C_fadeduration = ofToFloat(m.getArgAsString(0));

        //admin tab
        else if (m.getAddress() == "/admin_dmx") dmx_on = m.getArgAsInt32(0);
        else if (m.getAddress() == "/admin_dmx_up_lim") upper_pulsing_limit = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/admin_dmx_low_lim") lower_pulsing_limit = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/admin_dmx_brightness") LED_brightness = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/admin_dmx_pulsing_time") LED_frequency = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/admin_points_lifetime") point_lifetime = ofToFloat(m.getArgAsString(0));
        else if (m.getAddress() == "/admin_points_newpointdistance") newPointDistance = ofToFloat(m.getArgAsString(0));
        
        //stencil stuff
        else if (m.getAddress() == "/stencilText")
            changeStencilText(m.getArgAsString(0));
    }

    if (ofGetElapsedTimef() - last_ipad_update_time > 0.04) {
        oscupdate_interface();
        last_ipad_update_time = ofGetElapsedTimef();
    }

}

void gFrameApp::oscupdate_interface() {
    ofxOscMessage update;
    
    //styles
    update.clear();
    update.setAddress("/1/t_wild");
    if (current_style == STYLE_SCRIZZLE) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    
    update.clear();
    update.setAddress("/1/t_threedee");
    if (current_style == STYLE_PROFILE) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);

    update.clear();
    update.setAddress("/1/t_brush");
    if (current_style == STYLE_CALIGRAPHY) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    
/* Color selection will be replaced by color picker
    //color
    //red
    update.clear();
    update.setAddress("/1/t_red");
    if ((ofColor)localBrushColor == ofColor::red) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    //green
    update.clear();
    update.setAddress("/1/t_green");
    if ((ofColor)localBrushColor == ofColor::green) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    //blue
    update.clear();
    update.setAddress("/1/t_blue");
    if ((ofColor)localBrushColor == ofColor::blue) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    //yellow
    update.clear();
    update.setAddress("/1/t_yellow");
    if ((ofColor)localBrushColor == ofColor::yellow) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    //purple
    update.clear();
    update.setAddress("/1/t_orange");
    if ((ofColor)localBrushColor == ofColor::orange) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    //pink
    update.clear();
    update.setAddress("/1/t_pink");
    if ((ofColor)localBrushColor == ofColor::pink) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
*/
  
    //brush settings
    //wild aka scrizzle
    //amplitude
    update.clear();
    update.setAddress("wild_amplitude");
    update.addFloatArg(W_amplitude);
    sender.sendMessage(update);
    //wavelength
    update.clear();
    update.setAddress("wild_wavelength");
    update.addFloatArg(W_wavelength);
    sender.sendMessage(update);
    //mainline
    update.clear();
    update.setAddress("wild_mainline");
    update.addFloatArg(W_mainLine_thickness);
    sender.sendMessage(update);
    //by lines
    update.clear();
    update.setAddress("wild_bylines");
    update.addFloatArg(W_byLine_thicknes);
    sender.sendMessage(update);
    //nervosity
    update.clear();
    update.setAddress("wild_nervosity");
    update.addFloatArg(W_nervosity);
    sender.sendMessage(update);
    //fadeouttime
    update.clear();
    update.setAddress("wild_fadeouttime");
    update.addFloatArg(W_fadeout_time);
    sender.sendMessage(update);
    //fadeduration
    update.clear();
    update.setAddress("wild_fadeduration");
    update.addFloatArg(W_fadeduration);
    sender.sendMessage(update);
    
    //profile
    //depth
    update.clear();
    update.setAddress("td_depth");
    update.addFloatArg(style_profile_depth);
    sender.sendMessage(update);
    //width
    update.clear();
    update.setAddress("td_width");
    update.addFloatArg(style_profile_width);
    sender.sendMessage(update);
    //z-speed
    update.clear();
    update.setAddress("td_zspeed");
    update.addFloatArg(style_profile_zspeed);
    sender.sendMessage(update);
    //twist
    update.clear();
    update.setAddress("td_twist");
    update.addFloatArg(style_profile_twist);
    sender.sendMessage(update);
    
    //caligraphy
    update.clear();
    update.setAddress("br_min_stroke_width");
    update.addFloatArg(C_width_min);
    sender.sendMessage(update);
    
    update.clear();
    update.setAddress("br_max_stroke_width");
    update.addFloatArg(C_width_max);
    sender.sendMessage(update);
    
    update.clear();
    update.setAddress("br_fadeouttime");
    update.addFloatArg(C_fadeout_time);
    sender.sendMessage(update);
    
    update.clear();
    update.setAddress("br_adeduration");
    update.addFloatArg(C_fadeduration);
    sender.sendMessage(update);

    
    //admin settings
    update.clear();
    update.setAddress("admin_dmx");
    if (dmx_on) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    //upper
    update.clear();
    update.setAddress("admin_dmx_up_lim");
    update.addFloatArg(upper_pulsing_limit);
    sender.sendMessage(update);
    //lower
    update.clear();
    update.setAddress("admin_dmx_low_lim");
    update.addFloatArg(lower_pulsing_limit);
    sender.sendMessage(update);
    //brightness
    update.clear();
    update.setAddress("admin_dmx_brightness");
    update.addFloatArg(LED_brightness);
    sender.sendMessage(update);
    //frequency
    update.clear();
    update.setAddress("admin_dmx_pulsing_time");
    update.addFloatArg(LED_frequency);
    sender.sendMessage(update);
    
    //remote_ip
    update.clear();
    update.setAddress("/3/l_remoteip");
    update.addStringArg(remote_ip);
    sender.sendMessage(update);
    //connected
    update.clear();
    update.setAddress("/3/led_connected");
    if (network.getNumClients() > 0)
        update.addFloatArg(1);
    else
        update.addFloatArg(0);
    sender.sendMessage(update);
    
    //lifetime
    update.clear();
    update.setAddress("admin_points_lifetime");
    update.addFloatArg(point_lifetime);
    sender.sendMessage(update);

    //new point distance
    update.clear();
    update.setAddress("admin_points_newpointdistance");
    update.addFloatArg(newPointDistance);
    sender.sendMessage(update);

}
//--------------------------------------------------------------
void gFrameApp::updateLEDpulsing(){
    //create triangle wave for pulsing led lights
    int time = abs(((int)ofGetElapsedTimeMillis() % (LED_pulsing_time*2)) - LED_pulsing_time);
    
    //check how long no point has been added
    if (ofGetElapsedTimeMillis() - last_points_time > 500 && !LED_pulsing)
    {
        LED_level -= 0.01;
        float new_level = ofMap(time, 0, LED_pulsing_time, lower_pulsing_limit, upper_pulsing_limit);
        if (LED_level - new_level < 0)
            start_pulsing();
    }
    float ledlevel2;
    if (LED_pulsing) {
        //int time = abs(((int)ofGetElapsedTimeMillis() % (LED_pulsing_time*2)) - LED_pulsing_time);
        LED_level = ofMap(time, 0, LED_pulsing_time, lower_pulsing_limit, upper_pulsing_limit);
    }
    
    setLEDColor(localBrushColor);
}

void gFrameApp::start_pulsing() {
    //LED_level = 0.0;
    LED_pulsing =true;
}

void gFrameApp::stop_pulsing() {
    LED_pulsing = false;
    LED_level = 1.0;
}

void gFrameApp::setLEDColor(ofColor color){
    //send to color to the dmx device
    int r,g,b;
    float fr = 0,fg = 0,fb = 0;
    r = (int)color.r; fr = (float)r * LED_level*LED_brightness;
    g = (int)color.g; fg = (float)g * LED_level*LED_brightness;
    b = (int)color.b; fb = (float)b * LED_level*LED_brightness;
    //dmx channels are 2, 3 & 4
    dmx.setLevel(2, (int)fr);     //red
    dmx.setLevel(3, (int)fg);     //green
    dmx.setLevel(4, (int)fb);     //blue
    dmx.update();
}

void gFrameApp::guiSetup() {
    //GUI Setup
    gui.setup();
    gui.setPosition(ofGetWidth() - gui.getWidth() - 10, 10);
    gui.setName("general settings");
    
    ///output settings
    parameters_output.setName("output settings");
    outputwidth.setName("width");
    outputheight.setName("height");
    
    parameters_output.add(outputwidth);
    parameters_output.add(outputheight);
    parameters_output.add(stencilText);
    
    //DMX & LEDs
    dmx_settings.setName("dmx settings");
    dmx_settings.add(dmx_on.set("DMX on", false));
    dmx_settings.add(upper_pulsing_limit.set("upper PL",0,0,1));
    dmx_settings.add(lower_pulsing_limit.set("lower PL", 0,0,1));
    dmx_settings.add(LED_brightness.set("LED brightness",1.0,0.0,1));
    dmx_settings.add(LED_frequency.set("LED freqency",2000,500,5000));
    
    ///OSC
    parameters_osc.setName("osc");
    ipad_ip.setName("iPad IP");
    parameters_osc.add(ipad_ip);
    ipad_port.setName("iPad Port");
    parameters_osc.add(ipad_port);
    local_osc_port.setName("local OSC port");
    parameters_osc.add(local_osc_port);
    
    ///network
    parameters_network.setName("network");
    host_port.setName("host port");
    remote_ip.setName("remote ip");
    remote_port.setName("remote port");
    
    parameters_network.add(host_port);
    parameters_network.add(remote_ip);
    parameters_network.add(remote_port);

    ///general brush settings
    parameters_brush.setName("brush settings");
    newPointDistance.set("new point distance", 10,1,100);
    parameters_brush.add(newPointDistance);
    parameters_brush.add(point_lifetime.set("point lifetime", 10, 1, 100));
    localBrushColor.setName("color");
    parameters_brush.add(localBrushColor.set("local color", ofColor(255, 255, 255), ofColor(0,0,0), ofColor(255,255,255)));
    
    // finger positions
    parameters_finger.setName("finger positions");
    parameters_finger.add(draw_finger_positions.set("draw finger positions", true));
    parameters_finger.add(finger_position_size.set("finger circle radius", 30, 2, 100));
    
    // input settings
    parameters_input.setName("input selection");
    parameters_input.add(tuioPort.set("TUIO port", 3333));
    parameters_input.add(input_mouse.set("mouse", false));
    parameters_input.add(input_pqlabs.set("pqlabs", false));
    parameters_input.add(input_tuio.set("tuio", true));
    
    //add the subgroups to main parameter group
    parameters.add(dmx_settings);
    parameters.add(parameters_network);
    parameters.add(parameters_osc);
    parameters.add(parameters_finger);
    parameters.add(parameters_brush);
    parameters.add(parameters_input);

    //add all parameters to the gui
    gui.add(parameters);
}
void gFrameApp::styleGuiSetup() {
    //more specific style settings
    style_gui.setup();
    style_gui.setName("style settings");
    style_gui.setPosition(ofGetWidth() - 2*style_gui.getWidth() - 20, 10);
    
    parameters_profile_style.setName("profile parameters");
    parameters_profile_style.add(style_profile_depth.set("depth", 10, 2, 50));
    parameters_profile_style.add(style_profile_width.set("width", 10, 2, 50));
    parameters_profile_style.add(style_profile_zspeed.set("z-speed", 1, 1, 100));
    parameters_profile_style.add(style_profile_twist.set("twist", 5, 2, 20));
    style_settings.add(parameters_profile_style);
    
    wild_parameters.setName("wild parameters");
    wild_parameters.add(W_amplitude.set("amplitude",8.0,0.0,20));
    wild_parameters.add(W_wavelength.set("wavelength", 4.0, 1.0, 10.0));
    wild_parameters.add(W_fadeout_time.set("fadeout time",10.0,2.0,60.0));
    wild_parameters.add(W_fadeduration.set("fade duration", 5.0, 0.0, 60));
    wild_parameters.add(W_nervosity.set("nervousity",1.0,0.5,20.0));
    wild_parameters.add(W_mainLine_thickness.set("main line thickness", 4.0, 1.0, 10.0));
    wild_parameters.add(W_byLine_thicknes.set("by line thickness", 0.5, 0.1, 5.0));
    style_settings.add(wild_parameters);
    
    caligraphy_parameters.setName("caligraphy parameters");
    caligraphy_parameters.add(C_width_min.set("width min", 1, 0, 20));
    caligraphy_parameters.add(C_width_max.set("width max", 20, 1, 60));
    caligraphy_parameters.add(C_fadeout_time.set("fadeout time",10.0,2.0,60.0));
    caligraphy_parameters.add(C_fadeduration.set("fade duration", 5.0, 0.0, 60));
    style_settings.add(caligraphy_parameters);
    
    style_gui.add(style_settings);
}

void gFrameApp::onSettingsReload() {
    gui.loadFromFile("settings.xml");
    //network
    network.disconnect();
    network.setup(host_port, remote_ip, remote_port);
    //osc
    receiver.setup(local_osc_port);
    try {
        sender.setup(ipad_ip,ipad_port);
        use_ipad = true;
    } catch (exception e) {
        ofLogError() << "unable to connect to iPad on " << ipad_ip <<":"<< ipad_port << endl;
        use_ipad = false;
    }
    //tuio
    tuioClient.start(tuioPort);
    //clean stroklist
    stroke_list.clear();
    
    cout << "New settings loaded" << endl;
}

void gFrameApp::onSettingsSave() {
    gui.saveToFile("settings.xml");
}

void gFrameApp::onStyleSettingsreload() {
    style_gui.loadFromFile("stylesettings.xml");
    ofLog() << "style settings reloaded";
}

void gFrameApp::onStyleSettingsSave() {
    style_gui.saveToFile("stylesettings.xml");
    ofLog() << "style settings saved";    
}

void gFrameApp::flowGuiSetup() {
    flow_gui.setup();
    flow_gui.setName("flow settings");
    flow_gui.setPosition(10, 10);
    flow_gui.add(fluid.parameters);
}

void gFrameApp::onFlowSettingsSave() {
    ofFileDialogResult save_result = ofSystemSaveDialog("NewFlowSettings.xml", "save new flow settings");
    string new_filename = save_result.getPath();
    cout << new_filename << endl;
    if (new_filename != "")
        flow_gui.saveToFile(new_filename);
}

void gFrameApp::onFlowSettingsReload() {
    ofFileDialogResult load_result = ofSystemLoadDialog();
    string load_filename = load_result.getPath();
    if (load_filename != "")
        flow_gui.loadFromFile(load_filename);
}

void gFrameApp::changeStencilText(string _newStencilText) {
    stencilText = _newStencilText;
    stencilFont.loadFont("AkzidenzGrotesk-Cond.otf", 150);
    stencilFBO.allocate(outputRect.width, outputRect.height, GL_RGBA, 2);
    stencilFBO.begin();
    ofClear(0);
    ofRectangle stringBounds = stencilFont.getStringBoundingBox(_newStencilText, 0, 0);
    int text_x = (ofGetWidth() - stringBounds.width) / 2;
    int text_y = (ofGetHeight() + stringBounds.height) /2;
    stencilFont.drawString(_newStencilText, text_x, text_y);
    stencilFBO.end();
    fluid.reset_obstacle();
    fluid.addObstacle(stencilFBO.getTextureReference());
    fluid.addTempObstacle(stencilFBO.getTextureReference());
}