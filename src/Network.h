//
//  Network.h
//  NetworkTest
//
//  Created by Julian Adenauer on 22.10.14.
//
//

#ifndef __NetworkTest__Network__
#define __NetworkTest__Network__

#include <iostream>
#include "ofMain.h"
#include "ofxNetwork.h"
#include "GPoint.h"

class Network{
    
public: 
    Network();
    void setup(int local_server_port, string remote_server_ip, int remote_server_port);
    void update();

    GPoint popPoint();
    void send(GPoint p);
    
    int getReceiveQueueLength();
    int getSendQueueLength();
    

private:
    
    ofxTCPServer tcp_server;
    ofxTCPClient tcp_client;
    
    void connectToRemoteHost();
    
    queue<GPoint> send_queue;
    queue<GPoint> receive_queue;
    
    int local_server_port;
    string remote_server_ip;
    int remote_server_port;
    
    bool connected;
    float last_connection_check;

};

#endif /* defined(__NetworkTest__Network__) */
