#include "gFrameApp.h"

//--------------------------------------------------------------
void gFrameApp::setup(){
    //just set up the openFrameworks stuff
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofBackground(82,70,86);
    
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
    timeTolive = 5.0;
    
    //ensure that points_f is not empty but filled with empty vectors
    for (int i =0; i<12;i++)
    {
        vector<ofVec3f> vec;
        points_f.push_back(vec);
    }
    
    //same for points_t
    for (int i =0; i<12;i++)
    {
        vector<ofVec2f> vec;
        points_t.push_back(vec);
    }
    
    //DMX for controlling RGB LED Strips
    dmx.connect(0);
    setLEDColor(ofColor::fromHsb(0,255,10));
    
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
    for(unsigned int i = 1; i < all_points.size(); i++)
    {
        {
            all_points[i].lifetime -= 0.01;

        }
    }
    
    //setLEDColor(localPenColor);
}

//--------------------------------------------------------------
void gFrameApp::draw(){
    //draw all
    for(unsigned int i = 1; i < all_points.size(); i++)
    {
        {
            if (all_points[i].lifetime >= 0.0)
            {
                ofSetColor(all_points[i].color, ofMap(all_points[i].lifetime, 0.0, timeTolive, 0, 255));
                ofCircle(all_points[i].loc.x, all_points[i].loc.y, 5);
                ofLine(all_points[i-1].loc.x, all_points[i-1].loc.y, all_points[i].loc.x, all_points[i].loc.y);
            }
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
    the_point.lifetime = timeTolive;
    all_points.push_back(the_point);
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
    the_point.loc = ofVec2f(event.touchPoint.x, event.touchPoint.y);
    the_point.point_id = event.touchPoint.id;
    the_point.color = localPenColor;
    the_point.type = LOCALFRAME;
    the_point.lifetime = timeTolive;
    all_points.push_back(the_point);
}

void gFrameApp::setLEDColor(ofColor color){
    int r,g,b;
    r = (int)color.r;
    g = (int)color.g;
    b = (int)color.b;
    //dmx channels are 2, 3 & 4
    dmx.setLevel(2, g);     //green
    dmx.setLevel(3, r);     //red
    dmx.setLevel(4, b);     //blue
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
    the_point.lifetime = timeTolive;
    all_points.push_back(the_point);
    
    //points_t[id].push_back(TUIOpoint);
    
}

void gFrameApp::tuioUpdated(ofxTuioCursor &cursor) {
    gPoint the_point;
    the_point.loc = ofVec2f(cursor.getX()*ofGetWidth(), cursor.getY()*ofGetHeight());
    the_point.point_id = cursor.getFingerId();
    the_point.color = localPenColor;
    the_point.type = TUIO;
    the_point.lifetime = timeTolive;
    all_points.push_back(the_point);
}

void gFrameApp::tuioRemoved(ofxTuioCursor &cursor) {

}