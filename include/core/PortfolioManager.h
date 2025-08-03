// include/core/PortfolioManager.h
#ifndef HFT_SYSTEM_PORTFOLIOMANAGER_H
#define HFT_SYSTEM_PORTFOLIOMANAGER_H

#include "Component.h"
#include "../events/Event.h" // Required for the method declarations

namespace hft_system
{

    class PortfolioManager : public Component
    {
    public:
        // Constructor DECLARATION (definition is in the .cpp file)
        PortfolioManager(std::shared_ptr<EventBus> event_bus, std::string name, double initial_capital);

        // Method DECLARATIONS for functions defined in the .cpp file
        void start() override;
        void stop() override;

    private:
        // Event handler DECLARATIONS
        void on_signal(const Event &event);
        void on_fill(const Event &event);

        // Member variables
        double capital_;
        double cash_;
    };

} // namespace hft_system

#endif // HFT_SYSTEM_PORTFOLIOMANAGER_H