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
    mCanvasPositionAndSize = ofRectangle(98,259,93,167);
    mCanvas.allocate(mCanvasPositionAndSize.width, mCanvasPositionAndSize.height, OF_IMAGE_COLOR);
    mPanels.allocate(mPanelPositionAndSize.width, mPanelPositionAndSize.height, OF_IMAGE_COLOR);
    mPanels.setColor(0);
//    fiespMask.loadImage("SP_Urban_MASK_025.png");

    
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
    dmxUpdate();
    
    //dealing with different output modes
    //might be crushed down to a simple if statement if there are no options to deal with for the other outut modes
    switch (outputmode) {
        case SESI:
        {
            toPanelsGFrame(mCanvas, mPanels);
            ofFbo tempFBO;
            tempFBO.allocate(1024, 768);
            tempFBO.begin();
            ofBackground(0);
            //fiespMask.draw(0,0);
            mPanels.draw(mPanelPositionAndSize.x,mPanelPositionAndSize.y);
            tempFBO.end();
            syphonMainOut.publishTexture(&tempFBO.getTextureReference());
            break;
        }
        default:
            syphonMainOut.publishTexture(&mCanvas.getTextureReference());
            break;
    }
    
    //update the brush settings
    //scrizzle style
    scrizzleStyle.setMainLineThickness(W_mainLine_thickness, W_byLine_thicknes);
    scrizzleStyle.setAmplitude(W_amplitude);
    scrizzleStyle.setLength(W_wavelength);
    scrizzleStyle.setNervousity(W_nervosity);
    scrizzleStyle.setFadeOutTime(W_fadeout_time*1000.0, W_fadeduration*1000.0);
    scrizzleStyle.setNewPointDistance(newPointDistance);
    
    //fadeouttime
    
    
    //profile style
    
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
    
    //some texture juggling if outputmode is SESI
    mCanvas.allocate(outputRect.width, outputRect.height, OF_IMAGE_COLOR);
    mCanvas.grabScreen(0, 0, outputRect.width, outputRect.height);
    
    //debug output here
    glDisable(GL_DEPTH_TEST);
    gui.draw();
    glEnable(GL_DEPTH_TEST);
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
        outputRect.width = 768;
        outputRect.height = 288;
        scrizzleStyle.setNewPointDistance(outputRect.width/50.0);
        outputmode = LED1;
        orientation = LANDSCAPE;
    }
    else if (key == '2') {
        //ofSetWindowShape(480, 288);
        outputRect.width = 480;
        outputRect.height = 288;
        scrizzleStyle.setNewPointDistance(outputRect.width/50.0);
        outputmode = LED2;
        orientation = LANDSCAPE;
    }
    else if (key == '3') {
        //ofSetWindowShape(mCanvasPositionAndSize.width, mCanvasPositionAndSize.height);
        outputRect.width = mCanvasPositionAndSize.width;
        outputRect.height = mCanvasPositionAndSize.height;
        outputmode = SESI;
        orientation = PORTRAIT;
    }
    else if (key == '4') {
        //ofSetWindowShape(1024, 768);
        outputRect.width = 1024;
        outputRect.height =768;
        scrizzleStyle.setNewPointDistance(outputRect.width/25.0);
     
        outputmode = PROJECTOR;
        orientation = LANDSCAPE;
    }
    else if (key == '5') {
        //ofSetWindowShape(mCanvasPositionAndSize.width, mCanvasPositionAndSize.height);
        outputRect.width = 512;
        outputRect.height = 768;
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
    
    stop_pulsing();
    last_points_time = ofGetElapsedTimeMillis();
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
            break;
        }
        case TP_MOVE:
        {
            stroke_list.add(the_point);
            break;
        }
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
        
    //add the subgroups to main parameter group
    parameters.add(parameters_output);
    parameters.add(parameters_network);
    parameters.add(parameters_osc);
    parameters.add(parameters_brush);
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