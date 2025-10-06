#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>
#include <chrono>

namespace chat {

// Forward declarations
class Client;

struct Message {
    std::string sender_id;
    std::string content;
    std::chrono::system_clock::time_point timestamp;
    
    Message(const std::string& sender, const std::string& msg);
};

class ClientManager {
private:
    std::vector<std::shared_ptr<Client>> clients_;
    mutable std::mutex clients_mutex_;
    
public:
    void add_client(std::shared_ptr<Client> client);
    void remove_client(const std::string& client_id);
    std::vector<std::shared_ptr<Client>> get_all_clients() const;
    size_t get_client_count() const;
    std::shared_ptr<Client> find_client(const std::string& client_id) const;
};

class MessageHistory {
private:
    std::queue<Message> messages_;
    mutable std::mutex history_mutex_;
    static const size_t MAX_HISTORY_SIZE = 1000;
    
public:
    void add_message(const Message& message);
    std::vector<Message> get_recent_messages(size_t count = 50) const;
    size_t get_message_count() const;
};

class Client {
private:
    std::string client_id_;
    int socket_fd_;
    std::thread client_thread_;
    std::atomic<bool> is_connected_;
    
public:
    Client(const std::string& id, int socket_fd);
    ~Client();
    
    const std::string& get_id() const { return client_id_; }
    int get_socket() const { return socket_fd_; }
    bool is_connected() const { return is_connected_.load(); }
    
    void disconnect();
    bool send_message(const std::string& message);
};

class ChatServer {
private:
    int server_socket_;
    std::atomic<bool> is_running_;
    std::thread accept_thread_;
    
    ClientManager client_manager_;
    MessageHistory message_history_;
    
    int port_;
    int max_clients_;
    
    void accept_connections();
    void handle_client(std::shared_ptr<Client> client);
    void send_history_to_client(std::shared_ptr<Client> client);
    void broadcast_message(const Message& message, const std::string& sender_id = "");
    
public:
    ChatServer(int port = 8080, int max_clients = 100);
    ~ChatServer();
    
    bool start();
    void stop();
    bool is_running() const { return is_running_.load(); }
    
    size_t get_connected_clients() const;
    size_t get_total_messages() const;
};

} // namespace chat

#endif // CHAT_SERVER_H
