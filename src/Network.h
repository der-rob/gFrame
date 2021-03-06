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
#include "ofThread.h"

class Network : public ofThread {
    
public: 
    Network();
    void setup(int local_server_port, string remote_server_ip, int remote_server_port);
    void disconnect();

    GPoint popPoint();
    void send(GPoint p);
    
    int getReceiveQueueLength();
    int getSendQueueLength();
    bool isConnected();
    int getNumClients();
    
    void threadedFunction();

private:
    void connectToRemoteHost();
    
    int local_server_port;
    string remote_server_ip;
    int remote_server_port;

    float last_connection_check;

protected:
    ofxTCPServer tcp_server;
    ofxTCPClient tcp_client;
    
    queue<GPoint> send_queue;
    queue<GPoint> receive_queue;
    
    bool connected;
};

#endif /* defined(__NetworkTest__Network__) */
