//
//  ScrizzleStyle.h
//  brushtest_02
//
//  Created by Robert Albert on 24.10.14.
//
//
#include <stdio.h>
#include "ofMain.h"
#include "BaseStyle.h"
#include "MSACore.h"
#include "MSAInterpolator.h"

class ScrizzleStyle : public BaseStyle {

private:
    //generall brush parameters
//    int new_point_distance = ofGetWidth()/50.0;
    //amplitude
    float amplitude1 = 4.0;
    float amplitude2 = 8.0;
    float amplitude3 = 12.0;
    //periodic speed
    float speed1 = 1.0;
    float speed2 = 0.9;
    float speed3 = 0.7;
    //length of the period
    float length1 = 5.0;
    float length2 = 4.0;
    float length3 = 6.0;
    //interpolator steps
    int isteps = 3;
    //color for each stroke
    ofColor color1 = ofColor(0, 0, 255);
    ofColor color2 = ofColor(12, 70, 232);
    ofColor color3 = ofColor(13, 140, 255);
    //linewidth for each stroke
    float linewidth1 = 4.0;
    float linewidth2 = 2.0;
    float linewidth3 = 2.0;
    float byLineScale = 0.5;
    
    //aging and fading out
    float startFadeAge = 500;
    float endFadeAge = 1000;

    msa::InterpolatorT<ofVec3f> interpolator;
    msa::InterpolationType interpolationType = msa::kInterpolationCubic;

public:
    double counter = 0;
    void render(vector<GPoint> &points);

    //setters
    void setAmplitude(float _ampl) {amplitude1 = 0.5 * _ampl; amplitude2 = _ampl; amplitude3 = 1.5 * _ampl;}
    void setLength(float _length) {length1 = 0.8 * _length; length2 = _length; length3 = 1.2 * _length;}
    void setMainLineThickness(float _linewidth, float _byLineScale) { linewidth1 = _linewidth; linewidth2 = _byLineScale * _linewidth; linewidth3 = _byLineScale * _linewidth;}
    void setFadeOutTime(float _fadeOuteTime) {return;};
    void setNervousity(float _speed) { speed1 = _speed; speed2 = 0.6 * _speed; speed3 = 0.4 * _speed; }
    
    
    void setInterpolatorSteps(int _iSteps) {isteps = _iSteps;}
    
    
    void setStartFadeAge(float _age) {startFadeAge = _age;}
    void setEndFadeAge( float _age) {endFadeAge = _age;}
    
    //these functions will be needed to adjust the line width and otehr parameters for the different screen resolutions
    
    
    
    
//    void setNewPointDistance(int _distance) {new_point_distance = _distance;}
    
    //getters
    float getAmplitude1() {return amplitude1;}
    float getAmplitude2() {return amplitude2;}
    float getAmplitude3() {return amplitude3;}
    
    float getSpeed1() {return speed1;}
    float getSpeed2() {return speed2;}
    float getSpeed3() {return speed3;}
    
    float getLength1() {return length1;}
    float getLength2() {return length2;}
    float getLength3() {return length3;}
    
    int getInterpolatorSteps() {return isteps;}
    
    ofColor getColor1() {return color1;}
    ofColor getColor2() {return color2;}
    ofColor getColor3() {return color3;}
    
    float getStartFadeAge() {return startFadeAge;}
    float getEndFadeAge() {return endFadeAge;}
    
    void setColor(ofColor _color);
};

