#pragma once

#define LOCALFRAME 1
#define REMOTEFRAME 2
#define TUIO 3
#define MOUSE 4

#include "ofMain.h"
#include "ofxSyphon.h"
#include "ofxDmx.h"
#include "ofxOsc.h"
#include "ofxTuio.h"
#include "ofxXmlSettings.h"
#include "GPoint.h"
#include "ProfileStyle.h"
#include "StrokeList.h"
#include "ScrizzleStyle.h"
#include "Network.h"


#define STYLE_PROFILE 0
#define STYLE_SCRIZZLE 1

enum OutputMode {SESI, LED1, LED2, PROJECTOR};

class gFrameApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    void exit();
    void mouseMoved(int x, int y);
    void keyPressed(int key);
    
    // DMX
    void dmxUpdate();
    void setLEDColor(ofColor ledColor);
    void start_pulsing();
    void stop_pulsing();

    //OSC
    void oscUpdate();
    void oscupdate_interface();
    
    //settings
    void saveSettings();

private:
    
    // DRAWING
    StrokeList stroke_list;
    int current_style = STYLE_SCRIZZLE;
    ProfileStyle profileStyle;
    ScrizzleStyle scrizzleStyle;
    
    ofLight light;

    //output
    OutputMode outputmode = PROJECTOR;
    ofxSyphonServer syphonMainOut;
    //brazil support
    ofImage mPanels, mCanvas;
    ofRectangle mPanelPositionAndSize;
    ofRectangle mCanvasPositionAndSize;
    void toPanels(ofImage &canvas, ofImage &panels);
    void toPanelsGFrame(ofImage &canvas, ofImage &panels);
    ofImage fiespMask;
    ofImage brazilianOut;
    
    //drawing parameter
    ofParameterGroup localDrawingParameters;
    ofParameter<int> localPenWidth;
    ofParameter<ofColor> localPenColor;
    
    //OSC
    ofxOscReceiver receiver;
    ofxOscSender sender;
    float last_ipad_update_time = 0;
    
    //DMX
    ofxDmx dmx;
    ofColor LEDstripColor;
    float upper_pulsing_limit, lower_pulsing_limit;
    bool LED_pulsing;
    int LED_pulsing_time;
    float LED_level;
    float last_points_time;
    
    //setting
    ofxXmlSettings settings;
    
    //TUIO support
    ofxTuioClient   tuioClient;
    void	tuioAdded(ofxTuioCursor & tuioCursor);
	void	tuioRemoved(ofxTuioCursor & tuioCursor);
	void	tuioUpdated(ofxTuioCursor & tuioCursor);
    
    // NETWORK
    Network network;
};


