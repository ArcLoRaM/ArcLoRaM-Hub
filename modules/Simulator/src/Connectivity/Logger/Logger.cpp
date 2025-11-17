#include "Logger.hpp"
#include "../../Node/Node.hpp"

Logger::Logger()
    : logProcessor(std::make_unique<LogProcessor>()),
      tcpSender(std::make_unique<TcpSender>()) {
}

void Logger::start() {
    logProcessor->start();
    tcpSender->start();
}

void Logger::stop() {
    logProcessor->stop();
    tcpSender->stop();
}

// ===== LOGGING API =====
void Logger::logEvent(int nodeId, const std::string& message) {
    //TODO: flog to display log event when starting running
    // logProcessor->logEvent(nodeId, message);
}

void Logger::logDebug(const std::string& message) {
    logProcessor->logMessage(LogSeverity::DEBUG, message);
}

void Logger::logInfo(const std::string& message) {
    logProcessor->logMessage(LogSeverity::INFO, message);
}

void Logger::logWarning(const std::string& message) {
    logProcessor->logMessage(LogSeverity::WARNING, message);
}

void Logger::logError(const std::string& message) {
    logProcessor->logMessage(LogSeverity::ERROR, message);
}

void Logger::logCritical(const std::string& message) {
    logProcessor->logMessage(LogSeverity::CRITICAL, message);
}

void Logger::logSystem(const std::string& message) {
    logProcessor->logMessage(LogSeverity::SYSTEM, message);
}

void Logger::log(LogSeverity severity, const std::string& message) {
    logProcessor->logMessage(severity, message);
}

void Logger::setCurrentTick(uint64_t tick) {
    logProcessor->setCurrentTick(tick);
}

void Logger::resetTick() {
    logProcessor->resetTick();
}

// ===== TCP API =====
void Logger::sendTcpPacket(sf::Packet packet) {
    tcpSender->sendPacket(packet);
}

void Logger::setTcpClient(Client* clientPtr) {
    tcpSender->setClient(clientPtr);
}

// ===== CONFIGURATION =====
void Logger::enableFileOutput(const std::string& filepath) {
    logProcessor->enableFileOutput(filepath);
}

void Logger::setNodes(const std::vector<NodeInfo>& nodeInfoList) {
    logProcessor->setNodes(nodeInfoList);
}

void Logger::enableColorOutput(bool enabled) {
    logProcessor->enableColorOutput(enabled);
}

void Logger::exportCombinedSchedule(
    const std::vector<std::shared_ptr<Node>>& nodes,
    const std::multimap<int64_t, std::shared_ptr<Node>>& communicationSteps,
    const std::string& outputFile)
{
    logWarning("exportCombinedSchedule not implemented");
}
