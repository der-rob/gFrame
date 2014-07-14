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
    localDrawingParameters.add(localPenColor.set("local pencolor", ofColor::brown));
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
    
    //setLEDColor(localPenColor);
}

//--------------------------------------------------------------
void gFrameApp::draw(){
    //draw mouse
    ofSetColor(localPenColor.get());
    for(unsigned int i = 1; i < points_m.size(); i++)
    {
        ofCircle(points_m[i].x, points_m[i].y, 10);
        ofLine(points_m[i-1].x, points_m[i-1].y, points_m[i].x, points_m[i].y);
    }
    
    
    //draw local frame
    for(unsigned int i = 0; i < points_f.size(); i++)
    {
        for (int j = 1;j< points_f[i].size(); j++)
        {
            ofCircle(points_f[i][j].x, points_f[i][j].y, 10);
            ofLine(points_f[i][j-1].x, points_f[i][j-1].y, points_f[i][j].x, points_f[i][j].y);
        }
    }
    
    //draw TUIO
    for(unsigned int i = 0; i < points_t.size(); i++)
    {
        for (int j = 1;j< points_t[i].size(); j++)
        {
            ofCircle(points_t[i][j].x, points_t[i][j].y, 10);
            ofLine(points_t[i][j-1].x, points_t[i][j-1].y, points_t[i][j].x, points_t[i][j].y);
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
    ofVec3f mousePoint(x,y,0);
    points_m.push_back(mousePoint);
    LEDstripColor.set(colorFromPoint(mousePoint));

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
    ofVec3f framePoint(event.touchPoint.x,event.touchPoint.y,0);
    
    //sort points by touch id
    int id = event.touchPoint.id;
    points_f[id].push_back(framePoint);
    
    LEDstripColor.set(colorFromPoint(framePoint));
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

ofColor gFrameApp::colorFromPoint(ofVec3f thePoint) {
    float hue = ofMap(thePoint.x, 0, ofGetWidth(), 0,255);
    float sat = ofMap(thePoint.y, 0, ofGetHeight(), 0,255);
    ofColor theColor = ofColor::fromHsb(hue, sat, 200);
    return theColor;
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
    int id = cursor.getFingerId();
    ofVec2f TUIOpoint = ofVec2f(cursor.getX(), cursor.getY());
    cout << TUIOpoint << endl;
    points_t[id].push_back(TUIOpoint);
}

void gFrameApp::tuioUpdated(ofxTuioCursor &cursor) {
    int id = cursor.getFingerId();
    ofVec2f TUIOpoint = ofVec2f(cursor.getX()*ofGetWidth(), cursor.getY()*ofGetHeight());
    cout << TUIOpoint << endl;
    points_t[id].push_back(TUIOpoint);
    
}

void gFrameApp::tuioRemoved(ofxTuioCursor &cursor) {

}