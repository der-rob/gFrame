#include "gFrameApp.h"

//--------------------------------------------------------------
void gFrameApp::setup(){
    //just set up the openFrameworks stuff
    ofSetFrameRate(30);
    ofSetVerticalSync(true);
    ofBackground(ofColor::black);
    ofSetWindowShape(1024, 768);
    
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
    receiver.setup(8000);
    sender.setup("192.168.1.37",9000);
    
    //drawing parameters
    localDrawingParameters.setName("Local drawing parameters");
    localDrawingParameters.add(localPenColor.set("local pencolor", ofColor::white));
    localDrawingParameters.add(localPenWidth.set("local penwidth", 10));
    
    // SETUP LIGHT
    light.enable();
    light.setPointLight();
    light.setPosition(0,-300,0);
    
    // SETUP OPENGL
    ofEnableDepthTest(); // IMPORTANT!!!
    
    // NETWORK
    network.setup(9001, "localhost", 9000);
    stroke_list.setupSync(&network);

    //brazil support
    mPanelPositionAndSize = ofRectangle(37,259,214,167);
    mCanvasPositionAndSize = ofRectangle(98,259,93,167);
    mCanvas.allocate(mCanvasPositionAndSize.width, mCanvasPositionAndSize.height, OF_IMAGE_COLOR);
    mPanels.allocate(mPanelPositionAndSize.width, mPanelPositionAndSize.height, OF_IMAGE_COLOR);
    mPanels.setColor(0);
    fiespMask.loadImage("SP_Urban_MASK_025.png");


}
void gFrameApp::exit(){
    pqlabsframe.~ofxPQLabs();
    setLEDColor(ofColor::black);
    dmx.disconnect();
}

//--------------------------------------------------------------
void gFrameApp::update(){
    
    network.update();
    
    stroke_list.update();
    oscUpdate();
    tuioClient.getMessage();
    
    // DMX UPDATE
    dmxUpdate();
    
    
    //dealing with different output modes
    //might be crushed down to a simple if statement if there are no options to deal with for the other outut modes
    switch (outputmode) {
        case PROJECTOR:
        {
            break;
        }
        case LED1:
        {
            break;
        }
        case LED2:
        {
            break;
        }

        case SESI:
        {
            ofFbo tempFBO;
            tempFBO.allocate(1024, 768);
            tempFBO.begin();
            ofBackground(128);
            //fiespMask.draw(0,0);
            mPanels.draw(mPanelPositionAndSize.x,mPanelPositionAndSize.y);
            tempFBO.end();
            syphonMainOut.publishTexture(&tempFBO.getTextureReference());
            break;
        }
        default:
            break;
    }
    
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
    
    //some texture juggling if outputmode is SESI
    if (outputmode == SESI)
    {
//        mCanvas.allocate(1024,768,OF_IMAGE_COLOR);
        mCanvas.allocate(mCanvasPositionAndSize.width, mCanvasPositionAndSize.height,OF_IMAGE_COLOR);
        mCanvas.grabScreen(0, 0, mCanvasPositionAndSize.width, mCanvasPositionAndSize.height);
//        mCanvas.resize(mPanelPositionAndSize.width, mPanelPositionAndSize.height);
        toPanelsGFrame(mCanvas, mPanels);
    } else {
//        syphonMainOut.publishScreen();
    }
}

//--------------------------------------------------------------
void gFrameApp::keyPressed(int key){
    if (key == 'r') {
        localPenColor = ofColor::red;
        
    }
    else if (key == 'b') {
        localPenColor = ofColor::blue;
    }
    else if (key == 'g') {
        localPenColor = ofColor::green;
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
        ofSetWindowShape(768, 288);
        scrizzleStyle.setNewPointDistance(ofGetWidth()/50.0);
        outputmode = LED1;
        orientation = LANDSCAPE;
    }
    else if (key == '2') {
        ofSetWindowShape(480, 288);
        scrizzleStyle.setNewPointDistance(ofGetWidth()/50.0);
        outputmode = LED2;
        orientation = LANDSCAPE;
    }
    else if (key == '3') {
        ofSetWindowShape(mCanvasPositionAndSize.width, mCanvasPositionAndSize.height);
        scrizzleStyle.setNewPointDistance(ofGetWidth()/50.0);outputmode = SESI;
        orientation = PORTRAIT;
    }
    else if (key == '4') {
        ofSetWindowShape(1024, 768);
        scrizzleStyle.setNewPointDistance(ofGetWidth()/50.0);
        outputmode = PROJECTOR;
        orientation = LANDSCAPE;
    }

}

//--------------------------------------------------------------
void gFrameApp::mouseMoved(int x, int y){
    
    GPoint the_point;
    the_point.setLocation(ofVec2f(x,y));
    the_point.setId(0);
    the_point.setColor(localPenColor);
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
    the_point.setColor(localPenColor);
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
    the_point.setColor(localPenColor);
    the_point.setType(TUIO);
    the_point.setStyle(current_style);
    stroke_list.add(the_point);
    
    stop_pulsing();
    last_points_time = ofGetElapsedTimeMillis();
}


//--------------------------------------------------------------
void gFrameApp::onTouchPoint(TouchPointEvent &event) {
    switch (event.touchPoint.point_event)
    {
        case TP_DOWN:
        {
            GPoint the_point;
            int x = ofMap(event.touchPoint.x, 0, 1680, 0, ofGetWidth());
            int y = ofMap(event.touchPoint.y, 0, 1050, 0, ofGetHeight());
            cout << event.touchPoint.x << " " << event.touchPoint.y << endl;
            if (orientation == PORTRAIT) {
                int temp = x;
                x = 1024-y;
                y = 768-temp;
            }

            the_point.setLocation(ofVec2f(x, y));
            the_point.setId((int)event.touchPoint.id);
            the_point.setColor(localPenColor);
            the_point.setType(LOCALFRAME);
            the_point.setStyle(current_style);
            stroke_list.addToNewStroke(the_point);
            
            //stop pulsing LEDs
            stop_pulsing();
            last_points_time = ofGetElapsedTimeMillis();
            break;
        }
        case TP_MOVE:
        {
            GPoint the_point;
            int x = ofMap(event.touchPoint.x, 0, 1680, 0, ofGetWidth());
            int y = ofMap(event.touchPoint.y, 0, 1050, 0, ofGetHeight());
            cout << event.touchPoint.x << " " << event.touchPoint.y << endl;
            if (orientation == PORTRAIT) {
                int temp = x;
                x = 1024-y;
                y = 768-temp;
            }
            
            the_point.setLocation(ofVec2f(x, y));
            the_point.setId((int)event.touchPoint.id);
            the_point.setColor(localPenColor);
            the_point.setType(LOCALFRAME);
            the_point.setStyle(current_style);
            stroke_list.add(the_point);
            
            //stop pulsing LEDs
            stop_pulsing();
            last_points_time = ofGetElapsedTimeMillis();
            break;
        }
        case TP_UP:
        {
            break;
        }
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
        if (m.getAddress() == "/color/red") localPenColor = ofColor::red;
        else if (m.getAddress() == "/color/green") localPenColor = ofColor::green;
        else if (m.getAddress() == "/color/blue") localPenColor = ofColor::blue;
        else if (m.getAddress() == "/color/yellow") localPenColor = ofColor::yellow;
        else if (m.getAddress() == "/color/purple") localPenColor = ofColor::purple;
        else if (m.getAddress() == "/color/pink") localPenColor = ofColor::pink;
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
    if ((ofColor)localPenColor == ofColor::red) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    
    //green
    update.clear();
    update.setAddress("/color/green");
    if ((ofColor)localPenColor == ofColor::green) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    
    //blue
    update.clear();
    update.setAddress("/color/blue");
    if ((ofColor)localPenColor == ofColor::blue) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    
    //yellow
    update.clear();
    update.setAddress("/color/yellow");
    if ((ofColor)localPenColor == ofColor::yellow) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    
    //purple
    update.clear();
    update.setAddress("/color/purple");
    if ((ofColor)localPenColor == ofColor::purple) update.addFloatArg(1);
    else update.addFloatArg(0);
    sender.sendMessage(update);
    
    //pink
    update.clear();
    update.setAddress("/color/pink");
    if ((ofColor)localPenColor == ofColor::pink) update.addFloatArg(1);
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
    
    setLEDColor(localPenColor);
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
            panels.setColor(panels.getWidth()/2+rowWidthHalf+1+x+gapSize, y, localPenColor);
            panels.setColor(panels.getWidth()/2-rowWidthHalf-1-x-gapSize, y, localPenColor);
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
            panels.setColor(panels.getWidth()/2+rowWidthHalf+1+x+gapSize, y, localPenColor);
            panels.setColor(panels.getWidth()/2-rowWidthHalf-1-x-gapSize, y, localPenColor);
        }
    }
    panels.reloadTexture();
}