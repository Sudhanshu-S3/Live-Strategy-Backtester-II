# Live-Strategy-Backtester

A modular C++ event‑driven trading/backtesting framework focused on clean abstractions, extensibility, and performance awareness.

## Features
- Event‑driven architecture (market, signal, order, fill, news events)
- Pluggable strategies (Strategy base + concrete implementations)
- Risk & portfolio management layers
- Multiple data sources (historical CSV, news, websocket stub)
- Analytics: walk‑forward, optimization, Monte Carlo, regime detection, ML model manager
- API server placeholder for external integration
- Performance monitoring (timers, performance tests)
- Config parsing & structured logging
- Unit, integration, performance tests (CTest)

## High‑Level Flow

```mermaid
%% TRADING SYSTEM ARCHITECTURE — LIGHT THEME (LEFT → RIGHT)
%% Clean version (no emojis), enlarged Event Bus
%% ------------------------------------------------------------

%% Init (Light Theme)
%%{init: {
  "theme": "default",
  "themeVariables": {
    "fontFamily": "Inter, Arial",
    "fontSize": "14px",
    "clusterBkg": "#f9fafb",
    "clusterBorder": "#e5e7eb",
    "primaryColor": "#f0f4ff",
    "primaryBorderColor": "#2563eb",
    "primaryTextColor": "#111827"
  }
}}%%

%% Layout config
%%{config: {
  "flowchart": {
    "curve": "basis",
    "nodeSpacing": 70,
    "rankSpacing": 70
  }
}}%%

flowchart LR

%% ================================
%% Phase 6 — User Interface
%% ================================
subgraph PH6["Phase 6: User Interface"]
    UI["Web Frontend<br/>React / Vue + D3.js"]
end

%% ================================
%% Phase 5 — API / Reporting
%% ================================
subgraph PH5["Phase 5: API & Reporting Layer"]
    API["API Gateway<br/>REST / WebSockets"]
end

%% ================================
%% Phase 4 — Analytics / ML
%% ================================
subgraph PH4["Phase 4: Advanced Analytics & ML"]
    ML["ML Model Manager<br/>Prediction Engine"]
    MR["Market Regime Detector<br/>Bull / Bear / Volatility"]
end

%% ================================
%% Phase 3 — Core Trading Engine
%% ================================
subgraph PH3["Phase 3: Core Application"]
    SM["Strategy Manager<br/>Executes trading logic"]
    PM["Portfolio Manager<br/>Tracks Positions & P&L"]
    RM["Risk Manager<br/>Enforces limits"]
end

%% ================================
%% Phase 2 — Data & Execution
%% ================================
subgraph PH2["Phase 2: Data & Execution Services"]
    DH["Live Price Data Handler<br/>WebSockets + simdjson"]
    NH["News / Sentiment Handler<br/>NLP + APIs"]
    EH["Execution Handler<br/>Simulated fills & latency"]
    HB["Historical DB<br/>FlatBuffers / HDF5"]
end

%% ================================
%% Phase X — Event Bus (Central Nervous System)
%% Increased size via custom styling
%% ================================
subgraph CNS["Phase 1: Event Bus"]
    EB(("Event Bus "))
end


%% FLOW CONNECTIONS (Left → Right)
UI -->|REST / WebSockets| API
API -->|Commands| SM
API -->|Portfolio Query| PM
API -->|ML Insights| ML

SM -->|Order Request| EB
EB -->|Risk Check| RM
RM -->|Validated Order| EH
EH -->|Fill Confirmation| EB
EB -->|Update Portfolio| PM
EB -->|Live Data Feed| SM

DH -->|PriceTickEvent| EB
NH -->|SentimentEvent| EB
HB -->|HistoricalDataEvent| EB

EB -->|Feature Stream| ML
EB -->|Market Regime Data| MR
ML -->|PredictionEvent| EB
MR -->|RegimeChangeEvent| EB


%% ================================
%% Color coding / class styling
%% ================================
classDef ui fill:#dbeafe,stroke:#2563eb,stroke-width:2px,color:#111827;
classDef api fill:#ede9fe,stroke:#7c3aed,stroke-width:2px,color:#111827;
classDef analytics fill:#ffedd5,stroke:#c2410c,stroke-width:2px,color:#111827;
classDef core fill:#dcfce7,stroke:#15803d,stroke-width:2px,color:#111827;
classDef data fill:#e0f2fe,stroke:#0369a1,stroke-width:2px,color:#111827;

class UI ui;
class API api;
class ML,MR analytics;
class SM,PM,RM core;
class DH,NH,EH,HB data;

%% Make the Event Bus larger + thicker border
classDef eventbus fill:#fff3cd,stroke:#b45309,stroke-width:3px,color:#111827,font-size:18px;
class EB eventbus;

linkStyle default stroke:#6b7280,stroke-width:1.5px;

```

DataHandler → EventBus → Strategy → RiskManager → PortfolioManager → ExecutionHandler → (fills back as events)
Offline analytics modules (optimizer, walk‑forward, Monte Carlo, regime detection, ML) operate on stored data/results.

## Directory Structure
- include/, src/: Headers & implementations (mirrored)
- core/: Application, EventBus, Portfolio, logging, common types
- data/: Data handlers (historic, news, websocket)
- strategy/: Base strategy + concrete strategies + manager
- execution/: Execution handler abstraction
- risk/: Risk manager
- analytics/: Optimization, walk‑forward, Monte Carlo, regime detection, ML, aggregate analytics
- utils/: Timing / performance helpers
- config/: Config & parser
- api/: API server stub
- tests/: Unit, integration, performance tests, sample data

## Build
```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

## Run (example)
```bash
./build/bin/hft_system
./build/bin/run_tests        
```

## Tests
```bash
cd build
ctest --output-on-failure
```

## Key Principles
Single Responsibility, Separation of Concerns, Dependency Inversion, Strategy & Observer patterns (via EventBus), modular extensibility, testability, performance awareness.

## Core Data Structures
- std::vector: Collections (strategies, Monte Carlo paths)
- std::unordered_map: Symbol → position, config key → value
- std::queue/deque: Event dispatch buffering
- Smart pointers (unique_ptr/shared_ptr): Polymorphic ownership
- std::chrono: Timing, performance metrics
- Enums & variant event types (in Event system)

## Extending
1. Add a new Strategy: create header/source inheriting Strategy, register in StrategyManager.
2. Add a data source: subclass DataHandler, publish events to EventBus.
3. Add analytics module: implement component, integrate via Analytics orchestrator.
4. Add risk rule: extend RiskManager logic or modularize into rule set.

## Disclaimer
For educational / research use.
