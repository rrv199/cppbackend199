#pragma once

#include <boost/log/trivial.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/json.hpp>
#include <iostream>

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace json = boost::json;

// Атрибуты для дополнительных данных
BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", json::value)

class Logger {
public:
    static void Init() {
        // Добавляем общие атрибуты
        logging::add_common_attributes();
        
        // Настраиваем формат вывода
        logging::formatter formatter = [] (logging::record_view const& rec, logging::formatting_ostream& str) {
            json::object obj;
            
            // timestamp
            auto timestamp = logging::extract<boost::posix_time::ptime>("TimeStamp", rec);
            if (timestamp) {
                obj["timestamp"] = boost::posix_time::to_iso_extended_string(*timestamp);
            }
            
            // message
            auto message = logging::extract<std::string>("Message", rec);
            if (message) {
                obj["message"] = *message;
            }
            
            // additional data
            auto data = rec[additional_data];
            if (data) {
                obj["data"] = data.extract<json::value>();
            }
            
            str << json::serialize(obj);
        };
        
        // Настраиваем вывод в консоль
        logging::add_console_log(std::cout, keywords::format = formatter);
        
        // Устанавливаем severity level
        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
    }
    
    template<typename T>
    static void Log(logging::trivial::severity_level level, T&& message) {
        BOOST_LOG_STREAM_WITH_PARAMS(::boost::log::trivial::logger::get(), \
            (::boost::log::keywords::severity = level)) << message;
    }
    
    template<typename T>
    static void Log(json::value data, logging::trivial::severity_level level, T&& message) {
        BOOST_LOG_STREAM_WITH_PARAMS(::boost::log::trivial::logger::get(), \
            (::boost::log::keywords::severity = level)) \
            << logging::add_value(additional_data, std::move(data)) << message;
    }
};
