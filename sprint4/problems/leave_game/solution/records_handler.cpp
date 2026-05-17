        else if (target == "api/v1/game/records") {
            if (req.method() == http::verb::get) {
                // Парсим query параметры
                int start = 0;
                int max_items = 100;
                
                // Извлекаем query строку
                std::string query;
                auto query_pos = target.find('?');
                if (query_pos != std::string::npos) {
                    query = target.substr(query_pos + 1);
                    target = target.substr(0, query_pos);
                    
                    // Парсим параметры
                    std::istringstream iss(query);
                    std::string param;
                    while (std::getline(iss, param, '&')) {
                        auto eq_pos = param.find('=');
                        if (eq_pos != std::string::npos) {
                            std::string key = param.substr(0, eq_pos);
                            std::string value = param.substr(eq_pos + 1);
                            if (key == "start") {
                                start = std::stoi(value);
                            } else if (key == "maxItems") {
                                max_items = std::stoi(value);
                                if (max_items > 100) {
                                    auto response = MakeJsonResponse(http::status::bad_request,
                                        json::serialize(json::object{{"code", "badRequest"}, {"message", "maxItems cannot exceed 100"}}),
                                        11, true);
                                    send(std::move(response));
                                    return;
                                }
                            }
                        }
                    }
                }
                
                try {
                    auto records = record_manager_.GetRecords(start, max_items);
                    json::array records_array;
                    for (const auto& rec : records) {
                        records_array.push_back(json::object{
                            {"name", rec.name},
                            {"score", rec.score},
                            {"playTime", rec.play_time}
                        });
                    }
                    
                    auto response = MakeJsonResponse(http::status::ok, json::serialize(records_array), 11, true);
                    response.set(http::field::cache_control, "no-cache");
                    send(std::move(response));
                } catch (const std::exception& e) {
                    auto response = MakeJsonResponse(http::status::internal_server_error,
                        json::serialize(json::object{{"code", "internalError"}, {"message", e.what()}}),
                        11, true);
                    send(std::move(response));
                }
                return;
            } else {
                auto response = MakeJsonResponse(http::status::method_not_allowed,
                    json::serialize(json::object{{"code", "invalidMethod"}, {"message", "Method not allowed"}}),
                    11, true);
                response.set(http::field::allow, "GET");
                send(std::move(response));
                return;
            }
        }
