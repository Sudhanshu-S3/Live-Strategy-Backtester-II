add_library(hft_system_core
    core/EventBus.cpp
    core/Log.cpp
    core/PortfolioManager.cpp
    core/Application.cpp
    config/ConfigParser.cpp
    data/HistoricCSVDataHandler.cpp
    data/WebSocketDataHandler.cpp
    data/NewsDataHandler.cpp
    strategy/Strategy.cpp
    strategy/BuyEveryTickStrategy.cpp
    strategy/OrderBookImbalanceStrategy.cpp
    strategy/StrategyManager.cpp
    risk/RiskManager.cpp
    execution/ExecutionHandler.cpp
    analytics/Analytics.cpp
    api/APIServer.cpp
    
)

target_include_directories(hft_system_core PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${boost_beast_SOURCE_DIR}/include"
)

target_link_libraries(hft_system_core PUBLIC
    spdlog::spdlog
    simdjson::simdjson
    Boost::system
    Boost::thread
    cpr::cpr    
)