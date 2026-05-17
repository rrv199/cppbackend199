#pragma once
#include <memory>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <pqxx/pqxx>

class ConnectionPool {
public:
    using ConnectionPtr = std::shared_ptr<pqxx::connection>;

    template <typename ConnectionFactory>
    ConnectionPool(size_t capacity, ConnectionFactory&& connection_factory) {
        pool_.reserve(capacity);
        for (size_t i = 0; i < capacity; ++i) {
            pool_.emplace_back(connection_factory());
        }
    }

    class ConnectionWrapper {
    public:
        ConnectionWrapper(ConnectionPtr&& conn, ConnectionPool& pool) noexcept
            : conn_(std::move(conn)), pool_(&pool) {}
        
        pqxx::connection& operator*() { return *conn_; }
        pqxx::connection* operator->() { return conn_.get(); }

        ~ConnectionWrapper() {
            if (conn_) pool_->ReturnConnection(std::move(conn_));
        }
    private:
        ConnectionPtr conn_;
        ConnectionPool* pool_;
    };

    ConnectionWrapper GetConnection() {
        std::unique_lock lock(mutex_);
        cond_var_.wait(lock, [this] { return used_connections_ < pool_.size(); });
        return {std::move(pool_[used_connections_++]), *this};
    }

private:
    void ReturnConnection(ConnectionPtr&& conn) {
        std::lock_guard lock(mutex_);
        pool_[--used_connections_] = std::move(conn);
        cond_var_.notify_one();
    }

    std::mutex mutex_;
    std::condition_variable cond_var_;
    std::vector<ConnectionPtr> pool_;
    size_t used_connections_ = 0;
};
