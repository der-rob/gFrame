#pragma once

#define LOCALFRAME 1
#define REMOTEFRAME 2
#define TUIO 3
#define MOUSE 4

#include "ofMain.h"
#include "ofxPQLabs.h"
#include "ofxSyphon.h"
#include "ofxDmx.h"
#include "ofxOsc.h"
#include "ofxTuio.h"

typedef struct {
    ofVec2f loc;
    int point_id;
    float lifetime;
    ofColor color;
    int type;
} gPoint;

class gFrameApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
        void exit();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    //frame and point data
    ofxPQLabs touchFrame;
    void onTouchPoint(TouchPointEvent &event);

    //vectors containing point data
    //mouse for easier testing
    //vector containing ALL points
    vector <gPoint>  all_points;
    float timeTolive;
    vector<ofVec3f> points_m;
    //local frame
    vector< vector<ofVec3f> > points_f;
    //tuio points for testing without touchframe
    vector< vector<ofVec2f> > points_t;
    
    //TUIO support
    ofxTuioClient   tuioClient;
    void	tuioAdded(ofxTuioCursor & tuioCursor);
	void	tuioRemoved(ofxTuioCursor & tuioCursor);
	void	tuioUpdated(ofxTuioCursor & tuioCursor);
    
    //DMX
    ofxDmx dmx;
    void setLEDColor(ofColor ledColor);
    ofColor LEDstripColor;
    
    //OSC
    ofxOscReceiver receiver;
    void oscUpdate();
    
    //Syphon output
    ofxSyphonServer syphonMainOut;
    
    //drawing parameter
    ofParameterGroup localDrawingParameters;
    ofParameter<int> localPenWidth;
    ofParameter<ofColor> localPenColor;
};


