
    const std::unordered_map<int, std::shared_ptr<Player>>& GetPlayersOnMap(const std::string& map_id) const {
        static const std::unordered_map<int, std::shared_ptr<Player>> empty;
        auto it = map_players_.find(map_id);
        if (it != map_players_.end()) {
            return it->second;
        }
        return empty;
    }
