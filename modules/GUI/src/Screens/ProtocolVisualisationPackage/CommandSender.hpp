#pragma once
#include "../../Network/TcpServer/TcpServer.hpp"

class CommandSender {

public:
    explicit CommandSender();
    void sendPing();
    void sendStop();
    void sendRestart();
    void sendLaunch(double threshold, const std::string& mode, const std::string& topology);
    
private:

};
