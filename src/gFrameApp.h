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
#include "GPoint.h"
#include "ProfileStyle.h"
#include "StrokeList.h"
#include "ScrizzleStyle.h"
#include "Network.h"
#include "ofxPQLabs.h"
#include "ofxGui.h"


#define STYLE_PROFILE 0
#define STYLE_SCRIZZLE 1

enum OutputMode {SESI, LED1, LED2, PROJECTOR, PROJECTOR_PORTRAIT};
enum Orientation {PORTRAIT, LANDSCAPE};

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

private:
    
    // DRAWING
    StrokeList stroke_list;
    int current_style = STYLE_SCRIZZLE;
    ofParameter<ofColor> localBrushColor;
    ofParameter<int> newPointDistance;
    
    ProfileStyle profileStyle;
    ofParameter<float> depthSpeed;
    ofParameter<float> TD_stroke_width;
    ofParameter<float> TD_fadeout_time;
    
    ScrizzleStyle scrizzleStyle;
    ofParameterGroup wild_parameters;
    ofParameter<float> W_amplitude;
    ofParameter<float> W_wavelength;
//    ofParameter<float> W_stroke_width;
    ofParameter<float> W_nervosity;
    ofParameter<float> W_mainLine_thickness;
    ofParameter<float> W_byLine_thicknes;
    ofParameter<float> W_fadeout_time;
    ofParameter<float> W_fadeduration;
    
    //setup method for the brushes
    void setupWildBrush();
    
    ofLight light;

    //output
    ofParameter<int> outputwidth = 1024;
    ofParameter<int> outputheight = 768;
    ofRectangle outputRect;
    OutputMode outputmode = PROJECTOR;
    Orientation orientation = LANDSCAPE;
    
    ofxSyphonServer syphonMainOut;
    //brazil support
    ofImage mPanels, mCanvas;
    ofRectangle mPanelPositionAndSize;
    ofRectangle mCanvasPositionAndSize;
    void toPanels(ofImage &canvas, ofImage &panels);
    void toPanelsGFrame(ofImage &canvas, ofImage &panels);
    ofImage fiespMask;
    ofImage brazilianOut;

    
    //OSC
    ofParameter<string> ipad_ip;
    ofParameter<int> ipad_port;
    ofParameter<int> local_osc_port;
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
        
    //TUIO support
    ofxTuioClient   tuioClient;
    ofxPQLabs pqlabsframe;
    void tuioAdded(ofxTuioCursor & tuioCursor);
	void tuioRemoved(ofxTuioCursor & tuioCursor);
	void tuioUpdated(ofxTuioCursor & tuioCursor);
    void onTouchPoint(TouchPointEvent &event);
    
    // NETWORK
    Network network;
    ofParameter<string> remote_ip;
    ofParameter<int> remote_port;
    ofParameter<int> host_port;

private:
    // GUI
    void guiSetup();
    ofxPanel gui;
    ofParameterGroup parameters;
    ofParameterGroup parameters_osc;
    ofParameterGroup parameters_network;
    ofParameterGroup parameters_output;
    
    ofParameter<string> gui_outputmode;
    ofParameter<string> gui_direction;
    
    ofParameterGroup parameters_brush;
    
    //eventhandlers for gui inputs
    void onSettingsReload();
};


