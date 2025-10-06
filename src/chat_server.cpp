#include "chat_server.h"
#include "libtslog.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <algorithm>

namespace chat {

// Implementação Message
Message::Message(const std::string& sender, const std::string& msg)
    : sender_id(sender), content(msg), timestamp(std::chrono::system_clock::now()) {
}

// Implementação ClientManager
void ClientManager::add_client(std::shared_ptr<Client> client) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    clients_.push_back(client);
    LOG_INFO("Cliente adicionado: " + client->get_id() + " (Total: " + std::to_string(clients_.size()) + ")");
}

void ClientManager::remove_client(const std::string& client_id) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = std::remove_if(clients_.begin(), clients_.end(),
        [&client_id](const std::shared_ptr<Client>& client) {
            return client->get_id() == client_id;
        });
    
    if (it != clients_.end()) {
        clients_.erase(it, clients_.end());
        LOG_INFO("Cliente removido: " + client_id + " (Total: " + std::to_string(clients_.size()) + ")");
    }
}

std::vector<std::shared_ptr<Client>> ClientManager::get_all_clients() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_;
}

size_t ClientManager::get_client_count() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_.size();
}

std::shared_ptr<Client> ClientManager::find_client(const std::string& client_id) const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = std::find_if(clients_.begin(), clients_.end(),
        [&client_id](const std::shared_ptr<Client>& client) {
            return client->get_id() == client_id;
        });
    return (it != clients_.end()) ? *it : nullptr;
}

// Implementação MessageHistory
void MessageHistory::add_message(const Message& message) {
    std::lock_guard<std::mutex> lock(history_mutex_);
    messages_.push(message);
    
    // Limitar tamanho do histórico
    while (messages_.size() > MAX_HISTORY_SIZE) {
        messages_.pop();
    }
}

std::vector<Message> MessageHistory::get_recent_messages(size_t count) const {
    std::lock_guard<std::mutex> lock(history_mutex_);
    std::vector<Message> result;
    
    // Converter queue para vector (limitado)
    std::queue<Message> temp = messages_;
    while (!temp.empty() && result.size() < count) {
        result.push_back(temp.front());
        temp.pop();
    }
    
    return result;
}

size_t MessageHistory::get_message_count() const {
    std::lock_guard<std::mutex> lock(history_mutex_);
    return messages_.size();
}

// Implementação Client
Client::Client(const std::string& id, int socket_fd)
    : client_id_(id), socket_fd_(socket_fd), is_connected_(true) {
}

Client::~Client() {
    disconnect();
}

void Client::disconnect() {
    if (is_connected_.exchange(false)) {
        if (socket_fd_ >= 0) {
            close(socket_fd_);
            socket_fd_ = -1;
        }
    }
}

bool Client::send_message(const std::string& message) {
    if (!is_connected_) {
        return false;
    }
    
    ssize_t sent = send(socket_fd_, message.c_str(), message.length(), MSG_NOSIGNAL);
    if (sent < 0) {
        LOG_ERROR("Erro ao enviar mensagem para " + client_id_);
        disconnect();
        return false;
    }
    
    return true;
}

// Implementação ChatServer
ChatServer::ChatServer(int port, int max_clients)
    : server_socket_(-1), is_running_(false), port_(port), max_clients_(max_clients) {
}

ChatServer::~ChatServer() {
    stop();
}

bool ChatServer::start() {
    // Criar socket
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        LOG_ERROR("Erro ao criar socket do servidor");
        return false;
    }
    
    // Permitir reutilização de endereço
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOG_ERROR("Erro ao configurar socket");
        close(server_socket_);
        return false;
    }
    
    // Configurar endereço
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);
    
    // Bind
    if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOG_ERROR("Erro ao fazer bind na porta " + std::to_string(port_));
        close(server_socket_);
        return false;
    }
    
    // Listen
    if (listen(server_socket_, max_clients_) < 0) {
        LOG_ERROR("Erro ao escutar conexões");
        close(server_socket_);
        return false;
    }
    
    is_running_ = true;
    LOG_INFO("Servidor iniciado na porta " + std::to_string(port_));
    
    // Iniciar thread de aceitação
    accept_thread_ = std::thread(&ChatServer::accept_connections, this);
    
    return true;
}

void ChatServer::accept_connections() {
    while (is_running_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            if (is_running_) {
                LOG_ERROR("Erro ao aceitar conexão");
            }
            continue;
        }
        
        // Criar ID do cliente
        std::string client_id = std::string(inet_ntoa(client_addr.sin_addr)) + ":" + 
                               std::to_string(ntohs(client_addr.sin_port));
        
        LOG_INFO("Nova conexão de: " + client_id);
        
        // Criar cliente
        auto client = std::make_shared<Client>(client_id, client_socket);
        client_manager_.add_client(client);
        
        // Criar thread para lidar com o cliente
        std::thread(&ChatServer::handle_client, this, client).detach();
    }
}

void ChatServer::handle_client(std::shared_ptr<Client> client) {
    std::string join_notification = "*** " + client->get_id() + " entrou no chat ***\n";
    auto clients = client_manager_.get_all_clients();
    for (const auto& other_client : clients) {
        if (other_client->get_id() != client->get_id()) {
            other_client->send_message(join_notification);
        }
    }
    LOG_INFO("Notificação de entrada enviada para outros clientes");
    
    send_history_to_client(client);
    
    char buffer[4096];
    
    while (is_running_ && client->is_connected()) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_received = recv(client->get_socket(), buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            LOG_INFO("Cliente desconectado: " + client->get_id());
            
            std::string leave_notification = "*** " + client->get_id() + " saiu do chat ***\n";
            auto remaining_clients = client_manager_.get_all_clients();
            for (const auto& other_client : remaining_clients) {
                if (other_client->get_id() != client->get_id()) {
                    other_client->send_message(leave_notification);
                }
            }
            LOG_INFO("Notificação de saída enviada para outros clientes");
            
            client->disconnect();
            client_manager_.remove_client(client->get_id());
            break;
        }
        
        std::string message_content(buffer, bytes_received);
        LOG_INFO("Mensagem de " + client->get_id() + ": " + message_content);
        
        // Criar mensagem e adicionar ao histórico
        Message message(client->get_id(), message_content);
        message_history_.add_message(message);
        
        // Broadcast para todos os clientes
        broadcast_message(message, client->get_id());
    }
}

void ChatServer::send_history_to_client(std::shared_ptr<Client> client) {
    auto recent_messages = message_history_.get_recent_messages(50);
    
    if (recent_messages.empty()) {
        std::string welcome_msg = "=== Bem-vindo ao Chat! Você é o primeiro aqui. ===\n";
        client->send_message(welcome_msg);
        return;
    }
    
    std::string header = "=== Histórico de Mensagens (últimas " + 
                        std::to_string(recent_messages.size()) + " mensagens) ===\n";
    client->send_message(header);
    
    for (const auto& message : recent_messages) {
        std::string formatted_message = "[" + message.sender_id + "]: " + message.content + "\n";
        client->send_message(formatted_message);
    }
    
    std::string footer = "=== Fim do Histórico ===\n";
    client->send_message(footer);
    
    LOG_INFO("Histórico enviado para " + client->get_id() + " (" + 
             std::to_string(recent_messages.size()) + " mensagens)");
}

void ChatServer::broadcast_message(const Message& message, const std::string& sender_id) {
    std::string formatted_message = "[" + message.sender_id + "]: " + message.content + "\n";
    
    auto clients = client_manager_.get_all_clients();
    for (const auto& client : clients) {
        // Não enviar de volta para o remetente
        if (client->get_id() != sender_id) {
            client->send_message(formatted_message);
        }
    }
}

void ChatServer::stop() {
    if (!is_running_.exchange(false)) {
        return;
    }
    
    LOG_INFO("Parando servidor...");
    
    if (server_socket_ >= 0) {
        shutdown(server_socket_, SHUT_RDWR);
        close(server_socket_);
        server_socket_ = -1;
    }
    
    // Desconectar todos os clientes
    auto clients = client_manager_.get_all_clients();
    for (auto& client : clients) {
        client->disconnect();
    }
    
    // Aguardar thread de aceitação
    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }
    
    LOG_INFO("Servidor parado");
}

size_t ChatServer::get_connected_clients() const {
    return client_manager_.get_client_count();
}

size_t ChatServer::get_total_messages() const {
    return message_history_.get_message_count();
}

} // namespace chat
