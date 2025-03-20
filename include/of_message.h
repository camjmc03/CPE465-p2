#pragma once

#include <cstdint>
#include <string>

// OpenFlow message types
#define OF_HELLO           0
#define OF_ERROR           1
#define OF_ECHO_REQUEST    2
#define OF_ECHO_REPLY      3
#define OF_FEATURES_REQUEST 5
#define OF_FEATURES_REPLY  6
#define OF_PACKET_IN       10
#define OF_PORT_STATUS     12

// OpenFlow version
#define OF_VERSION_1_0     0x01

// OpenFlow header structure
struct OFMessageHeader {
    uint8_t version;
    uint8_t type;
    uint16_t length;
    uint32_t xid;
    
    OFMessageHeader();
    bool receive(int socket);
    bool send(int socket) const;
};

// HELLO message
struct OFHelloMessage : public OFMessageHeader {
    OFHelloMessage();
    bool send(int socket) const;
};

// ECHO_REQUEST message
struct OFEchoRequestMessage : public OFMessageHeader {
    OFEchoRequestMessage();
    bool send(int socket) const;
};

// ECHO_REPLY message
struct OFEchoReplyMessage : public OFMessageHeader {
    OFEchoReplyMessage(uint32_t xid);
    bool send(int socket) const;
};

// FEATURES_REQUEST message
struct OFFeaturesRequestMessage : public OFMessageHeader {
    OFFeaturesRequestMessage();
    bool send(int socket) const;
};

// FEATURES_REPLY message
struct OFFeaturesReplyMessage : public OFMessageHeader {
    uint64_t datapath_id;
    uint32_t n_buffers;
    uint8_t n_tables;
    uint32_t capabilities;
    uint32_t actions;
    uint32_t port_count;
    
    OFFeaturesReplyMessage();
    bool receive(int socket, const OFMessageHeader& header);
};

// PORT_STATUS message
struct OFPortStatusMessage : public OFMessageHeader {
    uint8_t reason;
    uint32_t port_no;
    std::string hw_addr;
    std::string name;
    uint32_t config;
    uint32_t state;
    
    OFPortStatusMessage();
    bool receive(int socket, const OFMessageHeader& header);
};
