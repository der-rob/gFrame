#include "gFrameApp.h"

//--------------------------------------------------------------
void gFrameApp::setup(){
    //just set up the openFrameworks stuff
    ofSetFrameRate(60);
    //ofSetVerticalSync(true);
    //ofBackground(82,70,86);
    ofBackground(ofColor::black);
    //ofBackground(ofColor::white);
    ofSetWindowShape(840, 540);
    //Syphon stuff
    syphonMainOut.setName("gFrame Main Out");

    //mutlitouch frame setup
    touchFrame.connect("127.0.0.1");
    ofAddListener(touchFrame.touchEventDispatcher, this, &gFrameApp::onTouchPoint);
    
    //TUIO setup
    tuioClient.start(3333);
    ofAddListener(tuioClient.cursorAdded,this,&gFrameApp::tuioAdded);
	ofAddListener(tuioClient.cursorRemoved,this,&gFrameApp::tuioRemoved);
	ofAddListener(tuioClient.cursorUpdated,this,&gFrameApp::tuioUpdated);
    
    //general Point setup
    timeToDie = 500;
    
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
    
    ofEnableDepthTest(); // IMPORTANT!!!

}
void gFrameApp::exit(){
    setLEDColor(ofColor::black);
    dmx.disconnect();
}

//--------------------------------------------------------------
void gFrameApp::update(){
    
    oscUpdate();

    tuioClient.getMessage();
    
    //todo: sort point by type
    
    //calculating time to life
    for(int i = 0; i < all_points.size(); i++)
    {
//        all_points[i].lifetime += 0.01;
        if ((ofGetElapsedTimeMillis()/10) - all_points[i].getTimestamp() > timeToDie )
        {
            all_points.erase(all_points.begin() + i);
        }
    }
    
    
    //create triangle wave for pulsing led lights
    int time = abs(((int)ofGetElapsedTimeMillis() % (LED_pulsing_time*2)) - LED_pulsing_time);
    
    //check how long no point has been added
    if (ofGetElapsedTimeMillis() - last_points_time > 500 && !LED_pulsing)
    {
        LED_level -= 0.01;
        float new_level = ofMap(time, 0, LED_pulsing_time, lower_pulsing_limit, upper_pulsing_limit);
        cout << new_level << " " << LED_level << endl;
        if (LED_level - new_level < 0)
            start_pulsing();
    }
    
    if (LED_pulsing) {
        //int time = abs(((int)ofGetElapsedTimeMillis() % (LED_pulsing_time*2)) - LED_pulsing_time);
        LED_level = ofMap(time, 0, LED_pulsing_time, lower_pulsing_limit, upper_pulsing_limit);
    }
    
    setLEDColor(localPenColor);
}

//--------------------------------------------------------------
void gFrameApp::draw(){
    //draw all
    

//    for(int i = 1; i < all_points.size(); i++)
//    {
//        {
////            if (all_points[i].lifetime >= 0.0)
////            {
////                ofSetColor(all_points[i].color, ofMap(all_points[i].getTimestamp(), timeToDie, 0.0, 0, 255));
////                ofLine(all_points[i-1].loc, all_points[i].loc);
//                ofCircle(all_points[i].getLocation().x, all_points[i].getLocation().y, 2);
//                //ofLine(all_points[i-1].loc.x, all_points[i-1].loc.y, all_points[i].loc.x, all_points[i].loc.y);
////            }
//        }
//    }
    
    for(vector<GPoint> stroke : *pointList.getAllStrokes()){
        if(stroke[0].getId() == 0)
            profileStyle.render(stroke);
    }
    
    
    //syphon
    syphonMainOut.publishScreen();
}

//--------------------------------------------------------------
void gFrameApp::keyPressed(int key){
    if (key == 'r')
        localPenColor = ofColor::red;
    else if (key == 'b')
        localPenColor = ofColor::blue;
    else if (key == 'g')
        localPenColor = ofColor::green;
        

}

//--------------------------------------------------------------
void gFrameApp::keyReleased(int key){

}

//--------------------------------------------------------------
void gFrameApp::mouseMoved(int x, int y){
    GPoint the_point;
    the_point.setLocation(ofVec2f(x,y));
    the_point.setId(0);
    the_point.setColor(localPenColor);
    the_point.setType(MOUSE);
//    the_point.lifetime = 0;
//    all_points.push_back(the_point);
    pointList.add(the_point);
    
    stop_pulsing();
    last_points_time = ofGetElapsedTimeMillis();
}

//--------------------------------------------------------------
void gFrameApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void gFrameApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void gFrameApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void gFrameApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void gFrameApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void gFrameApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void gFrameApp::onTouchPoint(TouchPointEvent &event) {
    GPoint the_point;
    int x = ofMap(event.touchPoint.x, 0, 1680, 0, ofGetWidth());
    int y = ofMap(event.touchPoint.y, 0, 1080, 0, ofGetHeight());
    the_point.setLocation(ofVec2f(x, y));
    the_point.setId((int)event.touchPoint.id);
    the_point.setColor(localPenColor);
    the_point.setType(LOCALFRAME);
//    the_point.lifetime = 0;
    all_points.push_back(the_point);
    
    //stop pulsing LEDs
    stop_pulsing();
    last_points_time = ofGetElapsedTimeMillis();
}

void gFrameApp::setLEDColor(ofColor color){
    int r,g,b;
    float fr = 0,fg = 0,fb = 0;
    r = (int)color.r; fr = (float)r * LED_level;
    g = (int)color.g; fg = (float)g * LED_level;
    b = (int)color.b; fb = (float)b * LED_level;
    //cout << fr << " " << fg << " " << fb << endl;
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
        else if (m.getAddress() == "/settings/timetolive") timeToDie = m.getArgAsFloat(0);
        else if (m.getAddress() == "/settings/pulsing_limits/2") upper_pulsing_limit = m.getArgAsFloat(0);
        else if (m.getAddress() == "/settings/pulsing_limits/1") lower_pulsing_limit = m.getArgAsFloat(0);
        else if (m.getAddress() == "/settings/push_clear") all_points.clear();
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
    update.addFloatArg(timeToDie);
    sender.sendMessage(update);
    update.clear();
    update.setAddress("/settings/label_ttl");
    update.addStringArg(ofToString(timeToDie));
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

void gFrameApp::tuioAdded(ofxTuioCursor &cursor) {
    GPoint the_point;
    the_point.setLocation(ofVec2f(cursor.getX()*ofGetWidth(), cursor.getY()*ofGetHeight()));
    the_point.setId(cursor.getFingerId());
    the_point.setColor(localPenColor);
    the_point.setType(TUIO);
//    the_point.lifetime = 0;
    pointList.addToNewStroke(the_point);
//    all_points.push_back(the_point);
    
    stop_pulsing();
    last_points_time = ofGetElapsedTimeMillis();
    //points_t[id].push_back(TUIOpoint);
}

void gFrameApp::tuioUpdated(ofxTuioCursor &cursor) {
    GPoint the_point;
    the_point.setLocation(ofVec2f(cursor.getX()*ofGetWidth(), cursor.getY()*ofGetHeight()));
    the_point.setId(cursor.getFingerId());
    the_point.setColor(localPenColor);
    the_point.setType(TUIO);
    //    the_point.lifetime = 0;
//    all_points.push_back(the_point);
    pointList.add(the_point);
    
    stop_pulsing();
    last_points_time = ofGetElapsedTimeMillis();
}

void gFrameApp::tuioRemoved(ofxTuioCursor &cursor) {

}

void gFrameApp::start_pulsing() {
    //LED_level = 0.0;
    LED_pulsing =true;
}

void gFrameApp::stop_pulsing() {
    LED_pulsing = false;
    LED_level = 1.0;
}