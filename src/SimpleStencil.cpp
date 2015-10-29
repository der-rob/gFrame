//
//  SimpleStencil.cpp
//  2DStencil
//
//  Created by Robert Albert on 08.09.15.
//
//

#include "SimpleStencil.h"

void SimpleStencil::setup(int _width, int _height, string _fontName, float _fadingTime, float _fadeOutAfterSeconds) {
    width = _width;
    height = _height;
    fontName = _fontName;
    fadeOutAfterSeconds = _fadeOutAfterSeconds;
    fadingTime = _fadingTime;
    
    font.loadFont(fontName, 45);

    stencilFBO.allocate(_width,_height, GL_RGBA, 4);
    canvasFBO.allocate(_width, _height, GL_RGBA, 4);
    composedFBO.allocate(_width, _height, GL_RGBA, 4);
    
    //fresh and clean fbos at startup
    stencilFBO.begin(); ofClear(0,0); stencilFBO.end();
    composedFBO.begin(); ofClear(0,0); composedFBO.end();
    canvasFBO.begin(); ofClear(0,0); canvasFBO.end();
    
    cutoutShader.load("shader/mask.vert","shader/mask.frag");
    fadeoutShader.load("shader/alpha.vert", "shader/alpha.frag");
    
    invert = false;
    old_invert = false;
    
    size = MEDIUM;
    
    text = "";
    bounds = font.getStringBoundingBox(text, 0,0);
}

//--------------------------------------------------------------
//update text will be called when the user entered a new text via the interface
//--------------------------------------------------------------
void SimpleStencil::updateText(string _text)
{
    oldText = text;
    text = _text;
    bounds = font.getStringBoundingBox(text, 0,0);
    cout << text << endl;
}

//--------------------------------------------------------------
// updateStencil creates the new stencil texture with the current text
// will be called when the new stencil is placed, that means after the first
// input with the stencil drawing device
//--------------------------------------------------------------
void SimpleStencil::updateStencil(int x, int y)
{
    switch (size) {
        case MEDIUM:
            font.loadFont(fontName, 30);
            bounds = font.getStringBoundingBox(text, 0,0);
            break;
        case LARGE:
            font.loadFont(fontName, 45);
            bounds = font.getStringBoundingBox(text, 0,0);
            break;
        case XTRALARGE:
            font.loadFont(fontName, 60);
            bounds = font.getStringBoundingBox(text, 0,0);
            break;
    }
    
    center.x = min(max(bounds.width/2,(float)x), width-bounds.width/2);
    center.y = min(max(bounds.height/2,(float)y), height-bounds.height/2);
    
    stencilFBO.begin();
    ofClear(0);
    ofSetColor(255);
    font.drawString(text, center.x-bounds.width/2, center.y+bounds.height/2);
    stencilFBO.end();
}

//--------------------------------------------------------------
void SimpleStencil::drawLastStencilPrints()
{
    //update alpha values of old stencil prints

    for (int i=0; i<oldStencilPrints.size();i++)
    {
        float age = ofGetElapsedTimef() - oldStencilPrints[i].timeStamp;
        if (age > fadeOutAfterSeconds+fadingTime) {
            oldStencilPrints.erase(oldStencilPrints.begin()+i);
            break;
        }
        oldStencilPrints[i].alpha = ofMap(age-fadeOutAfterSeconds,0.0, fadingTime, 1.0,0.0, true);
    }
    
    //Draw all the remaining old stencil prints
    for (StencilPrint the_print : oldStencilPrints) {
        fadeoutShader.begin();
        fadeoutShader.setUniform1f("alpha", the_print.alpha);
        the_print.image.draw(0,0);
        fadeoutShader.end();
    }
}

//--------------------------------------------------------------
void SimpleStencil::draw() {
    composedFBO.draw(0,0);
}

//--------------------------------------------------------------
void SimpleStencil::addNewPrint()
{
    ofPixels pixels;
    composedFBO.readToPixels(pixels);
    StencilPrint latestPrint;
    latestPrint.image.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
    latestPrint.image.setFromPixels(pixels);
    latestPrint.alpha = 1.0;
    latestPrint.timeStamp = ofGetElapsedTimef();
    latestPrint.text = oldText;
    
    oldStencilPrints.push_back(latestPrint);
    
    old_invert = invert;
    placeMode = false;
}

//--------------------------------------------------------------
void SimpleStencil::clearFBOs() {
    composedFBO.begin(); ofClear(0); composedFBO.end();
    canvasFBO.begin(); ofClear(0); canvasFBO.end();
}

//--------------------------------------------------------------
void SimpleStencil::startCanvas() {
    canvasFBO.begin();
    ofEnableAlphaBlending();
    ofClear(0);
}

//--------------------------------------------------------------
void SimpleStencil::finishCanvas()
{
    canvasFBO.end();
    
    composedFBO.begin();
    ofClear(0,0);
    cutoutShader.begin();
    cutoutShader.setUniformTexture("imageMask", stencilFBO.getTextureReference(), 1);
    cutoutShader.setUniform1i("invert", old_invert);
    canvasFBO.draw(0,0);
    cutoutShader.end();
    composedFBO.end();
}