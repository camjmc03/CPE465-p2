#include "of_switch.h"

OpenFlowSwitch::OpenFlowSwitch()
    : datapath_id_(0), version_(0) {
}

void OpenFlowSwitch::setDatapathId(uint64_t id) {
    datapath_id_ = id;
}

void OpenFlowSwitch::setVersion(uint8_t version) {
    version_ = version;
}

void OpenFlowSwitch::setPortCount(uint32_t count) {
    // Reserve space for ports
    ports_.reserve(count);
}

void OpenFlowSwitch::addPort(const PortInfo& port) {
    ports_.push_back(port);
}

uint64_t OpenFlowSwitch::getDatapathId() const {
    return datapath_id_;
}

uint8_t OpenFlowSwitch::getVersion() const {
    return version_;
}

uint32_t OpenFlowSwitch::getPortCount() const {
    return ports_.size();
}

const std::vector<PortInfo>& OpenFlowSwitch::getPorts() const {
    return ports_;
}
