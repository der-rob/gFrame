//
//  Network.cpp
//  NetworkTest
//
//  Created by Julian Adenauer on 22.10.14.
//
//

#include "Network.h"


Network::Network(){
    connected = false;
    last_connection_check = -1000.0; // make sure that the connection is checked on startup
}

void Network::setup(int local_server_port, string remote_server_ip, int remote_server_port){
    this->remote_server_ip = remote_server_ip;
    this->remote_server_port = remote_server_port;
    this->local_server_port = local_server_port;
    
    // setup the local server
    if(tcp_server.setup(local_server_port))
        ofLog() << "server is set up on port " << ofToString(local_server_port);
    else "server setup failed";
    tcp_server.setMessageDelimiter("\n");
    
    connectToRemoteHost();
}

void Network::connectToRemoteHost(){
    if(ofGetElapsedTimef() - last_connection_check > 5.0){
        ofLog() << "trying to establish a connection to the remote server: " << ofToString(remote_server_ip) << ofToString(remote_server_port);
        connected = tcp_client.setup(remote_server_ip, remote_server_port);
        tcp_client.setMessageDelimiter("\n");
        
        if(connected){
            ofLog() << "client is connected to server " << tcp_client.getIP() << ":" << tcp_client.getPort();
        }
        
        last_connection_check = ofGetElapsedTimef();
    }
}

void Network::update(){

    //////////////////////
    // RECEIVING UPDATES
    //////////////////////
    // go through all the clients
    for(int client = 0; client < tcp_server.getLastID(); client++) {
        // if the client is not connected, skip it
		if( !tcp_server.isClientConnected(client) ) continue;
        
        string received = tcp_server.receive(client);
        
        if(received.size() > 10){
            GPoint p;
            p.unserialize(received);
            
            // put it on the queue
            receive_queue.push(p);
            
            // TODO: MORE CHECKS!!!!!
        }
        
    }
    
    //////////////////////
    // SENDING UPDATES
    //////////////////////
    
    if (connected) {
        while(send_queue.size() > 0){
            GPoint p = send_queue.front();
            string s;
            p.serialize(&s);
            
            bool send_succeeded = tcp_client.send(s);
            
            if(send_succeeded){
                send_queue.pop();
            }
            else {
                ofLog() << "sending did not succeed";
            }
        
            if (!tcp_client.isConnected()){
                connected = false;
            }
            
            // TODO: check num bytes sent
        }
    }
    else{
        // re-establish the connection
        connectToRemoteHost();
    }
}

void Network::send(GPoint p){
    send_queue.push(p);
}

GPoint Network::popPoint(){
    GPoint p = receive_queue.front();
    receive_queue.pop();
    return p;
}

int Network::getReceiveQueueLength(){
    return receive_queue.size();
}

int Network::getSendQueueLength(){
    return send_queue.size();
}