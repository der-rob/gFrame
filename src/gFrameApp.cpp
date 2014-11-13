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
    light.setPosition(0,-300,0);
    
    // SETUP OPENGL
    ofEnableDepthTest(); // IMPORTANT!!!
    
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
//    dmxUpdate();
    
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
    
    //caligraphy style
}

//--------------------------------------------------------------
void gFrameApp::draw(){
    
    ofBackground(0);
    ofSetColor(255);
//    
    
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
    glDisable(GL_DEPTH_TEST);
    gui.draw();
    ofSetColor(200);
    ofDrawBitmapString("r: " + ofToString(network.getReceiveQueueLength()), ofGetWidth()-200, ofGetHeight()-50);
    ofDrawBitmapString("s: " + ofToString(network.getSendQueueLength()), ofGetWidth()-200, ofGetHeight()-25 );
    ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate()), ofGetWidth()-200, ofGetHeight()-10 );
    glEnable(GL_DEPTH_TEST);
}

void gFrameApp::drawFingerPositions(){
    
    // disable depth test so the alpha blending works properly
    ofDisableDepthTest();
    
    ofColor outerColor = localBrushColor;
    outerColor.set(localBrushColor.get().r, localBrushColor.get().g, localBrushColor.get().b, 30);
    
    ofSetColor(localBrushColor);
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
        }
    }
    ofEnableDepthTest();
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

//--------------------------------------------------------------
void gFrameApp::tuioAdded(ofxTuioCursor &cursor) {
    GPoint the_point;
    the_point.setLocation(ofVec2f(cursor.getX()*ofGetWidth(), cursor.getY()*ofGetHeight()));
    the_point.setId(cursor.getFingerId());
    the_point.setColor(localBrushColor);
    the_point.setType(TUIO);
    the_point.setStyle(current_style);
    stroke_list.addToNewStroke(the_point);
    
    finger_positions[cursor.getFingerId()] = ofVec2f(cursor.getX()*ofGetWidth(), cursor.getY()*ofGetHeight());
    
    stop_pulsing();
    last_points_time = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void gFrameApp::tuioUpdated(ofxTuioCursor &cursor) {
    GPoint the_point;
    the_point.setLocation(ofVec2f(cursor.getX()*ofGetWidth(), cursor.getY()*ofGetHeight()));
    the_point.setId(cursor.getFingerId());
    the_point.setColor(localBrushColor);
    the_point.setType(TUIO);
    the_point.setStyle(current_style);
    stroke_list.add(the_point);
    
    finger_positions[cursor.getFingerId()] = ofVec2f(cursor.getX()*ofGetWidth(), cursor.getY()*ofGetHeight());
    
    stop_pulsing();
    last_points_time = ofGetElapsedTimeMillis();
}

void gFrameApp::tuioRemoved(ofxTuioCursor & cursor){
    finger_positions[cursor.getFingerId()] = ofVec2f(0, 0);
}


//--------------------------------------------------------------
void gFrameApp::onTouchPoint(TouchPointEvent &event) {
    
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
        cout << "new osc message" << endl;
        receiver.getNextMessage(&m);
        if (m.getAddress() == "/1/t_red") localBrushColor = ofColor::red;
        else if (m.getAddress() == "/1/t_green") localBrushColor = ofColor::green;
        else if (m.getAddress() == "/1/t_blue") localBrushColor = ofColor::blue;
        else if (m.getAddress() == "/1/t_yellow") localBrushColor = ofColor::yellow;
        else if (m.getAddress() == "/1/t_orange") localBrushColor = ofColor::purple;
        else if (m.getAddress() == "/1/t_pink") localBrushColor = ofColor::pink;
        //settings tab
        else if (m.getAddress() == "/settings/timetolive") stroke_list.setLifetime(m.getArgAsFloat(0));
        else if (m.getAddress() == "/settings/pulsing_limits/2") upper_pulsing_limit = m.getArgAsFloat(0);
        else if (m.getAddress() == "/settings/pulsing_limits/1") lower_pulsing_limit = m.getArgAsFloat(0);
        else if (m.getAddress() == "/settings/push_clear") stroke_list.clear();
    }
    
    
    if (ofGetElapsedTimef() - last_ipad_update_time > 0.04) {
        oscupdate_interface();
        last_ipad_update_time = ofGetElapsedTimef();
    }
}

void gFrameApp::oscupdate_interface() {
    ofxOscMessage update;
    
    //red
    update.clear();
    update.setAddress("/color/red");
    if ((ofColor)localBrushColor == ofColor::red) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    
    //green
    update.clear();
    update.setAddress("/color/green");
    if ((ofColor)localBrushColor == ofColor::green) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    
    //blue
    update.clear();
    update.setAddress("/color/blue");
    if ((ofColor)localBrushColor == ofColor::blue) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    
    //yellow
    update.clear();
    update.setAddress("/color/yellow");
    if ((ofColor)localBrushColor == ofColor::yellow) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    
    //purple
    update.clear();
    update.setAddress("/color/purple");
    if ((ofColor)localBrushColor == ofColor::purple) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    
    //pink
    update.clear();
    update.setAddress("/color/pink");
    if ((ofColor)localBrushColor == ofColor::pink) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    
    //timetolive
    update.clear();
    update.setAddress("/settings/timetolive");
    update.addFloatArg(stroke_list.getLifetime());
    sender.sendMessage(update);
    update.clear();
    update.setAddress("/settings/label_ttl");
    update.addStringArg(ofToString(stroke_list.getLifetime()));
    sender.sendMessage(update);
    
    //pulsing limits
    update.clear();
    update.setAddress("/settings/label_upl");
    update.addStringArg(ofToString(upper_pulsing_limit));
    sender.sendMessage(update);
    
    update.clear();
    update.setAddress("/settings/pulsing_limits/2");
    update.addFloatArg(upper_pulsing_limit);
    sender.sendMessage(update);
    
    update.clear();
    update.setAddress("/settings/pulsing_limits/1");
    update.addFloatArg(lower_pulsing_limit);
    sender.sendMessage(update);
    
    update.clear();
    update.setAddress("/settings/label_lpl");
    update.addStringArg(ofToString(lower_pulsing_limit));
    sender.sendMessage(update);
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
    parameters_brush.add(localBrushColor);
    newPointDistance.set("new point distance", 10,1,100);
    parameters_brush.add(newPointDistance);
        
//    parameters_brush.add(brush_radius.set("brush Radius", 8.0,2.0,20.0));
//    parameters_brush.add(brush_width.set("linewidth", 2.0, 1.0, 4.0));
    localBrushColor.setName("color");
    parameters_brush.add(localBrushColor);
    
    parameters_profile_style.setName("profile style");
    parameters_profile_style.add(style_profile_depth.set("depth", 10, 2, 50));
    parameters_profile_style.add(style_profile_width.set("width", 10, 2, 50));
    parameters_profile_style.add(style_profile_zspeed.set("z-speed", 1, 1, 15));
    parameters_profile_style.add(style_profile_twist.set("depth", 5, 2, 20));
    
    
    // finger positions
    parameters_finger.setName("finger positions");
    parameters_finger.add(draw_finger_positions.set("draw finger positions", true));
    parameters_finger.add(finger_position_size.set("finger circle radius", 30, 2, 100));
    
    
    //add the subgroups to main parameter group
    parameters.add(parameters_output);
    parameters.add(parameters_network);
    parameters.add(parameters_osc);
    parameters.add(parameters_finger);
    parameters.add(parameters_brush);
    parameters.add(parameters_profile_style);
    parameters.add(wild_parameters);
    //add all parameters to the gui
    gui.add(parameters);    
}

void gFrameApp::setupWildBrush() {
    wild_parameters.setName("Wild Brush Parameters");
    
    W_amplitude.set("amplitude",8.0,1.0,20);
    wild_parameters.add(W_amplitude);
    //wavelength
    W_wavelength.set("wavelength", 4.0, 1.0, 10.0);
    wild_parameters.add(W_wavelength);
    //strokewidth
    //may be obsolete since amplitude means the same
    //fadeouttime
    W_fadeout_time.set("fadeout time",10.0,2.0,60.0);
    wild_parameters.add(W_fadeout_time);
    //fadeduration
    W_fadeduration.set("fade duration", 5.0, 2.0, 60);
    wild_parameters.add(W_fadeduration);
    //nervosity aka speed
    W_nervosity.set("nervousity",1.0,0.5,20.0);
    wild_parameters.add(W_nervosity);
    //mainLine_thickness
    W_mainLine_thickness.set("main line thickness", 4.0, 1.0, 10.0);
    wild_parameters.add(W_mainLine_thickness);
    //byline_thickness
    W_byLine_thicknes.set("by line thickness", 0.5, 0.1, 5.0);
    wild_parameters.add(W_byLine_thicknes);
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