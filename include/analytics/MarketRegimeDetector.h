#ifndef HFT_SYSTEM_MARKETREGIMEDETECTOR_H
#define HFT_SYSTEM_MARKETREGIMEDETECTOR_H

#include "../core/Component.h"
#include <vector>
#include <deque>

namespace hft_system
{

    class MarketRegimeDetector : public Component
    {
    public:
        MarketRegimeDetector(std::shared_ptr<EventBus> event_bus, std::string name, int volatility_period, int trend_period);

        void start() override;
        void stop() override;

    private:
        void on_market_event(const Event &event);
        void calculate_regime();

        int volatility_period_;
        int trend_period_;
        std::deque<double> price_history_;
        MarketState current_state_;
    };

} // namespace hft_system
#endif // HFT_SYSTEM_MARKETREGIMEDETECTOR_H