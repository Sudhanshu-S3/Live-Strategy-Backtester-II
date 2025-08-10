#ifndef HFT_SYSTEM_RISKMANAGER_H
#define HFT_SYSTEM_RISKMANAGER_H

#include "../core/Component.h"
#include "../config/Config.h"
#include "../analytics/MLModelManager.h"
#include <map>
#include <memory>

namespace hft_system {

class RiskManager : public Component {
public:
    RiskManager(std::shared_ptr<EventBus> event_bus, std::string name, const Config& config, std::shared_ptr<MLModelManager> ml_manager);

    void start() override;
    void stop() override;

private:
    void on_signal(const Event& event);
    void on_market(const Event& event);
    void on_portfolio_update(const Event& event);
    // Add a handler for order book events
    void on_order_book(const Event& event);

    RiskConfig risk_config_;
    std::shared_ptr<MLModelManager> ml_manager_;
    double latest_equity_ = 0.0;
    double latest_cash_ = 0.0;
    std::map<std::string, double> latest_prices_;
};

} // namespace hft_system
#endif // HFT_SYSTEM_RISKMANAGER_H