#include "../../include/data/NewsDataHandler.h"
#include "../../include/core/Log.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cpr/cpr.h> // Include the CPR library for HTTP requests
#include "simdjson.h"

namespace hft_system
{

    double calculate_sentiment(const std::string &headline)
    {
        double score = 0.0;
        std::vector<std::string> positive_words = {"record", "high", "beats", "launches", "optimistic", "strong"};
        std::vector<std::string> negative_words = {"misses", "low", "plunges", "investigation", "fears", "weak"};

        std::string lower_headline = headline;
        std::transform(lower_headline.begin(), lower_headline.end(), lower_headline.begin(), ::tolower);

        for (const auto &word : positive_words)
        {
            if (lower_headline.find(word) != std::string::npos)
                score += 0.5;
        }
        for (const auto &word : negative_words)
        {
            if (lower_headline.find(word) != std::string::npos)
                score -= 0.5;
        }
        return std::clamp(score, -1.0, 1.0);
    }

    NewsDataHandler::NewsDataHandler(std::shared_ptr<EventBus> event_bus, std::string name)
        : DataHandler(event_bus, std::move(name)) {}

    void NewsDataHandler::start()
    {
        Log::get_logger()->info("{} started.", name_);
        run();
    }

    void NewsDataHandler::stop()
    {
        Log::get_logger()->info("{} stopped.", name_);
    }

    void NewsDataHandler::run()
    {
        Log::get_logger()->info("Fetching and analyzing news...");

        // This is a placeholder for a real news API.
        // We are using a free, generic news API for demonstration.
        // You would replace this URL and the parsing logic with your chosen provider.
        std::string api_url = "https://newsapi.org/v2/top-headlines?country=us&category=business&apiKey=YOUR_API_KEY";

        // Replace "YOUR_API_KEY" with a free key from newsapi.org
        cpr::Response r = cpr::Get(cpr::Url{api_url});

        if (r.status_code != 200)
        {
            Log::get_logger()->error("Failed to fetch news. Status code: {}", r.status_code);
            return;
        }

        try
        {
            simdjson::ondemand::parser parser;
            simdjson::ondemand::document doc = parser.iterate(r.text);

            for (auto article : doc["articles"].get_array())
            {
                std::string_view title_sv;
                if (article["title"].get_string().get(title_sv) == simdjson::SUCCESS)
                {
                    std::string headline(title_sv);
                    double sentiment = calculate_sentiment(headline);

                    // For simplicity, we'll assign the news to a default symbol.
                    // A more advanced system would parse entities from the headline.
                    auto news_event = std::make_shared<NewsEvent>("GENERIC", headline, sentiment);
                    event_bus_->publish(news_event);
                    Log::get_logger()->info("Published NewsEvent. Sentiment: {:.2f}, Headline: {}", sentiment, headline);
                }
            }
        }
        catch (const std::exception &e)
        {
            Log::get_logger()->error("Error parsing news JSON: {}", e.what());
        }
    }

} // namespace hft_system