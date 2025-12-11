#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>

class Actor;
class Hitbox;


struct Event {
    virtual ~Event() = default;
};


class EventBus {
private:
    struct EventWrapper {
        const Event* event;
        std::type_index type;

        EventWrapper(const Event* e, std::type_index t)
            : event(e), type(t) {
        }

        virtual ~EventWrapper() {
            delete event;
        }
    };

    template<typename T>
    struct Wrapper : EventWrapper {
        Wrapper(const T& evt)
            : EventWrapper(new T(evt), typeid(T)) {
        }
    };

    std::vector<std::unique_ptr<EventWrapper>> queueBuffer;
    std::unordered_map<std::type_index, std::vector<std::function<void(const Event&)>>> subscribers;

public:
	// Queue an event for later dispatch
    template<typename EventType>
    void queue(const EventType& event) {
        queueBuffer.emplace_back(std::make_unique<Wrapper<EventType>>(event));
    }

	// Dispatch all queued events to their subscribers
    void dispatch() {
        for (auto& e : queueBuffer) {
            auto it = subscribers.find(e->type);
            if (it != subscribers.end()) {
                for (auto& handler : it->second)
                    handler(*e->event);
            }
        }
        queueBuffer.clear();
    }

	// Subscribe to an event type
    template<typename EventType>
    void subscribe(std::function<void(const EventType&)> handler) {
        subscribers[typeid(EventType)].push_back(
            [handler](const Event& e) {
                handler(static_cast<const EventType&>(e));
            }
        );
    }
};




// Hitbox Collision Event
struct HitboxCollisionEvent : public Event {
    bool collided;
    Vec3 contactPoint;
    Actor* actorA;
    Actor* actorB;
};