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
    ofSetWindowShape(outputRect.width, outputRect.height);
    
    //brush setup
    setupWildBrush();
    
    //GUI setup
    guiSetup();
    ofAddListener(gui.loadPressedE, this, &gFrameApp::onSettingsReload);
    gui.loadFromFile("settings.xml");
    
    //Syphon stuff
    syphonMainOut.setName("gFrame Main Out");
    
    //TUIO setup
    pqlabsframe.connect("127.0.0.1");
    ofAddListener(pqlabsframe.touchEventDispatcher, this, &gFrameApp::onTouchPoint);
    tuioClient.start(3333);
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
    sender.setup(ipad_ip,ipad_port);
    
    // SETUP LIGHT
    light.enable();
    light.setPointLight();
    light.setPosition(0,0,0);
    
    // NETWORK
    network.setup(host_port, remote_ip, remote_port);
    stroke_list.setupSync(&network);

    //brazil support
    mPanelPositionAndSize = ofRectangle(37,259,214,167);
    dimSESI = ofRectangle(98,259,93,167);
    dimLED1 = ofRectangle(220,452,768,288);
    dimLED2 = ofRectangle(508, 77, 480, 288);
    grabOrigin = ofVec2f(0.0,0.0);
    mPanels.allocate(mPanelPositionAndSize.width, mPanelPositionAndSize.height, OF_IMAGE_COLOR);
    mPanels.setColor(0);
//    mCanvas.allocate(outputRect.width, outputRect.height, OF_IMAGE_COLOR);
//    mCanvas.grabScreen(0,0, outputRect.width, outputRect.height);

    // initialize finger positions
    for(ofVec2f finger : finger_positions){
        finger = ofVec2f(0,0);
    }
    
}
void gFrameApp::exit(){
//    pqlabsframe.~ofxPQLabs();
    setLEDColor(ofColor::black);
    dmx.disconnect();
    network.disconnect();
}

//--------------------------------------------------------------
void gFrameApp::update(){
    
    stroke_list.update();
    oscUpdate();
    tuioClient.getMessage();
    
    // DMX UPDATE
    if (dmx_on)
        dmxUpdate();
    
    //dealing with different output modes
    ofFbo tempFBO;
            tempFBO.allocate(1024, 768);
            tempFBO.begin();
            ofBackground(0);
    
    switch (outputmode) {
        case SESI:
        {
            toPanelsGFrame(mCanvas, mPanels);
            mPanels.draw(mPanelPositionAndSize.x,mPanelPositionAndSize.y);
            break;
        }
        default:
            mCanvas.draw(outputRect.x, outputRect.y, outputRect.width, outputRect.height);
//            syphonMainOut.publishTexture(&mCanvas.getTextureReference());
            break;
    }
    tempFBO.end();
    
    syphonMainOut.publishTexture(&tempFBO.getTextureReference());
    
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

    // lifetime
    stroke_list.setLifetime(point_lifetime * 1000);
    
    //caligraphy style
}

//--------------------------------------------------------------
void gFrameApp::draw(){
    
    ofBackground(0);
    ofSetColor(255);
    
    for(vector<GPoint> stroke : *stroke_list.getAllStrokes()){
        switch(stroke[0].getStyle()){
            case STYLE_PROFILE:
                profileStyle.render(stroke);
                break;
            case STYLE_SCRIZZLE:
                scrizzleStyle.render(stroke);
                break;
            default:
                profileStyle.render(stroke);
                break;
        }
    }
    
    if(draw_finger_positions){
        drawFingerPositions();
    }
    grabOrigin = ofVec2f((ofGetWidth()-outputRect.width)/2, (ofGetHeight()-outputRect.height)/2);
    //grab the screen for syphon output
    mCanvas.allocate(outputRect.width, outputRect.height, OF_IMAGE_COLOR);
    mCanvas.grabScreen((int)grabOrigin.x, (int)grabOrigin.y, outputRect.width, outputRect.height);
    
    //gui output here
    if(draw_gui){
        gui.draw();
        ofSetColor(255);
        ofDrawBitmapString("style: " + ofToString(current_style), ofGetWidth()-100, ofGetHeight()-55);
        ofDrawBitmapString("r: " + ofToString(network.getReceiveQueueLength()), ofGetWidth()-100, ofGetHeight()-40);
        ofDrawBitmapString("s: " + ofToString(network.getSendQueueLength()), ofGetWidth()-100, ofGetHeight()-25 );
        ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate()), ofGetWidth()-100, ofGetHeight()-10 );
    }
}

void gFrameApp::drawFingerPositions(){
    
    ofPushStyle();
    ofDisableDepthTest(); // disable depth test so the alpha blending works properly
    ofDisableLighting();
    
    ofColor outerColor = localBrushColor;
    outerColor.set(localBrushColor.get().r, localBrushColor.get().g, localBrushColor.get().b, 30);
    
    ofSetColor(localBrushColor);
    int i=0;
    for(ofVec2f finger : finger_positions){
        if(!(finger.x == 0 && finger.y == 0)){
            float incr = (float) ((2 * PI) / 32);
            
            glBegin(GL_TRIANGLE_FAN);
            ofSetColor(localBrushColor);
            glVertex2f(finger.x, finger.y);
            
            ofSetColor(outerColor);
            
            for(int i = 0; i < 32; i++){
                float angle = incr * i;
                float x = ((float) cos(angle) * finger_position_size) + finger.x;
                float y = ((float) sin(angle) * finger_position_size) + finger.y;
                glVertex2f(x, y);
            }
            
            glVertex2f(finger_position_size + finger.x, finger.y);
            glEnd();

            // draw finger id for debugging
            ofSetColor(255, 255, 255, 255);
            ofDrawBitmapString(ofToString(i), finger.x-5, finger.y+5);
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
    else if(key == 'h')
        draw_gui = !draw_gui;
    else if (key == 'd')
        dmx_on = !dmx_on;
    
    
    //switch between different output modes
    else if (key == '1') {
        //ofSetWindowShape(768, 288);
        outputRect.width = dimLED1.width;
        outputRect.height = dimLED1.height;
        outputRect.x =dimLED1.x;
        outputRect.y = dimLED1.y;
        outputmode = LED1;
        orientation = LANDSCAPE;
    }
    else if (key == '2') {
        //ofSetWindowShape(480, 288);
        outputRect.width = dimLED2.width;
        outputRect.height = dimLED2.height;
        outputRect.x =dimLED2.x;
        outputRect.y = dimLED2.y;
        outputmode = LED2;
        orientation = LANDSCAPE;
    }
    else if (key == '3') {
        //ofSetWindowShape(mCanvasPositionAndSize.width, mCanvasPositionAndSize.height);
        outputRect.width = dimSESI.width;
        outputRect.height = dimSESI.height;
        outputRect.x =dimSESI.x;
        outputRect.y = dimSESI.y;
        outputmode = SESI;
        orientation = PORTRAIT;
    }
    else if (key == '4') {
        //ofSetWindowShape(1024, 768);
        outputRect.width = 1024;
        outputRect.height =768;
        outputRect.x = 0;
        outputRect.y = 0;
        outputmode = PROJECTOR;
        orientation = LANDSCAPE;
    }
    else if (key == '5') {
        //ofSetWindowShape(dimSESI.width, dimSESI.height);
        outputRect.width = 512;
        outputRect.height = 768;
        outputRect.x = 0;
        outputRect.y = 0;
        outputmode = PROJECTOR_PORTRAIT;
        orientation = PORTRAIT;
    }

}

//--------------------------------------------------------------
void gFrameApp::mouseMoved(int x, int y){
    
    if(input_mouse){
        GPoint the_point;
        the_point.setLocation(ofVec2f(x,y));
        the_point.setId(0);
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
        the_point.setLocation(ofVec2f(cursor.getX()*ofGetWidth(), cursor.getY()*ofGetHeight()));
        the_point.setId(cursor.getFingerId());
        the_point.setStrokeId(cursor.getSessionId());
        the_point.setColor(localBrushColor);
        the_point.setType(TUIO);
        the_point.setStyle(current_style);
        stroke_list.addToNewStroke(the_point);
        
        finger_positions[cursor.getFingerId()] = ofVec2f(cursor.getX()*ofGetWidth(), cursor.getY()*ofGetHeight());
        
        stop_pulsing();
        last_points_time = ofGetElapsedTimeMillis();
    }
}

//--------------------------------------------------------------
void gFrameApp::tuioUpdated(ofxTuioCursor &cursor) {
    if(input_tuio){
        GPoint the_point;
        the_point.setLocation(ofVec2f(cursor.getX()*ofGetWidth(), cursor.getY()*ofGetHeight()));
        the_point.setId(cursor.getFingerId());
        the_point.setStrokeId(cursor.getSessionId());
        the_point.setColor(localBrushColor);
        the_point.setType(TUIO);
        the_point.setStyle(current_style);
        stroke_list.add(the_point);
        
        finger_positions[cursor.getFingerId()] = ofVec2f(cursor.getX()*ofGetWidth(), cursor.getY()*ofGetHeight());
        
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
void gFrameApp::onTouchPoint(TouchPointEvent &event) {
    
    if(input_pqlabs){
        GPoint the_point;
        int x,y;
        if (orientation == PORTRAIT) {
            int temp = x;
            x = outputRect.width-ofMap(event.touchPoint.y, 0, 1050, 0, outputRect.width);
            y = outputRect.height-ofMap(event.touchPoint.x, 0, 1680, 0, outputRect.height);;
        } else {
            x = ofMap(event.touchPoint.x, 0, 1680, 0, outputRect.width);
            y = ofMap(event.touchPoint.y, 0, 1050, 0, outputRect.height);
        }
        cout << x << " " << y << endl;
        the_point.setLocation(ofVec2f(x, y));
        the_point.setId((int)event.touchPoint.id);
        the_point.setColor(localBrushColor);
        the_point.setType(LOCALFRAME);
        the_point.setStyle(current_style);
        
        switch (event.touchPoint.point_event)
        {
            case TP_DOWN:
            {
                stroke_list.addToNewStroke(the_point);
                finger_positions[event.touchPoint.id] = ofVec2f(x, y);
                break;
            }
            case TP_MOVE:
            {
                stroke_list.add(the_point);
                finger_positions[event.touchPoint.id] = ofVec2f(x, y);
                break;
            }
            case TP_UP:
                finger_positions[event.touchPoint.id] = ofVec2f(0, 0);
                break;
        }
        
        //stop pulsing LEDs
        stop_pulsing();
        last_points_time = ofGetElapsedTimeMillis();
    }
}

//--------------------------------------------------------------
void gFrameApp::setLEDColor(ofColor color){
    int r,g,b;
    float fr = 0,fg = 0,fb = 0;
    r = (int)color.r; fr = (float)r * LED_level;
    g = (int)color.g; fg = (float)g * LED_level;
    b = (int)color.b; fb = (float)b * LED_level;
    //dmx channels are 2, 3 & 4
    dmx.setLevel(2, (int)fg);     //green
    dmx.setLevel(3, (int)fr);     //red
    dmx.setLevel(4, (int)fb);     //blue
    dmx.update();
}

void gFrameApp::oscUpdate() {
    while (receiver.hasWaitingMessages())
    {
        ofxOscMessage m;
        receiver.getNextMessage(&m);
        //brush style
        if (m.getAddress() == "/1/t_wild") current_style = STYLE_SCRIZZLE;
        else if (m.getAddress() == "/1/t_threedee") current_style = STYLE_PROFILE;
        //style color
        else if (m.getAddress() == "/1/t_red") localBrushColor = ofColor::red;
        else if (m.getAddress() == "/1/t_green") localBrushColor = ofColor::green;
        else if (m.getAddress() == "/1/t_blue") localBrushColor = ofColor::blue;
        else if (m.getAddress() == "/1/t_yellow") localBrushColor = ofColor::yellow;
        else if (m.getAddress() == "/1/t_orange") localBrushColor = ofColor::orange;
        else if (m.getAddress() == "/1/t_pink") localBrushColor = ofColor::pink;
        //brush settings tab
        //wild aka scrizzle
        else if (m.getAddress() =="/2/s_w_amplitude") W_amplitude = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_w_wavelength") W_wavelength = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_w_mainline") W_mainLine_thickness = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_w_bylines") W_byLine_thicknes = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_w_norvosity") W_nervosity = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_w_fadeouttime") W_fadeout_time = m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_w_fadeduration") W_fadeduration = m.getArgAsFloat(0);
        //threedee aka profile
        else if (m.getAddress() == "/2/s_td_depth") m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_td_width") m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_td_zspeed") m.getArgAsFloat(0);
        else if (m.getAddress() == "/2/s_td_twist") m.getArgAsFloat(0);
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
    update.setAddress("/1/s_w_amplitude");
    update.addFloatArg(W_amplitude);
    sender.sendMessage(update);
    //wavelength
    update.clear();
    update.setAddress("/1/s_w_wavelength");
    update.addFloatArg(W_wavelength);
    sender.sendMessage(update);
    //mainline
    update.clear();
    update.setAddress("/1/s_w_mainline");
    update.addFloatArg(W_mainLine_thickness);
    sender.sendMessage(update);
    //by lines
    update.clear();
    update.setAddress("/1/s_w_bylines");
    update.addFloatArg(W_byLine_thicknes);
    sender.sendMessage(update);
    //nervosity
    update.clear();
    update.setAddress("/1/s_w_nervosity");
    update.addFloatArg(W_nervosity);
    sender.sendMessage(update);
    //fadeouttime
    update.clear();
    update.setAddress("/1/s_w_fadeouttime");
    update.addFloatArg(W_nervosity);
    sender.sendMessage(update);
    //fadeduration
    update.clear();
    update.setAddress("/1/s_w_fadeduration");
    update.addFloatArg(W_fadeduration);
    sender.sendMessage(update);
    
    //profile
    //depth
    update.clear();
    update.setAddress("/1/s_td_depth");
    update.addFloatArg(style_profile_depth);
    sender.sendMessage(update);
    //width
    update.clear();
    update.setAddress("/1/s_td_width");
    update.addFloatArg(style_profile_width);
    sender.sendMessage(update);
    //z-speed
    update.clear();
    update.setAddress("/1/s_td_zspeed");
    update.addFloatArg(style_profile_zspeed);
    sender.sendMessage(update);
    //twist
    update.clear();
    update.setAddress("/1/s_td_twist");
    update.addFloatArg(style_profile_twist);
    sender.sendMessage(update);
    
    //caligraphy
}

void gFrameApp::dmxUpdate(){ // name of this method is a bit misleading, should be updateLED or something like that
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
    gui.setName("GFrame Settings");
    
    ///output settings
    parameters_output.setName("output settings");
    outputwidth.setName("width");
    outputheight.setName("height");
    parameters_output.add(outputwidth);
    parameters_output.add(outputheight);
    
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

    ///Brushes
    localBrushColor.setName("color");
    parameters_brush.setName("brush settings");
    parameters_brush.add(localBrushColor.set("local color", ofColor(255, 255, 255), ofColor(0,0,0), ofColor(255,255,255)));
    newPointDistance.set("new point distance", 10,1,100);
    parameters_brush.add(newPointDistance);
    parameters_brush.add(point_lifetime.set("point lifetime", 10, 1, 100));

    
    parameters_profile_style.setName("profile style");
    parameters_profile_style.add(style_profile_depth.set("depth", 10, 2, 50));
    parameters_profile_style.add(style_profile_width.set("width", 10, 2, 50));
    parameters_profile_style.add(style_profile_zspeed.set("z-speed", 1, 1, 100));
    parameters_profile_style.add(style_profile_twist.set("twist", 5, 2, 20));
    
    // finger positions
    parameters_finger.setName("finger positions");
    parameters_finger.add(draw_finger_positions.set("draw finger positions", true));
    parameters_finger.add(finger_position_size.set("finger circle radius", 30, 2, 100));
    
    // input settings
    parameters_input.add(input_mouse.set("mouse", false));
    parameters_input.add(input_pqlabs.set("pqlabs", false));
    parameters_input.add(input_tuio.set("tuio", true));
    
    //add the subgroups to main parameter group
    parameters.add(parameters_output);
    parameters.add(parameters_network);
    parameters.add(parameters_osc);
    parameters.add(parameters_finger);
    parameters.add(parameters_brush);
    parameters.add(parameters_profile_style);
    parameters.add(wild_parameters);
    parameters.add(parameters_input);

    //add all parameters to the gui
    gui.add(parameters);
    //minimize gui elements
    gui.minimizeAll();
}

void gFrameApp::setupWildBrush() {
    wild_parameters.setName("Wild Brush Parameters");
    wild_parameters.add(W_amplitude.set("amplitude",8.0,0.0,20));
    //wavelength
    wild_parameters.add(W_wavelength.set("wavelength", 4.0, 1.0, 10.0));
    //fadeouttime
    wild_parameters.add(W_fadeout_time.set("fadeout time",10.0,2.0,60.0));
    //fadeduration
    wild_parameters.add(W_fadeduration.set("fade duration", 5.0, 0.0, 60));
    //nervosity aka speed
    wild_parameters.add(W_nervosity.set("nervousity",1.0,0.5,20.0));
    //mainLine_thickness
    wild_parameters.add(W_mainLine_thickness.set("main line thickness", 4.0, 1.0, 10.0));
    //byline_thickness
    wild_parameters.add(W_byLine_thicknes.set("by line thickness", 0.5, 0.1, 5.0));
}

void gFrameApp::onSettingsReload() {
    //network
    network.disconnect();
    network.setup(host_port, remote_ip, remote_port);
    //osc
    receiver.setup(local_osc_port);
    sender.setup(ipad_ip,ipad_port);
    //clean stroklist
    stroke_list.clear();
    cout << "New settings loaded" << endl;
}