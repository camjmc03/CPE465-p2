#include "of_controller.h"
#include "of_message.h"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

OpenFlowController::OpenFlowController(uint16_t port)
    : listen_port_(port), listen_socket_(-1), running_(false) {
}

OpenFlowController::~OpenFlowController() {
    stop();
}

bool OpenFlowController::start() {
    if (running_) {
        std::cout << "Controller is already running" << std::endl;
        return false;
    }
    
    // Create socket
    listen_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket_ < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return false;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set socket options: " << strerror(errno) << std::endl;
        close(listen_socket_);
        return false;
    }
    
    // Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(listen_port_);
    
    if (bind(listen_socket_, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        close(listen_socket_);
        return false;
    }
    
    // Listen for connections
    if (listen(listen_socket_, 3) < 0) {
        std::cerr << "Failed to listen: " << strerror(errno) << std::endl;
        close(listen_socket_);
        return false;
    }
    
    running_ = true;
    listener_thread_ = std::thread(&OpenFlowController::listenerThread, this);
    
    std::cout << "OpenFlow controller started on port " << listen_port_ << std::endl;
    return true;
}

void OpenFlowController::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    // Close the listening socket to unblock accept()
    if (listen_socket_ >= 0) {
        close(listen_socket_);
        listen_socket_ = -1;
    }
    
    // Wait for listener thread to finish
    if (listener_thread_.joinable()) {
        listener_thread_.join();
    }
    
    // Close connections to all switches
    std::lock_guard<std::mutex> lock(switches_mutex_);
    switches_.clear();
    
    std::cout << "OpenFlow controller stopped" << std::endl;
}

size_t OpenFlowController::getSwitchCount() const {
    std::lock_guard<std::mutex> lock(switches_mutex_);
    return switches_.size();
}

void OpenFlowController::listenerThread() {
    while (running_) {
        struct sockaddr_in address;
        socklen_t addrlen = sizeof(address);
        
        int client_socket = accept(listen_socket_, (struct sockaddr *)&address, &addrlen);
        if (client_socket < 0) {
            if (running_) {
                std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            }
            continue;
        }
        
        std::cout << "New connection from " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << std::endl;
        
        // Handle the connection in a new thread
        std::thread([this, client_socket]() {
            this->handleConnection(client_socket);
        }).detach();
    }
}

void OpenFlowController::handleConnection(int client_socket) {
    // Send HELLO message
    OFHelloMessage hello;
    if (!hello.send(client_socket)) {
        std::cerr << "Failed to send HELLO message" << std::endl;
        close(client_socket);
        return;
    }
    
    // Receive HELLO message
    OFMessageHeader header;
    if (!header.receive(client_socket)) {
        std::cerr << "Failed to receive message header" << std::endl;
        close(client_socket);
        return;
    }
    
    if (header.type != OF_HELLO) {
        std::cerr << "Expected HELLO message, got type " << (int)header.type << std::endl;
        close(client_socket);
        return;
    }
    
    // Send FEATURES_REQUEST
    OFFeaturesRequestMessage featuresRequest;
    if (!featuresRequest.send(client_socket)) {
        std::cerr << "Failed to send FEATURES_REQUEST message" << std::endl;
        close(client_socket);
        return;
    }
    
    // Receive FEATURES_REPLY
    OFMessageHeader featuresHeader;
    if (!featuresHeader.receive(client_socket)) {
        std::cerr << "Failed to receive FEATURES_REPLY header" << std::endl;
        close(client_socket);
        return;
    }
    
    if (featuresHeader.type != OF_FEATURES_REPLY) {
        std::cerr << "Expected FEATURES_REPLY message, got type " << (int)featuresHeader.type << std::endl;
        close(client_socket);
        return;
    }
    
    OFFeaturesReplyMessage featuresReply;
    if (!featuresReply.receive(client_socket, featuresHeader)) {
        std::cerr << "Failed to receive FEATURES_REPLY message" << std::endl;
        close(client_socket);
        return;
    }
    
    uint64_t datapath_id = featuresReply.datapath_id;
    
    // Create and store a new switch object
    {
        std::lock_guard<std::mutex> lock(switches_mutex_);
        OpenFlowSwitch& sw = switches_[datapath_id];
        sw.setDatapathId(datapath_id);
        sw.setVersion(header.version);
        sw.setPortCount(featuresReply.port_count);
        
        std::cout << "Switch connected:" << std::endl;
        std::cout << "  Datapath ID: 0x" << std::hex << datapath_id << std::dec << std::endl;
        std::cout << "  OpenFlow version: 0x" << std::hex << (int)header.version << std::dec << std::endl;
        std::cout << "  Port count: " << featuresReply.port_count << std::endl;
    }
    
    // Process messages from this switch
    processMessages(client_socket, datapath_id);
    
    // Remove the switch when the connection is closed
    {
        std::lock_guard<std::mutex> lock(switches_mutex_);
        switches_.erase(datapath_id);
        std::cout << "Switch 0x" << std::hex << datapath_id << std::dec << " disconnected" << std::endl;
    }
    
    close(client_socket);
}

void OpenFlowController::processMessages(int client_socket, uint64_t datapath_id) {
    while (running_) {
        OFMessageHeader header;
        if (!header.receive(client_socket)) {
            break;
        }
        
        switch (header.type) {
            case OF_ECHO_REQUEST: {
                // Reply to echo requests
                OFEchoReplyMessage reply(header.xid);
                reply.send(client_socket);
                break;
            }
            
            case OF_PACKET_IN: {
                std::cout << "PACKET_IN event received from switch 0x" << std::hex << datapath_id << std::dec << std::endl;
                // Process PACKET_IN message (for Milestone 1, we just report it)
                break;
            }
            
            case OF_PORT_STATUS: {
                OFPortStatusMessage portStatus;
                if (portStatus.receive(client_socket, header)) {
                    std::cout << "Link state change detected on switch 0x" << std::hex << datapath_id << std::dec << std::endl;
                    std::cout << "  Port: " << portStatus.port_no << std::endl;
                    std::cout << "  Reason: " << (portStatus.reason == 0 ? "ADD" : 
                                               (portStatus.reason == 1 ? "DELETE" : "MODIFY")) << std::endl;
                }
                break;
            }
            
            default:
                std::cout << "Received message of type " << (int)header.type << " from switch 0x"
                          << std::hex << datapath_id << std::dec << std::endl;
                break;
        }
    }
}
