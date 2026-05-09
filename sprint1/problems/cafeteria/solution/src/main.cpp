#include "cafeteria.h"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <mutex>

using namespace std::literals;

std::mutex cout_mutex;

int main() {
    const unsigned num_workers = 4;
    net::io_context io(num_workers);
    
    auto cafeteria = std::make_shared<Cafeteria>(io);
    
    std::atomic<int> completed_orders{0};
    const int total_orders = 16;
    
    std::cout << "Starting " << total_orders << " orders with " << num_workers << " workers..." << std::endl;
    
    auto start_time = std::chrono::steady_clock::now();
    
    for (int i = 0; i < total_orders; ++i) {
        cafeteria->OrderHotDog([&completed_orders](Result<HotDog> result) {
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                if (result.HasValue()) {
                    std::cout << "HotDog #" << result.GetValue().GetId() << " ready" << std::endl;
                } else {
                    std::cout << "Order failed" << std::endl;
                }
            }
            completed_orders++;
        });
    }
    
    std::vector<std::thread> workers;
    for (unsigned i = 0; i < num_workers; ++i) {
        workers.emplace_back([&io] { io.run(); });
    }
    
    while (completed_orders < total_orders) {
        std::this_thread::sleep_for(10ms);
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    io.stop();
    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }
    
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "\n=== SUMMARY ===" << std::endl;
        std::cout << "Total orders completed: " << completed_orders << std::endl;
        std::cout << "Time taken: " << duration << " ms" << std::endl;
    }
    
    return 0;
}
