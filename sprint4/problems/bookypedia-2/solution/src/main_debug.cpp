// Найдите в AddBook строку:
// string title = trim(line.substr(cmd.length() + to_string(year).length()));
// И добавьте после нее:
cerr << "DEBUG: raw title from line: '" << line.substr(cmd.length() + to_string(year).length()) << "'" << endl;
cerr << "DEBUG: trimmed title: '" << title << "'" << endl;
cerr << "DEBUG: year: " << year << endl;
