#ifndef HFT_SYSTEM_PORTFOLIOMANAGER_H
#define HFT_SYSTEM_PORTFOLIOMANAGER_H
#include <map>
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
        
        void on_fill(const Event &event);
        

        double capital_;
        double cash_;
        std::map<std::string, double> latest_prices_;
    };

} // namespace hft_system

#endif // HFT_SYSTEM_PORTFOLIOMANAGER_H