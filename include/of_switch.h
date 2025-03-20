#pragma once

#include <cstdint>
#include <vector>
#include <string>

struct PortInfo {
    uint32_t port_no;
    std::string hw_addr;
    std::string name;
    uint32_t config;
    uint32_t state;
};

class OpenFlowSwitch {
public:
    OpenFlowSwitch();
    
    // Set switch properties
    void setDatapathId(uint64_t id);
    void setVersion(uint8_t version);
    void setPortCount(uint32_t count);
    void addPort(const PortInfo& port);
    
    // Get switch properties
    uint64_t getDatapathId() const;
    uint8_t getVersion() const;
    uint32_t getPortCount() const;
    const std::vector<PortInfo>& getPorts() const;

private:
    uint64_t datapath_id_;
    uint8_t version_;
    std::vector<PortInfo> ports_;
};
