# Create a library containing all our core system components.
# This allows the code to be shared between the main executable and the tests.


add_library(hft_system_core
    core/EventBus.cpp
    core/Log.cpp
    core/PortfolioManager.cpp
    config/ConfigParser.cpp
    data/HistoricCSVDataHandler.cpp
    strategy/BuyEveryTickStrategy.cpp
    strategy/StrategyManager.cpp
    execution/ExecutionHandler.cpp
    risk/RiskManager.cpp
)

target_include_directories(hft_system_core PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
)

# Link our core library against the dependencies it uses.
target_link_libraries(hft_system_core PUBLIC
    spdlog::spdlog
    simdjson::simdjson # This line is critical
)