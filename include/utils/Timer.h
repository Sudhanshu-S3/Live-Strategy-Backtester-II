#pragma once

#include <chrono>
#include <string>
#include <iostream>
#include <utility>

namespace hft_system
{

    class Timer
    {
    public:
        Timer(std::string name = "")
            : name_(std::move(name)),
              start_time_(std::chrono::high_resolution_clock::now())
        {
        }

        void restart()
        {
            start_time_ = std::chrono::high_resolution_clock::now();
        }

        // Get elapsed time in various units
        int64_t elapsed_nanoseconds() const
        {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                       std::chrono::high_resolution_clock::now() - start_time_)
                .count();
        }

        int64_t elapsed_microseconds() const
        {
            return std::chrono::duration_cast<std::chrono::microseconds>(
                       std::chrono::high_resolution_clock::now() - start_time_)
                .count();
        }

        int64_t elapsed_milliseconds() const
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::high_resolution_clock::now() - start_time_)
                .count();
        }

        double elapsed_seconds() const
        {
            return elapsed_nanoseconds() / 1e9;
        }

        // Print elapsed time with name
        void log_elapsed() const
        {
            auto elapsed = elapsed_nanoseconds();
            if (!name_.empty())
            {
                std::cout << name_ << ": ";
            }

            if (elapsed < 1000)
            {
                std::cout << elapsed << " ns" << std::endl;
            }
            else if (elapsed < 1000000)
            {
                std::cout << elapsed / 1000.0 << " μs" << std::endl;
            }
            else if (elapsed < 1000000000)
            {
                std::cout << elapsed / 1000000.0 << " ms" << std::endl;
            }
            else
            {
                std::cout << elapsed / 1000000000.0 << " s" << std::endl;
            }
        }

    private:
        std::string name_;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
    };

} // namespace hft_system