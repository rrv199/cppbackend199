// Вставьте в начало оператора() после try, заменив текущую обработку статических файлов на:

    // Обработка статических файлов
    if (req.method() != http::verb::get && req.method() != http::verb::head) {
        send(MakeStringResponse(http::status::method_not_allowed, "Method not allowed", req.version(), req.keep_alive()));
        return;
    }
    
    try {
        // Нормализуем target (убираем ведущий /)
        std::string decoded = UrlDecode(target);
        if (decoded.empty()) {
            decoded = "index.html";
        }
        if (!decoded.empty() && decoded[0] == '/') {
            decoded = decoded.substr(1);
        }
        
        // Если decoded пустой - это корень, ищем index.html
        if (decoded.empty()) {
            decoded = "index.html";
        }
        
        // Формируем путь к файлу
        fs::path file_path = static_root_ / decoded;
        fs::path canonical_path = fs::weakly_canonical(file_path);
        fs::path abs_static_root = fs::weakly_canonical(static_root_);
        
        // Отладка
        std::cerr << "DEBUG: static_root = " << abs_static_root.string() << std::endl;
        std::cerr << "DEBUG: requested = " << decoded << std::endl;
        std::cerr << "DEBUG: full path = " << canonical_path.string() << std::endl;
        std::cerr << "DEBUG: exists = " << fs::exists(canonical_path) << std::endl;
        
        // Проверка безопасности
        if (canonical_path.string().find(abs_static_root.string()) != 0) {
            send(MakeStringResponse(http::status::bad_request, "Bad Request", req.version(), req.keep_alive()));
            return;
        }
        
        // Если это директория - добавляем index.html
        if (fs::is_directory(canonical_path)) {
            canonical_path /= "index.html";
        }
        
        // Проверка существования файла
        if (!fs::exists(canonical_path) || !fs::is_regular_file(canonical_path)) {
            send(MakeStringResponse(http::status::not_found, "Not Found", req.version(), req.keep_alive()));
            return;
        }
        
        std::string mime_type = GetMimeType(canonical_path.string());
        StringResponse response = MakeFileResponse(canonical_path, mime_type, req.version(), req.keep_alive());
        
        if (req.method() == http::verb::head) {
            response.body() = "";
        }
        
        send(std::move(response));
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        send(MakeStringResponse(http::status::internal_server_error, "Internal Error", req.version(), req.keep_alive()));
    }
