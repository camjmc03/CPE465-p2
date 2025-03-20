#pragma once

#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <atomic>
#include "of_switch.h"

class OpenFlowController {
public:
    OpenFlowController(uint16_t port = 6653);
    ~OpenFlowController();
    
    // Start the controller
    bool start();
    
    // Stop the controller
    void stop();
    
    // Get connected switch count
    size_t getSwitchCount() const;

private:
    // Main listener thread
    void listenerThread();
    
    // Handle a new connection
    void handleConnection(int client_socket);
    
    // Process messages from a switch
    void processMessages(int client_socket, uint64_t datapath_id);
    
    uint16_t listen_port_;
    int listen_socket_;
    std::atomic<bool> running_;
    
    // Thread for listener
    std::thread listener_thread_;
    
    // Map of switches by datapath ID
    std::map<uint64_t, OpenFlowSwitch> switches_;
    mutable std::mutex switches_mutex_;
};
