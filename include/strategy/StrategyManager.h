#ifndef HFT_SYSTEM_STRATEGYMANAGER_H
#define HFT_SYSTEM_STRATEGYMANAGER_H

#include "../core/Component.h"
#include "../events/Event.h"
#include "Strategy.h"
#include <vector>
#include <memory>

namespace hft_system
{

    class StrategyManager : public Component
    {
    public:
        StrategyManager(std::shared_ptr<EventBus> event_bus, std::string name);

        void start() override;
        void stop() override;

        void add_strategy(std::unique_ptr<Strategy> strategy);

    private:
        void on_market_event(const Event &event);

        void on_order_book_event(const Event &event);

        std::vector<std::unique_ptr<Strategy>> strategies_;
    };

} // namespace hft_system

#endif // HFT_SYSTEM_STRATEGYMANAGER_H