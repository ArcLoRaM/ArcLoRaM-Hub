// ClientSession.hpp
#pragma once
#include <SFML/Network.hpp>
#include <optional>
#include <mutex>

class ClientSession {
public:
    static ClientSession& instance() noexcept {
        static ClientSession s;
        return s;
    }

    // setters
    void setConnected(bool state, std::optional<sf::IpAddress> ip = std::nullopt) {
        std::scoped_lock lock(m_);
        connected_ = state;
        remoteIp_  = std::move(ip);
    }

    // getters
    bool isConnected() const noexcept {
        std::scoped_lock lock(m_);
        return connected_;
    }

    std::optional<sf::IpAddress> remoteIp() const {
        std::scoped_lock lock(m_);
        return remoteIp_;
    }

private:
    ClientSession() = default;

    mutable std::mutex m_;
    bool connected_ = false;
    std::optional<sf::IpAddress> remoteIp_ = std::nullopt;
};
