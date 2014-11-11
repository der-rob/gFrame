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

/// Start the thread.
void Network::start()
{
    // Mutex blocking is set to true by default
    // It is rare that one would want to use startThread(false).
    startThread();
}

/// Signal the thread to stop.  After calling this method,
/// isThreadRunning() will return false and the while loop will stop
/// next time it has the chance to.
void Network::stop()
{
    stopThread();
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
    
    // DON'T DO THIS HERE ANYMORE
    // connectToRemoteHost();
}

void Network::connectToRemoteHost(){
    // close the connection (if it was open)
    tcp_client.close();
    
    if(ofGetElapsedTimef() - last_connection_check > 5.0){
        // check if IP is available
        bool server_available;
        string pingStr = (string)"ping -c 1 -t 1 " + remote_server_ip;
        
        int flag = system( pingStr.c_str());
        
        if(flag == 0){
            server_available = true;
            ofLog() << "server is available";
        }else{
            server_available = false;
            //   cout<<ofGetTimestampString()<<endl;
            ofLog() << "could not connect to timing server at IP "<<remote_server_ip<<endl;
        }
        
        if(server_available){
            ofLog() << "trying to establish a connection to the remote server: " << ofToString(remote_server_ip) << ":" << ofToString(remote_server_port);
            connected = tcp_client.setup(remote_server_ip, remote_server_port);
            tcp_client.setMessageDelimiter("\n");
            
            if(connected){
                ofLog() << "client is connected to server " << tcp_client.getIP() << ":" << tcp_client.getPort();
            }
        }
        
        last_connection_check = ofGetElapsedTimef();
    }
}

void Network::threadedFunction()
{

    while(isThreadRunning())
    {
        //////////////////////
        // RECEIVING UPDATES
        //////////////////////
        // go through all the clients
        for(int client = 0; client < tcp_server.getLastID(); client++) {
            // if the client is not connected, skip it
            if( !tcp_server.isClientConnected(client) ) continue;
            
            string received = tcp_server.receive(client);
            
            // now read as long as we get something
            // the value 10 is just random
            while (received.size() > 10){
                GPoint p;
                p.unserialize(received);
                
                // Attempt to lock the mutex.  If blocking is turned on,
                if(lock()){
                    // put it on the queue
                    receive_queue.push(p);
                    unlock();
                }
                else { ofLogWarning("network.threadedFunction()") << "Unable to lock mutex."; }
                received = tcp_server.receive(client);
            }
        }
    
        int fails = 0;
        
        //////////////////////
        // SENDING UPDATES
        //////////////////////
        if (connected) {
            if(lock()){
                while(send_queue.size() > 0){
                    GPoint p = send_queue.front();
                    string s;
                    p.serialize(&s);
                    
                    bool send_succeeded;
                    
                    send_succeeded = tcp_client.send(s);
                    
                    if(send_succeeded){
                        send_queue.pop();
                    }
                    else {
                        fails++;
                        ofLog() << "sending did not succeed: " << fails;
                    }
                    
                    if (!tcp_client.isConnected() || fails > 5){
                        connected = false;
                        ofLog() << "connection seems to be broken";
                        break;
                    }
                }
                unlock();
            }
            else {
                // If we reach this else statement, it means that we could not
                // lock our mutex, and so we do not need to call unlock().
                // Calling unlock without locking will lead to problems.
                ofLogWarning("network.threadedFunction()") << "Unable to lock mutex.";
            }
        }
    
        if (!connected){
            // re-establish the connection
            connectToRemoteHost();
        }
    }
}

void Network::send(GPoint p){
    ofScopedLock lock(mutex);
    send_queue.push(p);
}

GPoint Network::popPoint(){
    ofScopedLock lock(mutex);
    GPoint p = receive_queue.front();
    receive_queue.pop();
    return p;
}

int Network::getReceiveQueueLength(){
    ofScopedLock lock(mutex);
    return receive_queue.size();
}

int Network::getSendQueueLength(){
    ofScopedLock lock(mutex);
    return send_queue.size();
}

void Network::disconnect(){
    stopThread();
    tcp_server.close();
    tcp_client.close();
    ofLog() << "network connections closed";
}