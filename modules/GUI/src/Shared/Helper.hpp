#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <functional>
#include <mutex>
#include <vector>
#include <algorithm>
#include <TGUI/TGUI.hpp> // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>


//honestly no idea how this works
template <class T>
class Property {
public:
    using Callback = std::function<void(const T&)>;

    explicit Property(T v = {}) : value_(std::move(v)) {}

    // Subscribe; auto-unsub on Token destruction
    struct Token {
        size_t id{}; Property<T>* owner{};
        Token() = default;
        Token(size_t i, Property<T>* o): id(i), owner(o) {}
        Token(const Token&) = delete;
        Token& operator=(const Token&) = delete;
        Token(Token&& r) noexcept { *this = std::move(r); }
        Token& operator=(Token&& r) noexcept {
            if (this != &r) { reset(); id = r.id; owner = r.owner; r.owner = nullptr; }
            return *this;
        }
        ~Token() { reset(); }
        void reset() { if (owner) owner->unsubscribe(id); owner = nullptr; id = 0; }
    };

    Token subscribe(Callback cb) {
        std::lock_guard<std::mutex> lock(m_);
        const size_t id = ++counter_;
        listeners_.push_back({id, std::move(cb)});
        return Token{id, this};
    }

    void unsubscribe(size_t id) {
        std::lock_guard<std::mutex> lock(m_);
        auto it = std::remove_if(listeners_.begin(), listeners_.end(),
                                 [id](auto& p){ return p.first == id; });
        listeners_.erase(it, listeners_.end());
    }

    const T& get() const { return value_; }

    void set(const T& v) {
        std::vector<Callback> toCall;
        {
            std::lock_guard<std::mutex> lock(m_);
            if (v == value_) return;
            value_ = v;
            toCall.reserve(listeners_.size());
            for (auto& [_, cb] : listeners_) toCall.push_back(cb);
        }
        for (auto& cb : toCall) cb(value_);
    }

private:
    T value_{};
    mutable std::mutex m_;
    std::vector<std::pair<size_t, Callback>> listeners_;
    size_t counter_ = 0;
};

// One-way binding: source -> target (with optional transform)
template <class T, class U = T>
struct OneWayBinding {
    typename Property<T>::Token sub;
    OneWayBinding(Property<T>& source, Property<U>& target,
                  std::function<U(const T&)> transform = [](const T& v){ return U(v); })
    {
        sub = source.subscribe([&target, transform](const T& value) {
            target.set(transform(value));
        });
    }
};


//For the routing Arrow
// Function to create an arrowhead shape
inline sf::ConvexShape createArrowhead(sf::Vector2f position, sf::Vector2f direction, float headLength, float headWidth) {
    // Normalize the direction
    float magnitude = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    direction /= magnitude;

    // Perpendicular vector for arrowhead width
    sf::Vector2f perp(-direction.y, direction.x);

    // Arrowhead points
    sf::Vector2f tip = position;
    sf::Vector2f base1 = position - direction * headLength + perp * (headWidth / 2.0f);
    sf::Vector2f base2 = position - direction * headLength - perp * (headWidth / 2.0f);

    sf::ConvexShape arrowhead;
    arrowhead.setPointCount(3);
    arrowhead.setPoint(0, tip);
    arrowhead.setPoint(1, base1);
    arrowhead.setPoint(2, base2);
    arrowhead.setFillColor(sf::Color::Yellow);

    return arrowhead;
}

// Function to draw a shaft and multiple arrowheads along the shaft
inline void drawArrowWithHeads(tgui::CanvasSFML::Ptr canvas, sf::Vector2f start, sf::Vector2f end, float headSpacing) {
    // Calculate the direction vector and length of the arrow
    sf::Vector2f direction = end - start;
    float totalLength = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    // Normalize the direction vector
    direction /= totalLength;

    // Arrow properties
    float shaftThickness = 5.0f; // Thickness of the shaft
    float headLength = 20.0f;    // Length of each arrowhead
    float headWidth = 15.0f;     // Width of each arrowhead

    // Calculate the perpendicular vector for the shaft's thickness
    sf::Vector2f perp(-direction.y, direction.x);

    // Points for the shaft
    sf::Vector2f shaftStart1 = start + perp * (shaftThickness / 2.0f);
    sf::Vector2f shaftStart2 = start - perp * (shaftThickness / 2.0f);
    sf::Vector2f shaftEnd1 = end - perp * (shaftThickness / 2.0f);
    sf::Vector2f shaftEnd2 = end + perp * (shaftThickness / 2.0f);

    // Draw the shaft
    sf::ConvexShape shaft;
    shaft.setPointCount(4);
    shaft.setPoint(0, shaftStart1);
    shaft.setPoint(1, shaftStart2);
    shaft.setPoint(2, shaftEnd2);
    shaft.setPoint(3, shaftEnd1);
    shaft.setFillColor(sf::Color::Yellow);
    canvas->draw(shaft);

    // Place arrowheads along the shaft
    float currentLength = headSpacing; // Start after the first headSpacing
    while (currentLength < totalLength) {
        // Calculate the position for the current arrowhead
        sf::Vector2f headPosition = start + direction * currentLength;

        // Create and draw the arrowhead
        sf::ConvexShape arrowhead = createArrowhead(headPosition, direction, headLength, headWidth);
        canvas->draw(arrowhead);

        // Move to the next position
        currentLength += headSpacing;
    }
}



