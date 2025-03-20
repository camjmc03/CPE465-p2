#include "of_controller.h"
#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>
#include <atomic>

OpenFlowController* controller = nullptr;
std::atomic<bool> keep_running(true);

void signalHandler(int signum) {
    std::cout << "Signal " << signum << " received, shutting down..." << std::endl;
    keep_running = false;
    if (controller) {
        controller->stop();
    }
}

int main(int argc, char* argv[]) {
    // Register signal handler
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Default OpenFlow port
    uint16_t port = 6653;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) {
                port = std::stoi(argv[++i]);
            } else {
                std::cerr << "Error: Port number required after " << arg << std::endl;
                return 1;
            }
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -p, --port PORT   Listen on PORT (default: 6653)" << std::endl;
            std::cout << "  -h, --help        Show this help message" << std::endl;
            return 0;
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            return 1;
        }
    }
    
    // Create and start the controller
    controller = new OpenFlowController(port);
    
    if (!controller->start()) {
        std::cerr << "Failed to start controller" << std::endl;
        delete controller;
        return 1;
    }
    
    std::cout << "OpenFlow controller is running. Press Ctrl+C to exit." << std::endl;
    
    // Wait for controller to be stopped or Ctrl+C
    while (keep_running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // Clean up
    delete controller;
    controller = nullptr;
    
    std::cout << "Controller has been shut down." << std::endl;
    return 0;
}
