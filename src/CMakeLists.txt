# src/CMakeLists.txt

# Create a library containing all our core system components.
# This allows the code to be shared between the main executable and the tests.
# src/CMakeLists.txt
add_library(hft_system_core
    core/EventBus.cpp
    core/Log.cpp
    core/PortfolioManager.cpp
    data/HistoricCSVDataHandler.cpp
    strategy/BuyEveryTickStrategy.cpp
    strategy/StrategyManager.cpp
    execution/ExecutionHandler.cpp # Add this line
)

# This library needs access to our public header files in the /include directory.
target_include_directories(hft_system_core PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
)

# Link our core library against the dependencies it uses.
target_link_libraries(hft_system_core PUBLIC
    spdlog::spdlog
)