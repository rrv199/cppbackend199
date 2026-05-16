#pragma once
#include "game_state.h"
#include <string>
#include <fstream>
#include <filesystem>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace game_serialization {

class StateSerializer {
public:
    static bool Save(const GameState& state, const std::string& filename) {
        std::string temp_filename = filename + ".tmp";
        
        try {
            std::ofstream ofs(temp_filename);
            if (!ofs) return false;
            
            boost::archive::text_oarchive oa(ofs);
            oa << state;
            ofs.close();
            
            std::filesystem::rename(temp_filename, filename);
            return true;
        } catch (...) {
            std::error_code ec;
            std::filesystem::remove(temp_filename, ec);
            return false;
        }
    }
    
    static bool Load(GameState& state, const std::string& filename) {
        if (!std::filesystem::exists(filename)) return false;
        
        try {
            std::ifstream ifs(filename);
            if (!ifs) return false;
            
            boost::archive::text_iarchive ia(ifs);
            ia >> state;
            return true;
        } catch (...) {
            return false;
        }
    }
    
    static bool StateFileExists(const std::string& filename) {
        return std::filesystem::exists(filename);
    }
};

} // namespace game_serialization
