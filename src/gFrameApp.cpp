#pragma once
#include "gFrameApp.h"

#define USE_PROGRAMMABLE_GL     // Maybe there is a reason you would want to


//--------------------------------------------------------------
void gFrameApp::setup()
{
    //just set up the openFrameworks stuff
    ofSetFrameRate(30);
    ofSetVerticalSync(true);
    ofBackground(ofColor::black);
    
    //LEDFrame setup
    ledFrame.setup();
    
    //GUI setup
    guiSetup();
    styleGuiSetup();
    
    ofAddListener(gui.loadPressedE, this, &gFrameApp::onSettingsReload);
    ofAddListener(gui.savePressedE, this, &gFrameApp::onSettingsSave);
    ofAddListener(style_gui.loadPressedE, this, &gFrameApp::onStyleSettingsreload);
    ofAddListener(style_gui.savePressedE, this, &gFrameApp::onStyleSettingsSave);
    
    gui.loadFromFile("settings.xml");
    style_gui.loadFromFile("stylesettings.xml");
    
    //dimensions for final output
    //after settings are loaded from file
    ofSetWindowShape(outputwidth, outputheight);
    outputRect = ofRectangle(0,0,outputwidth, outputheight);
    
    //need to call this here, otherwise would get a BAD ACCESS FAULT
    gui.draw();
    style_gui.draw();
    
    //Syphon stuff
    syphonMainOut.setName("gFrame Main Out");
    
    //TUIO setup
    tuioClient.start(tuioPort);
    ofAddListener(tuioClient.cursorAdded,this,&gFrameApp::tuioAdded);
	ofAddListener(tuioClient.cursorUpdated,this,&gFrameApp::tuioUpdated);
    ofAddListener(tuioClient.cursorRemoved,this,&gFrameApp::tuioRemoved);
    
    //mouse
    current_mouse_id = 0;
    
    //OSC
    receiver.setup(local_osc_port);

#ifdef USE_NETWORK
    // NETWORK
    network.setup(host_port, remote_ip, remote_port);
    stroke_list.setupSync(&network);
#endif

    //syphonFBO.allocate(outputRect.width, outputRect.height, GL_RGBA, 16);
    canvasFBO.allocate(outputRect.width, outputRect.height, GL_RGBA, 16);
    //syphonFBO.begin(); ofClear(0); syphonFBO.end();
    canvasFBO.begin(); ofClear(0); canvasFBO.end();

    // initialize finger positions
    for(ofVec2f finger : finger_positions){
        finger = ofVec2f(0,0);
    }
}

//--------------------------------------------------------------
void gFrameApp::exit(){
    ledFrame.disconnect();
    network.disconnect();
}

//--------------------------------------------------------------
void gFrameApp::update()
{
    stroke_list.update();
    
    oscUpdate();
    
    tuioClient.getMessage();
    
    // DMX UPDATE
    if (ledFrame.getEnabled()) {
        ledFrame.updateLevel();
        ledFrame.setColor(localBrushColor);
        ledFrame.update();
    }
    float dt = ofGetLastFrameTime();
    
    //update brush scale animation
    for (int i = 0; i < 13; i++) {
        if (brushSizeScale[i].getAnimation() != NULL) {
            brushSizeScale[i].updateAnimation(dt);
        }
    }
    
    canvasFBO.begin();
    ofBackground(0);
    
    for(vector<GPoint> stroke : *stroke_list.getAllStrokes()){
        switch (stroke[0].getStyle())
        {
            case STYLE_SCRIZZLE:
                scrizzleStyle.render(stroke, (int)outputRect.width, (int)outputRect.height);
                break;
            case STYLE_CALIGRAPHY:
                caligraphyStyle.render(stroke, (int)outputRect.width, (int)outputRect.height);
                break;
            case STYLE_IMAGE:
                imageBrush.render(stroke, (int)outputRect.width, (int)outputRect.height);
                break;
            default:
                caligraphyStyle.render(stroke, (int)outputRect.width, (int)outputRect.height);
                break;
        }
    }
    
    if(draw_finger_positions){
        drawFingerPositions((int)outputRect.width, (int)outputRect.height);
    }
    
    canvasFBO.end();
    
    // lifetime
    stroke_list.setLifetime(point_lifetime * 1000);
}

//--------------------------------------------------------------
void gFrameApp::draw(){
    
    ofBackground(0);
    ofSetColor(255);

    syphonMainOut.publishTexture(&canvasFBO.getTextureReference());
    
    //switching of the main screen might improve the performance
    if (draw_on_main_screen)
    {
        canvasFBO.draw(0,0,outputRect.width, outputRect.height);
    }

    //gui output here
    if(draw_gui){
        gui.draw();
        style_gui.draw();
        ofSetColor(255);
        ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate(), 2), ofGetWidth()-120, ofGetHeight()-10 );
    }
}

//--------------------------------------------------------------
void gFrameApp::drawFingerPositions(int _width, int _height)
{
    ofPushStyle();
    ofDisableDepthTest(); // disable depth test so the alpha blending works properly
    ofDisableLighting();
    
    ofColor outerColor = localBrushColor;
    outerColor.set(localBrushColor.get().r, localBrushColor.get().g, localBrushColor.get().b, 30);
    
    ofSetColor(localBrushColor);
    int i=0;
    for(ofVec2f finger : finger_positions){
        int px = finger.x * _width;
        int py = finger.y * _height;
        if(!(px == 0 && py == 0)){
            float incr = (float) ((2 * PI) / 32);
            
            glBegin(GL_TRIANGLE_FAN);
            ofSetColor(localBrushColor);
            glVertex2f(px, py);
            
            ofSetColor(outerColor);
            
            for(int i = 0; i < 32; i++){
                float angle = incr * i;
                float x = ((float) cos(angle) * finger_position_size) + px;
                float y = ((float) sin(angle) * finger_position_size) + py;
                glVertex2f(x, y);
            }
            
            glVertex2f(finger_position_size + px, py);
            glEnd();
#if DEBUG
            // draw finger id for debugging
            ofSetColor(255, 255, 255, 255);
            ofDrawBitmapString(ofToString(i), finger.x-5, finger.y+5);
#endif
        }
        i++;
    }
    ofPopStyle();
}

//--------------------------------------------------------------
void gFrameApp::keyPressed(int key)
{
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
        imageBrush.clear();
    }
    else if (key == 'p')
        draw_finger_positions = !draw_finger_positions;
    else if (key == 's')
        current_style = STYLE_SCRIZZLE;
    else if (key == 'k')
        current_style = STYLE_CALIGRAPHY;
    else if(key == 'h')
        draw_gui = !draw_gui;
    else if (key == 'd')
        ledFrame.toggleEnabled();
    else if(key == 'y')
        draw_on_main_screen = !draw_on_main_screen;
    else if (key == 'x') {
        fullscreen = !fullscreen;
        ofSetFullscreen(fullscreen);
        style_gui.setPosition(ofGetWidth() - 2*style_gui.getWidth() - 20, 10);
        gui.setPosition(ofGetWidth() - gui.getWidth() - 10, 10);
        outputRect.width = ofGetWidth();
        outputRect.height = ofGetHeight();
        canvasFBO.allocate(outputRect.width, outputRect.height, GL_RGBA, 16);
        //syphonFBO.allocate(outputRect.width, outputRect.height, GL_RGBA, 16);
        cout << outputRect.width << " x " << outputRect.height << endl;
    }
    
    else if (key == 'm') {
        draw_on_main_screen = !draw_on_main_screen;
    }
}

//--------------------------------------------------------------
void gFrameApp::mouseMoved(int x, int y){
}

//--------------------------------------------------------------
void gFrameApp::mousePressed(int x, int y, int button)
{
    if(input_mouse && button == OF_MOUSE_BUTTON_1){
        
        //rescale mouse position
        ofVec2f mouse;
        mouse.set(x / (float)ofGetWindowWidth(), y / (float)ofGetWindowHeight());
        
        //size scale animation
        brushSizeScale[12].createAnimation(0, 1, 4, ZERO);
        
        GPoint the_point;
        the_point.setSizeScale(brushSizeScale[12].getValue());
        the_point.setLocation(ofVec2f(mouse.x,mouse.y));
        the_point.setId(0);
        the_point.setStrokeId(current_mouse_id);
        the_point.setColor(localBrushColor);
        the_point.setStyle(current_style);
        stroke_list.addToNewStroke(the_point);
        
        ledFrame.stopPulsing();
        ledFrame.updateLastPointsTime();
    }
}

//--------------------------------------------------------------
void gFrameApp::mouseDragged(int x, int y, int button)
{
    if(input_mouse && button == OF_MOUSE_BUTTON_1) {
        
        //rescale mouse position
        ofVec2f mouse;
        mouse.set(x / (float)ofGetWindowWidth(), y / (float)ofGetWindowHeight());
        
        GPoint the_point;
        the_point.setSizeScale(brushSizeScale[12].getValue());
        the_point.setLocation(ofVec2f(mouse.x,mouse.y));
        the_point.setId(0);
        the_point.setStrokeId(current_mouse_id);
        the_point.setColor(localBrushColor);
        the_point.setStyle(current_style);
        stroke_list.add(the_point);
        
        ledFrame.stopPulsing();
        ledFrame.updateLastPointsTime();
    }
}

//--------------------------------------------------------------
void gFrameApp::mouseReleased(int x, int y, int button)
{
    
    if (button == OF_MOUSE_BUTTON_1) {
        brushSizeScale[12].stopAnimation();
        current_mouse_id++;
    }
}


//--------------------------------------------------------------
void gFrameApp::windowResized(int w, int h)
{
    outputRect.width = w;
    outputRect.height = h;
}

//--------------------------------------------------------------
void gFrameApp::tuioAdded(ofxTuioCursor &cursor)
{
    if(input_tuio){
        GPoint the_point;
        float x,y;
        x = cursor.getX();
        y = cursor.getY();

        //size scale animation
        brushSizeScale[cursor.getFingerId()].createAnimation(0, 1, 4, ZERO);
        the_point.setSizeScale(brushSizeScale[cursor.getFingerId()].getValue());
        
        
        the_point.setLocation(ofVec2f(x, y));
        the_point.setId(cursor.getFingerId());
        the_point.setStrokeId(cursor.getSessionId());
        the_point.setColor(localBrushColor);
        the_point.setType(TUIO);
        the_point.setStyle(current_style);
        stroke_list.addToNewStroke(the_point);
        
        finger_positions[cursor.getFingerId()] = ofVec2f(x, y);
    
        ledFrame.stopPulsing();
        ledFrame.updateLastPointsTime();
    }
}

//--------------------------------------------------------------
void gFrameApp::tuioUpdated(ofxTuioCursor &cursor)
{
    if(input_tuio)
    {
        GPoint the_point;
        float x,y;
        x = cursor.getX();
        y = cursor.getY();
        
        the_point.setSizeScale(brushSizeScale[cursor.getFingerId()].getValue());
        the_point.setLocation(ofVec2f(x, y));
        the_point.setId(cursor.getFingerId());
        the_point.setStrokeId(cursor.getSessionId());
        the_point.setColor(localBrushColor);
        the_point.setType(TUIO);
        the_point.setStyle(current_style);
        stroke_list.add(the_point);
        
        finger_positions[cursor.getFingerId()] = ofVec2f(x, y);
        
        ledFrame.stopPulsing();
        ledFrame.updateLastPointsTime();
    }
}

//--------------------------------------------------------------
void gFrameApp::tuioRemoved(ofxTuioCursor & cursor)
{
    if(input_tuio)
    {
        finger_positions[cursor.getFingerId()] = ofVec2f(0, 0);
        brushSizeScale[cursor.getFingerId()].stopAnimation();
    }
}

//--------------------------------------------------------------
void gFrameApp::oscUpdate()
{
    while (receiver.hasWaitingMessages())
    {
        ofxOscMessage m;
        receiver.getNextMessage(&m);
        cout << m.getAddress() << " " << m.getArgAsString(0) << endl;
        
        //style color
        if (m.getAddress() == "/col_red") localBrushColor = ofColor::red;
        else if (m.getAddress() == "/col_green") localBrushColor = ofColor::green;
        else if (m.getAddress() == "/col_blue") localBrushColor = ofColor::blue;
        else if (m.getAddress() == "/col_yellow") localBrushColor = ofColor::yellow;
        else if (m.getAddress() == "/col_orange") localBrushColor = ofColor::orange;
        else if (m.getAddress() == "/col_pink") localBrushColor = ofColor::pink;
        else if (m.getAddress() == "/clearcanvas") stroke_list.clear();
        //brush style
        else if (m.getAddress() == "/style_wild") current_style = STYLE_SCRIZZLE;
        else if (m.getAddress() == "/style_cali") current_style = STYLE_CALIGRAPHY;
    }
}

//--------------------------------------------------------------
void gFrameApp::guiSetup() {
    //GUI Setup
    gui.setup();
    gui.setPosition(ofGetWidth() - gui.getWidth() - 10, 10);
    gui.setName("general settings");
    
    ///output settings
    parameters_output.setName("output settings");
    outputwidth.setName("width");
    parameters_output.add(outputwidth);
    outputheight.setName("height");
    parameters_output.add(outputheight);
    
    ///OSC
    parameters_osc.setName("osc");
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

    ///general brush settings
    parameters_brush.setName("brush settings");
    parameters_brush.add(point_lifetime.set("point lifetime", 10, 1, 100));
    localBrushColor.setName("color");
    parameters_brush.add(localBrushColor.set("local color", ofColor(255, 255, 255), ofColor(0,0,0), ofColor(255,255,255)));
    
    // finger positions
    parameters_finger.setName("finger positions");
    parameters_finger.add(draw_finger_positions.set("draw finger positions", true));
    parameters_finger.add(finger_position_size.set("finger circle radius", 30, 2, 100));
    
    // input settings
    parameters_input.setName("input selection");
    parameters_input.add(tuioPort.set("TUIO port", 3333));
    parameters_input.add(input_mouse.set("mouse", false));
    parameters_input.add(input_tuio.set("tuio", true));
    
    //add the subgroups to the gui
    gui.add(parameters_finger);
    gui.add(parameters_brush);
    gui.add(parameters_output);
    gui.add(ledFrame.parameters);
    gui.add(parameters_network);
    gui.add(parameters_osc);
    gui.add(parameters_input);
    
    //minimize some of the gui elements
    vector<string> controlNames = gui.getControlNames();
    for (int i=0; i < controlNames.size(); i++) {
        string the_name = controlNames[i];
        if (the_name == "output settings" ||
            the_name == "dmx settings" ||
            the_name == "network" ||
            the_name == "osc" ||
            the_name == "input selection")
        {
            ofxGuiGroup *the_group = &gui.getGroup(the_name);
            the_group->minimize();
        }
    }
}

//--------------------------------------------------------------
void gFrameApp::styleGuiSetup() {
    //more specific style settings
    style_gui.setup();
    style_gui.setName("style settings");
    style_gui.setPosition(ofGetWidth() - 2*style_gui.getWidth() - 20, 10);
    
    style_gui.add(caligraphyStyle.parameters);
    style_gui.add(scrizzleStyle.parameters);
    style_gui.add(imageBrush.parameters);
    
    style_gui.minimizeAll();
}

//--------------------------------------------------------------
void gFrameApp::onSettingsReload() {
    gui.loadFromFile("settings.xml");
    //network
    network.disconnect();
    network.setup(host_port, remote_ip, remote_port);
    //osc
    receiver.setup(local_osc_port);
    //tuio
    tuioClient.start(tuioPort);
    //clean stroklist
    stroke_list.clear();
    
    cout << "New settings loaded" << endl;
}

//--------------------------------------------------------------
void gFrameApp::onSettingsSave() {
    gui.saveToFile("settings.xml");
}

//--------------------------------------------------------------
void gFrameApp::onStyleSettingsreload() {
    style_gui.loadFromFile("stylesettings.xml");
    ofLog() << "style settings reloaded";
}

//--------------------------------------------------------------
void gFrameApp::onStyleSettingsSave() {
    style_gui.saveToFile("stylesettings.xml");
    ofLog() << "style settings saved";
}