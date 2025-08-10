#ifndef HFT_SYSTEM_MLMODELMANAGER_H
#define HFT_SYSTEM_MLMODELMANAGER_H

#include "../core/Component.h"
#include "../config/Config.h"

namespace hft_system
{

    class MLModelManager : public Component
    {
    public:
        MLModelManager(std::shared_ptr<EventBus> event_bus, std::string name, const MLConfig &config);

        void start() override;
        void stop() override;

        // Simulates running a model to get a confidence score for a trade
        double get_trade_confidence(const std::string &symbol);

    private:
        MLConfig config_;
        bool model_loaded_ = false;
    };

} // namespace hft_system
#endif // HFT_SYSTEM_MLMODELMANAGER_H