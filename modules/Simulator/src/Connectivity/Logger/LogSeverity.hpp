#pragma once

#include <string>

enum class LogSeverity {
    DEBUG, // Detailed debugging information
    INFO,  // General informational messages
    WARNING, // Indications of potential issues
    ERROR,   // Error events that might still allow the application to continue running
    CRITICAL, // Severe error events that will presumably lead the application to abort
    SYSTEM // System-level messages
};

class LogSeverityHelper {
public:
    static std::string toString(LogSeverity severity) {
        switch (severity) {
            case LogSeverity::DEBUG:    return "DEBUG";
            case LogSeverity::INFO:     return "INFO";
            case LogSeverity::WARNING:  return "WARNING";
            case LogSeverity::ERROR:    return "ERROR";
            case LogSeverity::CRITICAL: return "CRITICAL";
            case LogSeverity::SYSTEM:   return "SYSTEM";
            default:                    return "UNKNOWN";
        }
    }

    static std::string getColorCode(LogSeverity severity) {
        switch (severity) {
            case LogSeverity::DEBUG:    return "\033[34m";        // Blue
            case LogSeverity::INFO:     return "\033[32m";        // Green
            case LogSeverity::WARNING:  return "\033[38;5;208m";  // Orange (256-color)
            case LogSeverity::ERROR:    return "\033[91m";        // Bright Red
            case LogSeverity::CRITICAL: return "\033[1m\033[91m"; // Bold Bright Red
            case LogSeverity::SYSTEM:   return "\033[35m";        // Magenta
            default:                    return "\033[0m";         // Reset
        }
    }

    static std::string getResetCode() {
        return "\033[0m";
    }
};
