#pragma once

#include "net/session.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <unordered_map>

namespace ex {

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

class Server {
public:
    using InboundHandler = Session::InboundHandler;

    Server(asio::io_context& io, uint16_t port, InboundHandler on_msg)
        : io_(io),
          acceptor_(io, tcp::endpoint(tcp::v4(), port)),
          on_msg_(std::move(on_msg)) {}

    void start() { do_accept(); }

    // For RequestRouter outbound sends
    bool send_to(const std::string& client_id, const std::string& line) {
        auto it = sessions_.find(client_id);
        if (it == sessions_.end()) return false;
        if (auto s = it->second.lock()) {
            s->send_line(line);
            return true;
        }
        sessions_.erase(it);
        return false;
    }

private:
    void do_accept() {
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                auto session = std::make_shared<Session>(std::move(socket), on_msg_);
                sessions_[session->client_id()] = session; // client_id set at start()
                session->start();
                // client_id becomes valid after start(); fix map entry:
                sessions_.erase(""); // no-op safety
                sessions_[session->client_id()] = session;
            }
            do_accept();
        });
    }

    asio::io_context& io_;
    tcp::acceptor acceptor_;
    InboundHandler on_msg_;
    std::unordered_map<std::string, std::weak_ptr<Session>> sessions_;
};

} // namespace ex
