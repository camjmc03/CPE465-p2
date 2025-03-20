#include "of_message.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>

OFMessageHeader::OFMessageHeader()
    : version(OF_VERSION_1_0), type(0), length(8), xid(0) {
}

bool OFMessageHeader::receive(int socket) {
    uint8_t buffer[8];
    ssize_t bytes_read = read(socket, buffer, 8);
    
    if (bytes_read != 8) {
        return false;
    }
    
    version = buffer[0];
    type = buffer[1];
    length = ntohs(*(uint16_t*)(buffer + 2));
    xid = ntohl(*(uint32_t*)(buffer + 4));
    
    return true;
}

bool OFMessageHeader::send(int socket) const {
    uint8_t buffer[8];
    buffer[0] = version;
    buffer[1] = type;
    *(uint16_t*)(buffer + 2) = htons(length);
    *(uint32_t*)(buffer + 4) = htonl(xid);
    
    ssize_t bytes_sent = write(socket, buffer, 8);
    return bytes_sent == 8;
}

OFHelloMessage::OFHelloMessage() {
    type = OF_HELLO;
    length = 8;
}

bool OFHelloMessage::send(int socket) const {
    return OFMessageHeader::send(socket);
}

OFEchoRequestMessage::OFEchoRequestMessage() {
    type = OF_ECHO_REQUEST;
    length = 8;
}

bool OFEchoRequestMessage::send(int socket) const {
    return OFMessageHeader::send(socket);
}

OFEchoReplyMessage::OFEchoReplyMessage(uint32_t xid) {
    type = OF_ECHO_REPLY;
    length = 8;
    this->xid = xid;
}

bool OFEchoReplyMessage::send(int socket) const {
    return OFMessageHeader::send(socket);
}

OFFeaturesRequestMessage::OFFeaturesRequestMessage() {
    type = OF_FEATURES_REQUEST;
    length = 8;
}

bool OFFeaturesRequestMessage::send(int socket) const {
    return OFMessageHeader::send(socket);
}

OFFeaturesReplyMessage::OFFeaturesReplyMessage()
    : datapath_id(0), n_buffers(0), n_tables(0), capabilities(0), actions(0), port_count(0) {
    type = OF_FEATURES_REPLY;
}

bool OFFeaturesReplyMessage::receive(int socket, const OFMessageHeader& header) {
    if (header.type != OF_FEATURES_REPLY) {
        return false;
    }
    
    // Copy the header
    this->version = header.version;
    this->type = header.type;
    this->length = header.length;
    this->xid = header.xid;
    
    // Read the features reply data (24 bytes + ports)
    uint8_t buffer[24];
    ssize_t bytes_read = read(socket, buffer, 24);
    
    if (bytes_read != 24) {
        return false;
    }
    // datapath endianness is big-endian, convert to host endianness using ntohl
    datapath_id = ((uint64_t)ntohl(*(uint32_t*)(buffer)) << 32) | ntohl(*(uint32_t*)(buffer + 4));
    n_buffers = ntohl(*(uint32_t*)(buffer + 8));
    n_tables = buffer[12];
    // Padding 3 bytes
    capabilities = ntohl(*(uint32_t*)(buffer + 16));
    actions = ntohl(*(uint32_t*)(buffer + 20));
    
    // Calculate port count (each port is 48 bytes in OF 1.0)
    uint32_t remaining_bytes = header.length - 32; // 8 header + 24 read so far
    port_count = remaining_bytes / 48;
    
    // Skip the ports data for now - we'd parse this in a real implementation
    if (remaining_bytes > 0) {
        std::vector<uint8_t> port_data(remaining_bytes);
        bytes_read = read(socket, port_data.data(), remaining_bytes);
        if (bytes_read != (ssize_t)remaining_bytes) {
            return false;
        }
    }
    
    return true;
}

OFPortStatusMessage::OFPortStatusMessage()
    : reason(0), port_no(0), config(0), state(0) {
    type = OF_PORT_STATUS;
}

bool OFPortStatusMessage::receive(int socket, const OFMessageHeader& header) {
    if (header.type != OF_PORT_STATUS) {
        return false;
    }
    
    // Copy the header
    this->version = header.version;
    this->type = header.type;
    this->length = header.length;
    this->xid = header.xid;
    
    // Read the port status data (64 bytes)
    uint8_t buffer[64];
    ssize_t bytes_read = read(socket, buffer, 64);
    
    if (bytes_read != 64) {
        return false;
    }
    
    reason = buffer[0];
    // Padding 7 bytes
    port_no = ntohl(*(uint32_t*)(buffer + 8));
    
    // Extract MAC address (6 bytes)
    char mac[18]; // MAC address format: xx:xx:xx:xx:xx:xx (17 chars + null terminator)
    snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x", 
            buffer[16], buffer[17], buffer[18], buffer[19], buffer[20], buffer[21]);
    hw_addr = mac;
    
    // Extract port name (16 bytes)
    char port_name[17];
    memcpy(port_name, buffer + 24, 16);
    port_name[16] = '\0';
    name = port_name;
    
    config = ntohl(*(uint32_t*)(buffer + 40));
    state = ntohl(*(uint32_t*)(buffer + 44));
    
    return true;
}
