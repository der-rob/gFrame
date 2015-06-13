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
    
    //flowfield & gui
    simple_flow.setup(outputRect.width/2, outputRect.height/2);
    flowGuiSetup();
    flow_gui.loadFromFile("flow_default.xml");
    ofAddListener(flow_gui.loadPressedE, this, &gFrameApp::onFlowSettingsReload);
    ofAddListener(flow_gui.savePressedE, this, &gFrameApp::onFlowSettingsSave);
    
    
    simple_flow_2.setup(outputRect.width/2, outputRect.height/2);
    flow2GuiSetup();
    flow2_gui.loadFromFile("stroke.xml");
    ofAddListener(flow2_gui.loadPressedE, this, &gFrameApp::onFlow2SettingsReload);
    ofAddListener(flow2_gui.savePressedE, this, &gFrameApp::onFlow2SettingsSave);
    
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
    setLEDColor(ofColor::black);
    dmx.disconnect();
    network.disconnect();
}

//--------------------------------------------------------------
void gFrameApp::update()
{
    stroke_list.update();
    
    oscUpdate();
    
    tuioClient.getMessage();
    
    simple_flow.update();
    simple_flow_2.update();
    simple_flow_2.color = localBrushColor;
    
    // DMX UPDATE
    if (dmx_on)
        updateLEDpulsing();
    
    canvasFBO.begin();
    ofBackground(0);
    
//    flowField.setColor(ofColor::white);
//    ofBlendMode(OF_BLENDMODE_ALPHA);
//    flowField.render();
//    ofBlendMode(OF_BLENDMODE_ADD);
//    flowField2.render();
//
    simple_flow.draw();
    simple_flow_2.draw();
    
    for(vector<GPoint> stroke : *stroke_list.getAllStrokes()){
        switch (stroke[0].getStyle())
        {
            case STYLE_SCRIZZLE:
                scrizzleStyle.render(stroke, (int)outputRect.width, (int)outputRect.height);
                break;
            case STYLE_CALIGRAPHY:
                caligraphyStyle.render(stroke, (int)outputRect.width, (int)outputRect.height);
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
    
    //update the brush settings
    //scrizzle style
    scrizzleStyle.setMainLineThickness(W_mainLine_thickness, W_byLine_thicknes);
    scrizzleStyle.setAmplitude(W_amplitude);
    scrizzleStyle.setLength(W_wavelength);
    scrizzleStyle.setNervousity(W_nervosity);
    scrizzleStyle.setFadeOutTime(W_fadeout_time*1000.0, W_fadeduration*1000.0);
    scrizzleStyle.setNewPointDistance(W_new_pointdistance);
    
    //caligraphy style
    caligraphyStyle.setWidth(C_width_min, C_width_max);
    caligraphyStyle.setFadeOutTime(C_fadeout_time*1000.0, C_fadeduration*1000.0);
    caligraphyStyle.setNewPointDistance(C_new_pointdistance);

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
        flow_gui.draw();
        flow2_gui.draw();
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
        dmx_on = !dmx_on;
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
    
    if(input_mouse){
        ofVec2f mouse;
        
        mouse.set(x / (float)ofGetWindowWidth(), y / (float)ofGetWindowHeight());
        
        GPoint the_point;
        //rescale mouse position
        //float x_norm = ofMap(x, 0, ofGetWidth(), 0.0, outputRect.width);
        float x_norm = ofMap(x, 0, outputRect.width, 0.0, 1.0);
        //float y_norm = ofMap(y, 0, ofGetHeight(), 0.0, outputRect.height);
        float y_norm = ofMap(y, 0, outputRect.height, 0.0, 1.0);
        
        the_point.setLocation(ofVec2f(mouse.x,mouse.y));
        the_point.setId(0);
        the_point.setStrokeId(0);
        the_point.setColor(localBrushColor);
        the_point.setStyle(current_style);
        stroke_list.add(the_point);
        
        stop_pulsing();
        last_points_time = ofGetElapsedTimeMillis();
        
        //flowfield
        simple_flow.inputUpdate(x, y);
        simple_flow_2.inputUpdate(x, y);
    }
}

//--------------------------------------------------------------
void gFrameApp::mouseDragged(int x, int y, int button)
{
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

        the_point.setLocation(ofVec2f(x, y));
        the_point.setId(cursor.getFingerId());
        the_point.setStrokeId(cursor.getSessionId());
        the_point.setColor(localBrushColor);
        the_point.setType(TUIO);
        the_point.setStyle(current_style);
        stroke_list.addToNewStroke(the_point);
        
        finger_positions[cursor.getFingerId()] = ofVec2f(x, y);
        simple_flow.inputUpdate(x, y, cursor.getFingerId());
        simple_flow_2.inputUpdate(x, y, cursor.getFingerId());
        
        stop_pulsing();
        last_points_time = ofGetElapsedTimeMillis();
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
        
        the_point.setLocation(ofVec2f(x, y));
        the_point.setId(cursor.getFingerId());
        the_point.setStrokeId(cursor.getSessionId());
        the_point.setColor(localBrushColor);
        the_point.setType(TUIO);
        the_point.setStyle(current_style);
        stroke_list.add(the_point);
        
        finger_positions[cursor.getFingerId()] = ofVec2f(x, y);
        
        simple_flow.inputUpdate(x, y, cursor.getFingerId());
        simple_flow_2.inputUpdate(x, y, cursor.getFingerId());
        
        stop_pulsing();
        last_points_time = ofGetElapsedTimeMillis();
    }
}

//--------------------------------------------------------------
void gFrameApp::tuioRemoved(ofxTuioCursor & cursor)
{
    if(input_tuio)
    {
        finger_positions[cursor.getFingerId()] = ofVec2f(0, 0);
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
void gFrameApp::updateLEDpulsing(){
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
    float ledlevel2;
    if (LED_pulsing) {
        //int time = abs(((int)ofGetElapsedTimeMillis() % (LED_pulsing_time*2)) - LED_pulsing_time);
        LED_level = ofMap(time, 0, LED_pulsing_time, lower_pulsing_limit, upper_pulsing_limit);
    }
    
    setLEDColor(localBrushColor);
}

//--------------------------------------------------------------
void gFrameApp::start_pulsing() {
    //LED_level = 0.0;
    LED_pulsing =true;
}

//--------------------------------------------------------------
void gFrameApp::stop_pulsing() {
    LED_pulsing = false;
    LED_level = 1.0;
}

//--------------------------------------------------------------
void gFrameApp::setLEDColor(ofColor color){
    //send to color to the dmx device
    int r,g,b;
    float fr = 0,fg = 0,fb = 0;
    r = (int)color.r; fr = (float)r * LED_level*LED_brightness;
    g = (int)color.g; fg = (float)g * LED_level*LED_brightness;
    b = (int)color.b; fb = (float)b * LED_level*LED_brightness;
    //dmx channels are 2, 3 & 4
    dmx.setLevel(2, (int)fr);     //red
    dmx.setLevel(3, (int)fg);     //green
    dmx.setLevel(4, (int)fb);     //blue
    dmx.update();
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
    
    //DMX & LEDs
    dmx_settings.setName("dmx settings");
    dmx_settings.add(dmx_on.set("DMX on", false));
    dmx_settings.add(upper_pulsing_limit.set("upper PL",0,0,1));
    dmx_settings.add(lower_pulsing_limit.set("lower PL", 0,0,1));
    dmx_settings.add(LED_brightness.set("LED brightness",1.0,0.0,1));
    dmx_settings.add(LED_frequency.set("LED freqency",2000,500,5000));
    
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
    gui.add(dmx_settings);
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
    
    wild_parameters.setName("wild parameters");
    wild_parameters.add(W_new_pointdistance.set("new point distance",20,10,100));
    wild_parameters.add(W_amplitude.set("amplitude",8.0,0.0,20));
    wild_parameters.add(W_wavelength.set("wavelength", 4.0, 1.0, 10.0));
    wild_parameters.add(W_fadeout_time.set("fadeout time",10.0,2.0,60.0));
    wild_parameters.add(W_fadeduration.set("fade duration", 5.0, 0.0, 60));
    wild_parameters.add(W_nervosity.set("nervousity",1.0,0.5,20.0));
    wild_parameters.add(W_mainLine_thickness.set("main line thickness", 4.0, 1.0, 10.0));
    wild_parameters.add(W_byLine_thicknes.set("by line thickness", 0.5, 0.1, 5.0));
    
    caligraphy_parameters.setName("caligraphy parameters");
    caligraphy_parameters.add(C_new_pointdistance.set("new point distance",60,10,100));
    caligraphy_parameters.add(C_width_min.set("width min", 1, 0, 20));
    caligraphy_parameters.add(C_width_max.set("width max", 20, 1, 60));
    caligraphy_parameters.add(C_fadeout_time.set("fadeout time",10.0,2.0,60.0));
    caligraphy_parameters.add(C_fadeduration.set("fade duration", 5.0, 0.0, 60));
    
    style_gui.add(caligraphy_parameters);
    style_gui.add(wild_parameters);
    
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

//--------------------------------------------------------------
void gFrameApp::flowGuiSetup() {
    flow_gui.setup();
    flow_gui.setName("flow settings");
    flow_gui.setPosition(10, 10);
    flow_gui.add(simple_flow.fluid.parameters);
    flow_gui.minimizeAll();
}

//--------------------------------------------------------------
void gFrameApp::onFlowSettingsSave() {
    ofFileDialogResult save_result = ofSystemSaveDialog("NewFlowSettings.xml", "save new flow settings");
    string new_filename = save_result.getPath();
    cout << new_filename << endl;
    if (new_filename != "")
        flow_gui.saveToFile(new_filename);
}

//--------------------------------------------------------------
void gFrameApp::onFlowSettingsReload() {
    ofFileDialogResult load_result = ofSystemLoadDialog();
    string load_filename = load_result.getPath();
    if (load_filename != "")
        flow_gui.loadFromFile(load_filename);
}

//--------------------------------------------------------------
void gFrameApp::flow2GuiSetup() {
    flow2_gui.setup();
    flow2_gui.setName("flow settings");
    flow2_gui.setPosition(220, 10);
    flow2_gui.add(simple_flow_2.fluid.parameters);
    flow2_gui.minimizeAll();
}

//--------------------------------------------------------------
void gFrameApp::onFlow2SettingsSave() {
    ofFileDialogResult save_result = ofSystemSaveDialog("NewFlowSettings.xml", "save new flow settings");
    string new_filename = save_result.getPath();
    cout << new_filename << endl;
    if (new_filename != "")
        flow2_gui.saveToFile(new_filename);
}

//--------------------------------------------------------------
void gFrameApp::onFlow2SettingsReload() {
    ofFileDialogResult load_result = ofSystemLoadDialog();
    string load_filename = load_result.getPath();
    if (load_filename != "")
        flow2_gui.loadFromFile(load_filename);
}
