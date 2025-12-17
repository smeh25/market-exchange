#pragma once

#include <boost/asio.hpp>
#include <deque>
#include <functional>
#include <memory>
#include <string>

namespace ex {

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
public:
    using InboundHandler = std::function<void(const std::string& client_id, const std::string& payload)>;

    Session(tcp::socket socket, InboundHandler on_msg)
        : socket_(std::move(socket)), on_msg_(std::move(on_msg)) {}

    void start() {
        client_id_ = socket_.remote_endpoint().address().to_string() + ":" +
                     std::to_string(socket_.remote_endpoint().port());
        do_read();
    }

    const std::string& client_id() const { return client_id_; }

    // Thread-safe enough for Phase 1 if all calls happen on the io_context thread.
    void send_line(std::string line) {
        if (line.empty() || line.back() != '\n') line.push_back('\n');

        bool writing = !outbox_.empty();
        outbox_.push_back(std::move(line));
        if (!writing) do_write();
    }

private:
    void do_read() {
        auto self = shared_from_this();
        asio::async_read_until(socket_, read_buf_, '\n',
            [this, self](boost::system::error_code ec, std::size_t /*n*/) {
                if (ec) return; // disconnected
                std::istream is(&read_buf_);
                std::string line;
                std::getline(is, line);
                if (!line.empty()) {
                    on_msg_(client_id_, line);
                }
                do_read();
            });
    }

    void do_write() {
        auto self = shared_from_this();
        asio::async_write(socket_, asio::buffer(outbox_.front()),
            [this, self](boost::system::error_code ec, std::size_t /*n*/) {
                if (ec) return;
                outbox_.pop_front();
                if (!outbox_.empty()) do_write();
            });
    }

    tcp::socket socket_;
    asio::streambuf read_buf_;
    std::deque<std::string> outbox_;
    InboundHandler on_msg_;
    std::string client_id_;
};

} 
