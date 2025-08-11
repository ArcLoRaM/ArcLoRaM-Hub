#pragma once
#include "../../Network/TcpServer/TcpServer.hpp"

class CommandSender {

public:
    explicit CommandSender(TcpServer& server);
    void sendPing();
    void sendStop();
    void sendRestart();
    void sendLaunch(double threshold, const std::string& mode, const std::string& topology);
    std::string getClientStatus();
    
private:
    TcpServer& server;

};
