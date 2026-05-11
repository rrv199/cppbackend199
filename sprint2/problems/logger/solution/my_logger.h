#pragma once

#include <chrono>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <iomanip>
#include <ctime>

class Logger {
public:
    static Logger& GetInstance() {
        static Logger instance;
        return instance;
    }

    void SetTimestamp(std::chrono::system_clock::time_point timestamp) {
        std::lock_guard<std::mutex> lock(mutex_);
        fixed_timestamp_ = timestamp;
        use_fixed_timestamp_ = true;
    }

    std::chrono::system_clock::time_point GetTimeStamp() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (use_fixed_timestamp_) {
            return fixed_timestamp_;
        }
        return std::chrono::system_clock::now();
    }

    template <typename... Args>
    void Log(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = use_fixed_timestamp_ ? fixed_timestamp_ : std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        auto tm_now = *std::localtime(&time_t_now);
        
        std::string filename = GetFilename(tm_now);
        
        CheckAndRotate(filename, tm_now);
        
        std::ofstream file(current_filename_, std::ios::app);
        if (!file.is_open()) {
            return;
        }
        
        file << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S") << ": ";
        WriteToStream(file, std::forward<Args>(args)...);
        file << std::endl;
    }

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    template <typename T>
    void WriteToStream(std::ofstream& file, T&& arg) {
        file << std::forward<T>(arg);
    }
    
    template <typename T, typename... Rest>
    void WriteToStream(std::ofstream& file, T&& first, Rest&&... rest) {
        file << std::forward<T>(first);
        WriteToStream(file, std::forward<Rest>(rest)...);
    }
    
    std::string GetFilename(const std::tm& tm) {
        std::ostringstream oss;
        oss << "/var/log/sample_log_"
            << std::setfill('0')
            << (tm.tm_year + 1900) << "_"
            << std::setw(2) << (tm.tm_mon + 1) << "_"
            << std::setw(2) << tm.tm_mday
            << ".log";
        return oss.str();
    }
    
    void CheckAndRotate(const std::string& new_filename, const std::tm& tm) {
        int new_year = tm.tm_year + 1900;
        int new_month = tm.tm_mon + 1;
        int new_day = tm.tm_mday;
        
        if (new_year != current_year_ || new_month != current_month_ || new_day != current_day_) {
            current_filename_ = new_filename;
            current_year_ = new_year;
            current_month_ = new_month;
            current_day_ = new_day;
        }
    }
    
    mutable std::mutex mutex_;
    bool use_fixed_timestamp_ = false;
    std::chrono::system_clock::time_point fixed_timestamp_;
    std::string current_filename_;
    int current_year_ = 0;
    int current_month_ = 0;
    int current_day_ = 0;
};

// Макрос для удобного вызова
#define LOG(...) Logger::GetInstance().Log(__VA_ARGS__)
