//
//  ScrizzleStyle.h
//  brushtest_02
//
//  Created by Robert Albert on 24.10.14.
//
//
#include <stdio.h>
#include "BaseStyle.h"
#include "MSACore.h"
#include "MSAInterpolator.h"

class ScrizzleStyle : public BaseStyle {

private:
    //generall brush parameters
    int new_point_distance = 20;
    //amplitude
    float amplitude1 = 2.0;
    float amplitude2 = 4.0;
    float amplitude3 = 6.0;
    //periodic speed
    float speed1 = 1.0;
    float speed2 = 0.9;
    float speed3 = 0.7;
    //length of the period
    float length1 = 5.0;
    float length2 = 4.0;
    float length3 = 6.0;
    //interpolator steps
    int isteps1 = 3;
    int isteps2 = 3;
    int isteps3 = 3;
    //color for each stroke
    ofColor color1 = ofColor(41, 78, 127);
    ofColor color2 = ofColor(62, 117, 191);
    ofColor color3 = ofColor(82, 157, 255);
    //aging and fading out
    float startFadeAge = 500;
    float endFadeAge = 1000;

    msa::InterpolatorT<ofVec3f> interpolator;
    msa::InterpolationType interpolationType = msa::kInterpolationCubic;

public:
    double counter = 0;
    ScrizzleStyle();
    void render(vector<GPoint> &points);

    //setters
    void setAmplitude1(float _ampl) {amplitude1 = _ampl;}
    void setAmplitude2(float _ampl) {amplitude2 = _ampl;}
    void setAmplitude3(float _ampl) {amplitude3 = _ampl;}
    
    void setSpeed1(float _speed) {speed1 = _speed;}
    void setSpeed2(float _speed) {speed2 = _speed;}
    void setSpeed3(float _speed) {speed3 = _speed;}
    
    void setLegth1(float _length) {length1 = _length;}
    void setLegth2(float _length) {length2 = _length;}
    void setLegth3(float _length) {length3 = _length;}
    
    void setInterpolatorSteps(int _iSteps) {isteps1 = _iSteps; isteps2 = _iSteps; isteps3 = _iSteps;}
    
    void setColor1(ofColor _color) {color1 = _color;}
    void setColor2(ofColor _color) {color2 = _color;}
    void setColor3(ofColor _color) {color3 = _color;}
    
    void setStartFadeAge(float _age) {startFadeAge = _age;}
    void setEndFadeAge( float _age) {endFadeAge = _age;}
    
    //getters
    
};

