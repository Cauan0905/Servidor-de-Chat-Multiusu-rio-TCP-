#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <string>
#include <thread>
#include <atomic>
#include <functional>

namespace chat {

class ChatClient {
private:
    int socket_fd_;
    std::atomic<bool> is_connected_;
    std::thread receive_thread_;
    
    std::string server_host_;
    int server_port_;
    std::string client_id_;
    
    // Callback para mensagens recebidas
    std::function<void(const std::string&)> message_callback_;
    
    void receive_messages();
    
public:
    ChatClient(const std::string& host = "localhost", int port = 8080);
    ~ChatClient();
    
    bool connect(const std::string& client_id);
    void disconnect();
    bool is_connected() const { return is_connected_.load(); }
    
    bool send_message(const std::string& message);
    void set_message_callback(std::function<void(const std::string&)> callback);
    
    const std::string& get_client_id() const { return client_id_; }
};

} // namespace chat

#endif // CHAT_CLIENT_H
