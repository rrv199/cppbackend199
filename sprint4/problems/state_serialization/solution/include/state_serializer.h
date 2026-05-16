#pragma once
#include "game_state.h"
#include <string>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>

namespace game_serialization {

class StateSerializer {
public:
    // Сохранить состояние в файл
    static bool Save(const GameState& state, const std::string& filename) {
        // Сначала сохраняем во временный файл
        std::string temp_filename = filename + ".tmp";
        
        try {
            std::ofstream ofs(temp_filename, std::ios::binary);
            if (!ofs) {
                return false;
            }
            
            boost::archive::text_oarchive oa(ofs);
            oa << state;
            
            ofs.close();
            
            // Атомарно переименовываем временный файл в целевой
            std::filesystem::rename(temp_filename, filename);
            return true;
            
        } catch (const std::exception& e) {
            // Удаляем временный файл в случае ошибки
            std::filesystem::remove(temp_filename);
            return false;
        }
    }
    
    // Загрузить состояние из файла
    static bool Load(GameState& state, const std::string& filename) {
        if (!std::filesystem::exists(filename)) {
            return false;
        }
        
        try {
            std::ifstream ifs(filename, std::ios::binary);
            if (!ifs) {
                return false;
            }
            
            boost::archive::text_iarchive ia(ifs);
            ia >> state;
            
            return true;
            
        } catch (const std::exception& e) {
            return false;
        }
    }
    
    // Проверить, существует ли файл состояния
    static bool StateFileExists(const std::string& filename) {
        return std::filesystem::exists(filename);
    }
};

} // namespace game_serialization
