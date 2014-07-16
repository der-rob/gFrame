#include "gFrameApp.h"

//--------------------------------------------------------------
void gFrameApp::setup(){
    //just set up the openFrameworks stuff
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
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
    timeToDie = 5.0;
    
    //DMX for controlling RGB LED Strips
    dmx.connect(0);
    setLEDColor(ofColor::fromHsb(0,255,10));
    LED_pulsing = true;
    LED_pulsing_time = 2000; //in milliseconds
    LED_level = 0.0;
    
    
    //OSC
    receiver.setup(8000);
    
    //drawing parameters
    localDrawingParameters.setName("Local drawing parameters");
    localDrawingParameters.add(localPenColor.set("local pencolor", ofColor::white));
    localDrawingParameters.add(localPenWidth.set("local penwidth", 10));
    

}
void gFrameApp::exit(){
    setLEDColor(ofColor::black);
    dmx.disconnect();
}

//--------------------------------------------------------------
void gFrameApp::update(){
    
    oscUpdate();
    tuioClient.getMessage();
    
    //calculating time to life
    //todo: sort point by type
    for(int i = 1; i < all_points.size(); i++)
    {
        all_points[i].lifetime += 0.01;
        if (all_points[i].lifetime > 5.0 )
        {
            all_points.erase(all_points.begin() + i);
        }
    }
    
    //check how long no point has been added
    
    if (ofGetElapsedTimeMillis() - last_points_time > 500) start_pulsing();
    //fix needed: there is a jump in lumiosity when changing from full brightness while drawing to pulsing mode
    
    
    if (LED_pulsing)
    {
        //create triangle wave
        int time = abs(((int)ofGetElapsedTimeMillis() % (LED_pulsing_time*2)) - LED_pulsing_time);
        LED_level = ofMap(time, 0, LED_pulsing_time, 0.05, 0.6);
    }
    
    setLEDColor(localPenColor);
}

//--------------------------------------------------------------
void gFrameApp::draw(){
    //draw all

    for(int i = 1; i < all_points.size(); i++)
    {
        {
            if (all_points[i].lifetime >= 0.0)
            {
                ofSetColor(all_points[i].color, ofMap(all_points[i].lifetime, timeToDie, 0.0, 0, 255));
                ofCircle(all_points[i].loc.x, all_points[i].loc.y, 2);
                //ofLine(all_points[i-1].loc.x, all_points[i-1].loc.y, all_points[i].loc.x, all_points[i].loc.y);
            }
            /*else
                //erase the first element in the vector, it is supposed to have lifetime < 0
                all_points.erase(all_points.begin());
        */
        }
    }
    
    //syphon
    syphonMainOut.publishScreen();
}

//--------------------------------------------------------------
void gFrameApp::keyPressed(int key){

}

//--------------------------------------------------------------
void gFrameApp::keyReleased(int key){

}

//--------------------------------------------------------------
void gFrameApp::mouseMoved(int x, int y){
    gPoint the_point;
    the_point.loc = ofVec2f(x,y);
    the_point.point_id = 0;
    the_point.color = localPenColor;
    the_point.type = MOUSE;
    the_point.lifetime = 0;
    all_points.push_back(the_point);
    
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
    gPoint the_point;
    int x = ofMap(event.touchPoint.x, 0, 1680, 0, ofGetWidth());
    int y = ofMap(event.touchPoint.y, 0, 1080, 0, ofGetHeight());
    the_point.loc = ofVec2f(x, y);
    the_point.point_id = (int)event.touchPoint.id;
    the_point.color = localPenColor;
    the_point.type = LOCALFRAME;
    the_point.lifetime = 0;
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
        if (m.getAddress() == "/1/push_red") localPenColor = ofColor::red;
        else if (m.getAddress() == "/1/push_green") localPenColor = ofColor::green;
        else if (m.getAddress() == "/1/push_blue") localPenColor = ofColor::blue;
    }
}

void gFrameApp::tuioAdded(ofxTuioCursor &cursor) {
    gPoint the_point;
    the_point.loc = ofVec2f(cursor.getX()*ofGetWidth(), cursor.getY()*ofGetHeight());
    the_point.point_id = cursor.getFingerId();
    the_point.color = localPenColor;
    the_point.type = TUIO;
    the_point.lifetime = 0;
    all_points.push_back(the_point);
    
    stop_pulsing();
    last_points_time = ofGetElapsedTimeMillis();
    //points_t[id].push_back(TUIOpoint);
    
}

void gFrameApp::tuioUpdated(ofxTuioCursor &cursor) {
    gPoint the_point;
    the_point.loc = ofVec2f(cursor.getX()*ofGetWidth(), cursor.getY()*ofGetHeight());
    the_point.point_id = cursor.getFingerId();
    the_point.color = localPenColor;
    the_point.type = TUIO;
    the_point.lifetime = 0.0;
    all_points.push_back(the_point);
    
    stop_pulsing();
    last_points_time = ofGetElapsedTimeMillis();
}

void gFrameApp::tuioRemoved(ofxTuioCursor &cursor) {

}

void gFrameApp::start_pulsing() {
    LED_level = 0.0;
    LED_pulsing =true;
}

void gFrameApp::stop_pulsing() {
    LED_pulsing = false;
    LED_level = 1.0;
}