#include "gFrameApp.h"

//--------------------------------------------------------------
void gFrameApp::setup(){
    
    //just set up the openFrameworks stuff
    ofSetFrameRate(30);
    ofSetVerticalSync(true);
    ofBackground(ofColor::black);
    ofSetWindowShape(1024, 768);
    


    //dimensions for final output
    outputRect = ofRectangle(0,0,1024, 768);

    //GUI setup
    guiSetup();
    styleGuiSetup();
    ofAddListener(gui.loadPressedE, this, &gFrameApp::onSettingsReload);
    ofAddListener(gui.savePressedE, this, &gFrameApp::onSettingsSave);
    ofAddListener(style_gui.loadPressedE, this, &gFrameApp::onStyleSettingsreload);
    ofAddListener(style_gui.savePressedE, this, &gFrameApp::onStyleSettingsSave);
    
    gui.loadFromFile("settings.xml");
    style_gui.loadFromFile("stylesettings.xml");
    
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
    
    // NETWORK
    network.setup(host_port, remote_ip, remote_port);
    stroke_list.setupSync(&network);

    //brazil support
    syphonFBO.allocate(1024, 768, GL_RGBA, 2);
    canvasFBO.allocate(outputRect.width, outputRect.height, GL_RGBA, 2);
    mCanvas.allocate(outputRect.width, outputRect.height, OF_IMAGE_COLOR);
//    mPanelPositionAndSize = ofRectangle(37,259,214,167);
    mPanelPositionAndSize = ofRectangle(45,259,214,167);
    dimSESI = ofRectangle(98,259,93,167);
    dimLED1 = ofRectangle(220,452,768,288);
    dimLED2 = ofRectangle(508, 77, 480, 288);
    grabOrigin = ofVec2f(0.0,0.0);
    mPanels.allocate(mPanelPositionAndSize.width, mPanelPositionAndSize.height, OF_IMAGE_COLOR);
    mPanels.setColor(0);

    // initialize finger positions
    for(ofVec2f finger : finger_positions){
        finger = ofVec2f(0,0);
    }
    
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
                profileStyle.render(stroke);
                break;
        }
    }
    if(draw_finger_positions){
        drawFingerPositions((int)outputRect.width, (int)outputRect.height);
    }
    canvasFBO.end();
    
    canvasFBO.readToPixels(mCanvas.getPixelsRef());
    mCanvas.reloadTexture();
    
    syphonFBO.begin();
    ofBackground(0);
    ofSetColor(255);
#if DEBUG
    //LED 1
    ofCircle(220, 452, 10);
    ofCircle(220, 452+288, 10);
    ofCircle(220+768, 452, 10);
    ofCircle(220+768, 452+288, 10);
    //LED 2
    ofCircle(508, 77, 10);
    ofCircle(508+480, 77, 10);
    ofCircle(508, 77+288, 10);
    ofCircle(508+480, 77+288, 10);
#endif
    //dealing with different output modes
    switch (outputmode) {
        case SESI:
        {
            toPanelsGFrame(mCanvas, mPanels);
            mPanels.draw(mPanelPositionAndSize.x,mPanelPositionAndSize.y);
            break;
        }
        default:
            mCanvas.draw(outputRect.x, outputRect.y);
            break;
    }
    syphonFBO.end();
    syphonMainOut.publishTexture(&syphonFBO.getTextureReference());
    
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
    

}

//--------------------------------------------------------------
void gFrameApp::draw(){
    
    ofBackground(0);
    ofSetColor(255);
    
    //show size of drawing area
    ofSetColor(255,0, 0);
    ofCircle(grabOrigin, 10);
    ofCircle(grabOrigin.x+outputRect.width, grabOrigin.y,10);
    ofCircle(grabOrigin.x, grabOrigin.y + outputRect.height,10);
    ofCircle(grabOrigin.x+outputRect.width, grabOrigin.y+outputRect.height,10);
    ofSetColor(255);
    
    //switching of the main screen might improve the performance
    if (draw_on_main_screen)
    {
//        mCanvas.draw(0,0);
        syphonFBO.draw(0, 0, ofGetWidth(), ofGetHeight());
    }
    
    //gui output here
    if(draw_gui){
        gui.draw();
        style_gui.draw();
        ofSetColor(255);
        ofDrawBitmapString("clients: " + ofToString(network.isConnected()), ofGetWidth()-120, ofGetHeight()-85);
        ofDrawBitmapString("connected: " + ofToString(network.getNumClients()), ofGetWidth()-120, ofGetHeight()-70);
        ofDrawBitmapString("style: " + ofToString(current_style), ofGetWidth()-120, ofGetHeight()-55);
        ofDrawBitmapString("r: " + ofToString(network.getReceiveQueueLength()), ofGetWidth()-120, ofGetHeight()-40);
        ofDrawBitmapString("s: " + ofToString(network.getSendQueueLength()), ofGetWidth()-120, ofGetHeight()-25 );
        ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate(), 2), ofGetWidth()-120, ofGetHeight()-10 );
    }
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
    else if(key == 'f')
        draw_on_main_screen = !draw_on_main_screen;
    
    
    //switch between different output modes
    else if (key == '1') {
        //ofSetWindowShape(768, 288);
        outputRect.width = dimLED1.width;
        outputRect.height = dimLED1.height;
        outputRect.x =dimLED1.x;
        outputRect.y = dimLED1.y;
        outputmode = LED1;
        orientation = LANDSCAPE;
        canvasFBO.allocate(outputRect.width, outputRect.height);
        mCanvas.allocate(outputRect.width, outputRect.height, OF_IMAGE_COLOR);
    }
    else if (key == '2') {
        //ofSetWindowShape(480, 288);
        outputRect.width = dimLED2.width;
        outputRect.height = dimLED2.height;
        outputRect.x =dimLED2.x;
        outputRect.y = dimLED2.y;
        outputmode = LED2;
        orientation = LANDSCAPE;
        canvasFBO.allocate(outputRect.width, outputRect.height);
        mCanvas.allocate(outputRect.width, outputRect.height, OF_IMAGE_COLOR);
    }
    else if (key == '3') {
        //ofSetWindowShape(mCanvasPositionAndSize.width, mCanvasPositionAndSize.height);
        outputRect.width = dimSESI.width;
        outputRect.height = dimSESI.height;
        outputRect.x =dimSESI.x;
        outputRect.y = dimSESI.y;
        outputmode = SESI;
        orientation = PORTRAIT;
        canvasFBO.allocate(outputRect.width, outputRect.height);
        mCanvas.allocate(outputRect.width, outputRect.height, OF_IMAGE_COLOR);
    }
    else if (key == '4') {
        //ofSetWindowShape(1024, 768);
        outputRect.width = 1024;
        outputRect.height =768;
        outputRect.x = 0;
        outputRect.y = 0;
        outputmode = PROJECTOR;
        orientation = LANDSCAPE;
        canvasFBO.allocate(outputRect.width, outputRect.height);
        mCanvas.allocate(outputRect.width, outputRect.height, OF_IMAGE_COLOR);
    }
    else if (key == '5') {
        //ofSetWindowShape(dimSESI.width, dimSESI.height);
        outputRect.width = 512;
        outputRect.height = 768;
        outputRect.x = 0;
        outputRect.y = 0;
        outputmode = PROJECTOR_PORTRAIT;
        orientation = PORTRAIT;
        canvasFBO.allocate(outputRect.width, outputRect.height);
        mCanvas.allocate(outputRect.width, outputRect.height, OF_IMAGE_COLOR);
    }
    if (key == 'x') {
        fullscreen = !fullscreen;
        ofSetFullscreen(fullscreen);
        style_gui.setPosition(ofGetWidth() - 2*style_gui.getWidth() - 20, 10);
        gui.setPosition(ofGetWidth() - gui.getWidth() - 10, 10);
    }
    if (key == 'm') {
        input_mouse = !input_mouse;
    }

}

//--------------------------------------------------------------
void gFrameApp::mouseMoved(int x, int y){
    
    if(input_mouse){
        GPoint the_point;
        //rescale mouse position
        //float x_norm = ofMap(x, 0, ofGetWidth(), 0.0, outputRect.width);
        float x_norm = ofMap(x, 0, ofGetWidth(), 0.0, 1.0);
        //float y_norm = ofMap(y, 0, ofGetHeight(), 0.0, outputRect.height);
        float y_norm = ofMap(y, 0, ofGetHeight(), 0.0, 1.0);
        
        the_point.setLocation(ofVec2f(x_norm,y_norm));
        the_point.setId(0);
        the_point.setStrokeId(0);
        the_point.setColor(localBrushColor);
        the_point.setStyle(current_style);
        //    the_point.lifetime = 0;
        //    all_points.push_back(the_point);
        stroke_list.add(the_point);
        
        stop_pulsing();
        last_points_time = ofGetElapsedTimeMillis();
    }
}


//--------------------------------------------------------------
void gFrameApp::tuioAdded(ofxTuioCursor &cursor) {
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
        stroke_list.addToNewStroke(the_point);
        
        finger_positions[cursor.getFingerId()] = ofVec2f(x, y);
        
        stop_pulsing();
        last_points_time = ofGetElapsedTimeMillis();
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
        //brush style
        if (m.getAddress() == "/1/t_wild") current_style = STYLE_SCRIZZLE;
        else if (m.getAddress() == "/1/t_threedee") current_style = STYLE_PROFILE;
        else if (m.getAddress() == "/1/t_brush") current_style = STYLE_CALIGRAPHY;
        //style color
        else if (m.getAddress() == "/1/t_red") localBrushColor = ofColor::red;
        else if (m.getAddress() == "/1/t_green") localBrushColor = ofColor::green;
        else if (m.getAddress() == "/1/t_blue") localBrushColor = ofColor::blue;
        else if (m.getAddress() == "/1/t_yellow") localBrushColor = ofColor::yellow;
        else if (m.getAddress() == "/1/t_orange") localBrushColor = ofColor::orange;
        else if (m.getAddress() == "/1/t_pink") localBrushColor = ofColor::pink;
        else if (m.getAddress() == "/1/b_clearcanvas") stroke_list.clear();

        //style settings tab
        //wild aka scrizzle
        else if (m.getAddress() =="/2/s_w_amplitude") W_amplitude = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_w_wavelength") W_wavelength = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_w_mainline") W_mainLine_thickness = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_w_bylines") W_byLine_thicknes = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_w_nervosity") W_nervosity = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_w_fadeouttime") W_fadeout_time = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_w_fadeduration") W_fadeduration = m.getArgAsFloat(0);
        //threedee aka profile
        else if (m.getAddress() == "/2/s_td_depth") style_profile_depth = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_td_width") style_profile_width = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_td_zspeed") style_profile_zspeed = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_td_twist") style_profile_twist = m.getArgAsFloat(0);

        //caligraphy alias brush
        else if (m.getAddress() == "/brush/minwidth") C_width_min = m.getArgAsFloat(0);
        else if (m.getAddress() == "/brush/maxwidth") C_width_max = m.getArgAsFloat(0);
        else if (m.getAddress() == "/brush/fadeout") C_fadeout_time = m.getArgAsFloat(0);
        else if (m.getAddress() == "/brush/fadeduration") C_fadeduration = m.getArgAsFloat(0);

        //admin tab
        else if (m.getAddress() == "/3/t_dmxon") dmx_on = m.getArgAsInt32(0);
        else if (m.getAddress() == "/3/s_upper") upper_pulsing_limit = m.getArgAsFloat(0);
        else if (m.getAddress() == "/3/s_lower") lower_pulsing_limit = m.getArgAsFloat(0);
        else if (m.getAddress() == "/3/s_brightness") LED_brightness = m.getArgAsFloat(0);
        else if (m.getAddress() == "/3/s_frequency") LED_frequency = m.getArgAsFloat(0);
        else if (m.getAddress() == "/3/s_lifetime") point_lifetime = m.getArgAsFloat(0);
        else if (m.getAddress() == "/3/s_newpointdistance") newPointDistance = m.getArgAsFloat(0);}
    
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
    
    //brush settings
    //wild aka scrizzle
    //amplitude
    update.clear();
    update.setAddress("/2/s_w_amplitude");
    update.addFloatArg(W_amplitude);
    sender.sendMessage(update);
    //wavelength
    update.clear();
    update.setAddress("/2/s_w_wavelength");
    update.addFloatArg(W_wavelength);
    sender.sendMessage(update);
    //mainline
    update.clear();
    update.setAddress("/2/s_w_mainline");
    update.addFloatArg(W_mainLine_thickness);
    sender.sendMessage(update);
    //by lines
    update.clear();
    update.setAddress("/2/s_w_bylines");
    update.addFloatArg(W_byLine_thicknes);
    sender.sendMessage(update);
    //nervosity
    update.clear();
    update.setAddress("/2/s_w_nervosity");
    update.addFloatArg(W_nervosity);
    sender.sendMessage(update);
    //fadeouttime
    update.clear();
    update.setAddress("/2/s_w_fadeouttime");
    update.addFloatArg(W_fadeout_time);
    sender.sendMessage(update);
    //fadeduration
    update.clear();
    update.setAddress("/2/s_w_fadeduration");
    update.addFloatArg(W_fadeduration);
    sender.sendMessage(update);
    
    //profile
    //depth
    update.clear();
    update.setAddress("/2/s_td_depth");
    update.addFloatArg(style_profile_depth);
    sender.sendMessage(update);
    //width
    update.clear();
    update.setAddress("/2/s_td_width");
    update.addFloatArg(style_profile_width);
    sender.sendMessage(update);
    //z-speed
    update.clear();
    update.setAddress("/2/s_td_zspeed");
    update.addFloatArg(style_profile_zspeed);
    sender.sendMessage(update);
    //twist
    update.clear();
    update.setAddress("/2/s_td_twist");
    update.addFloatArg(style_profile_twist);
    sender.sendMessage(update);
    
    //caligraphy
    update.clear();
    update.setAddress("/brush/minwidth");
    update.addFloatArg(C_width_min);
    sender.sendMessage(update);
    
    update.clear();
    update.setAddress("/brush/maxwidth");
    update.addFloatArg(C_width_max);
    sender.sendMessage(update);
    
    update.clear();
    update.setAddress("/brush/fadeout");
    update.addFloatArg(C_fadeout_time);
    sender.sendMessage(update);
    
    update.clear();
    update.setAddress("/brush/fadeduration");
    update.addFloatArg(C_fadeduration);
    sender.sendMessage(update);

    
    //admin settings
    update.clear();
    update.setAddress("/3/t_dmxon");
    if (dmx_on) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    //upper
    update.clear();
    update.setAddress("/3/s_upper");
    update.addFloatArg(upper_pulsing_limit);
    sender.sendMessage(update);
    //lower
    update.clear();
    update.setAddress("/3/s_lower");
    update.addFloatArg(lower_pulsing_limit);
    sender.sendMessage(update);
    //brightness
    update.clear();
    update.setAddress("/3/s_brightness");
    update.addFloatArg(LED_brightness);
    sender.sendMessage(update);
    //frequency
    update.clear();
    update.setAddress("/3/s_frequency");
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
    update.setAddress("/3/s_lifetime");
    update.addFloatArg(point_lifetime);
    sender.sendMessage(update);

    //new point distance
    update.clear();
    update.setAddress("/3/s_newpointdistance");
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
//--------------------------------------------------------------
void gFrameApp::toPanels(ofImage &canvas, ofImage &panels){
    if(!(canvas.getWidth() == 214 && canvas.getHeight() == 167))
        return;
    for(int y=0; y<panels.getHeight(); y++){
        int rowWidthHalf = (int)((93.0-51.0)/panels.getHeight()*y/2.0+25.0);
        int rowCenterPixel = y*panels.getWidth()+panels.getWidth()/2;
        // center
        for(int x=0; x<=rowWidthHalf; x++){
            panels.setColor(panels.getWidth()/2+x, y, canvas.getColor(panels.getWidth()/2+x, y));
            panels.setColor(panels.getWidth()/2-x, y, canvas.getColor(panels.getWidth()/2-x, y));
        }
        // left/right
        int gapSize = (int)((0.0-74.0)/panels.getHeight()*y+74.0);
        int leftoverPixels = (int)((61.0-9.0)/panels.getHeight()*y+9.0);
        for(int x=0; x<=leftoverPixels; x++){
            panels.setColor(panels.getWidth()/2+rowWidthHalf+1+x+gapSize, y, localBrushColor);
            panels.setColor(panels.getWidth()/2-rowWidthHalf-1-x-gapSize, y, localBrushColor);
        }
    }
    panels.reloadTexture();
}

void gFrameApp::toPanelsGFrame(ofImage &canvas, ofImage &panels){
    if(!(canvas.getWidth() == 93 && canvas.getHeight() == 167))
        return;
    for(int y=0; y<panels.getHeight(); y++){
        int rowWidthHalf = (int)((93.0-51.0)/panels.getHeight()*y/2.0+25.0);
        int rowCenterPixel = y*panels.getWidth()+panels.getWidth()/2;
        // center
        for(int x=0; x<=rowWidthHalf; x++){
            panels.setColor(panels.getWidth()/2+x, y, canvas.getColor(canvas.getWidth()/2+x, y));
            panels.setColor(panels.getWidth()/2-x, y, canvas.getColor(canvas.getWidth()/2-x, y));
        }
        // left/right
        int gapSize = (int)((0.0-74.0)/panels.getHeight()*y+74.0);
        int leftoverPixels = (int)((61.0-9.0)/panels.getHeight()*y+9.0);
        for(int x=0; x<=leftoverPixels; x++){
            panels.setColor(panels.getWidth()/2+rowWidthHalf+1+x+gapSize, y, localBrushColor);
            panels.setColor(panels.getWidth()/2-rowWidthHalf-1-x-gapSize, y, localBrushColor);
        }
    }
    panels.reloadTexture();
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