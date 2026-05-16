#include <iostream>
#include <string>

using namespace std;

int main() {
    string line;
    while (getline(cin, line)) {
        if (line.empty()) continue;
        cout << "Command: " << line << endl;
    }
    return 0;
}
