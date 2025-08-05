
#ifndef HFT_SYSTEM_EVENTBUS_H
#define HFT_SYSTEM_EVENTBUS_H

#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>
#include "../events/Event.h"

namespace hft_system {

class EventBus {
public:
    // Define the type for a subscriber callback function.
    // It takes a const reference to an Event.
    using Subscriber = std::function<void(const Event&)>;

    EventBus();
    ~EventBus();

    // The EventBus is a singleton-like object, it cannot be copied or moved.
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    /**
     * @brief Subscribes a callback to a specific event type.
     * @param type The EventType to subscribe to.
     * @param subscriber The callback function to be invoked.
     */
    void subscribe(EventType type, Subscriber subscriber);

    /**
     * @brief Publishes an event to the bus.
     * The event is added to a thread-safe queue and processed asynchronously.
     * @param event A shared pointer to the event.
     */
    void publish(std::shared_ptr<Event> event);

    /**
     * @brief Starts the event bus's processing thread.
     */
    void start();

    /**
     * @brief Stops the event bus's processing thread gracefully.
     */
    void stop();

private:
    /**
     * @brief The main loop that runs on a dedicated thread.
     * It waits for events and dispatches them to subscribers.
     */
    void dispatch_loop();

    // A map where keys are event types and values are lists of subscribers.
    std::map<EventType, std::vector<Subscriber>> subscribers_;
    std::mutex subscribers_mutex_; // Mutex to protect access to the subscribers map.

    // Thread-safe queue for incoming events.
    std::queue<std::shared_ptr<Event>> event_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cond_;

    // The dedicated thread for running the dispatch_loop.
    std::thread dispatch_thread_;
    std::atomic<bool> is_running_;
};

} // namespace hft_system

#endif // HFT_SYSTEM_EVENTBUS_H