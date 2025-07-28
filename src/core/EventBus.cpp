#include "../../include/core/EventBus.h"
#include "../../include/core/Log.h"

namespace hft_system
{

    EventBus::EventBus() : is_running_(false) {}

    EventBus::~EventBus()
    {
        if (is_running_.load())
        {
            stop();
        }
    }

    void EventBus::subscribe(EventType type, Subscriber subscriber)
    {
        std::lock_guard<std::mutex> lock(subscribers_mutex_);
        subscribers_[type].push_back(std::move(subscriber));
        Log::get_logger()->info("New subscriber for event type {}", static_cast<int>(type));
    }

    void EventBus::publish(std::shared_ptr<Event> event)
    {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            event_queue_.push(std::move(event));
        }
        queue_cond_.notify_one();
    }

    void EventBus::start()
    {
        if (is_running_.load())
            return;
        is_running_.store(true);
        dispatch_thread_ = std::thread(&EventBus::dispatch_loop, this);
        Log::get_logger()->info("EventBus started.");
    }

    void EventBus::stop()
    {
        if (!is_running_.load())
            return;

        // Signal the dispatch loop to stop
        is_running_.store(false);
        queue_cond_.notify_all();

        if (dispatch_thread_.joinable())
        {
            dispatch_thread_.join();
        }
        Log::get_logger()->info("EventBus stopped.");
    }

    void EventBus::dispatch_loop()
    {
        Log::get_logger()->info("EventBus dispatch loop started.");

        // This loop will continue as long as the bus is running OR there are still events to process.
        while (is_running_.load() || !event_queue_.empty())
        {
            std::shared_ptr<Event> event;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                // Wait only if the bus is running and the queue is empty.
                queue_cond_.wait(lock, [this]
                                 { return !is_running_.load() || !event_queue_.empty(); });

                // If the queue is empty, the only reason we woke up is to shut down.
                if (event_queue_.empty())
                {
                    continue;
                }

                // Move the event out of the queue while under the lock.
                event = std::move(event_queue_.front());
                event_queue_.pop();
            }

            // Dispatch the event outside the lock.
            if (event)
            {
                std::vector<Subscriber> subscribers_to_notify;
                {
                    std::lock_guard<std::mutex> lock(subscribers_mutex_);
                    if (subscribers_.count(event->type))
                    {
                        subscribers_to_notify = subscribers_.at(event->type);
                    }
                }

                for (const auto &subscriber : subscribers_to_notify)
                {
                    try
                    {
                        subscriber(*event);
                    }
                    catch (const std::exception &e)
                    {
                        Log::get_logger()->error("Exception in subscriber: {}", e.what());
                    }
                }
            }
        }
        Log::get_logger()->info("EventBus dispatch loop finished.");
    }

} // namespace hft_system