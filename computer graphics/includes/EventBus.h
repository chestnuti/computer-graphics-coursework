#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>


struct Event {
    virtual ~Event() = default;
};


class EventBus {
private:
    struct EventWrapper {
        virtual ~EventWrapper() = default;
        const Event* event;
        std::type_index type;
    };

    template<typename T>
    struct Wrapper : EventWrapper {
        Wrapper(const T& evt) {
            event = new T(evt);
            type = typeid(T);
        }
        ~Wrapper() {
            delete event;
        }
    };

    std::vector<std::unique_ptr<EventWrapper>> queueBuffer;
    std::unordered_map<std::type_index, std::vector<std::function<void(const Event&)>>> subscribers;

public:
    template<typename EventType>
    void queue(const EventType& event) {
		// Store event in the queue buffer
        queueBuffer.emplace_back(std::make_unique<Wrapper<EventType>>(event));
    }

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

    template<typename EventType>
    void subscribe(std::function<void(const EventType&)> handler) {
        subscribers[typeid(EventType)].push_back(
            [handler](const Event& e) {
                handler(static_cast<const EventType&>(e));
            }
        );
    }
};

