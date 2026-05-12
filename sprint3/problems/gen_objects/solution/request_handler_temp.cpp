// Добавьте после получения target:
std::cerr << "DEBUG: raw target = " << req.target() << std::endl;
std::cerr << "DEBUG: after normalizing = " << target << std::endl;
std::cerr << "DEBUG: find api = " << target.find("api/") << std::endl;
