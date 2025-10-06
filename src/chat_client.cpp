#include "chat_client.h"
#include "libtslog.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <cerrno>

namespace chat {

ChatClient::ChatClient(const std::string& host, int port)
    : socket_fd_(-1), is_connected_(false), server_host_(host), server_port_(port) {
}

ChatClient::~ChatClient() {
    disconnect();
}

bool ChatClient::connect(const std::string& client_id) {
    client_id_ = client_id;
    
    // Criar socket
    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd_ < 0) {
        std::string error_msg = "Erro ao criar socket: " + std::string(strerror(errno));
        LOG_ERROR(error_msg);
        std::cerr << error_msg << std::endl;
        return false;
    }
    
    // Configurar endereço do servidor
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port_);
    
    if (inet_pton(AF_INET, server_host_.c_str(), &server_addr.sin_addr) <= 0) {
        std::string error_msg = "Endereço inválido: " + server_host_;
        LOG_ERROR(error_msg);
        std::cerr << error_msg << std::endl;
        close(socket_fd_);
        return false;
    }
    
    std::cout << "Tentando conectar ao servidor " << server_host_ << ":" << server_port_ << "..." << std::endl;
    
    // Conectar ao servidor
    if (::connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::string error_msg = "Erro ao conectar: " + std::string(strerror(errno));
        LOG_ERROR(error_msg + " (" + server_host_ + ":" + std::to_string(server_port_) + ")");
        std::cerr << error_msg << std::endl;
        std::cerr << "Verifique se o servidor está rodando com: make run-server" << std::endl;
        close(socket_fd_);
        return false;
    }
    
    is_connected_ = true;
    LOG_INFO("Conectado ao servidor " + server_host_ + ":" + std::to_string(server_port_));
    
    // Iniciar thread para receber mensagens
    receive_thread_ = std::thread(&ChatClient::receive_messages, this);
    
    return true;
}

void ChatClient::receive_messages() {
    char buffer[4096];
    
    while (is_connected_) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            if (is_connected_) {
                LOG_INFO("Conexão com servidor perdida");
                is_connected_ = false;
            }
            break;
        }
        
        std::string message(buffer, bytes_received);
        
        // Chamar callback se definido
        if (message_callback_) {
            message_callback_(message);
        } else {
            // Saída padrão
            std::cout << message;
        }
    }
}

bool ChatClient::send_message(const std::string& message) {
    if (!is_connected_) {
        LOG_ERROR("Cliente não está conectado");
        return false;
    }
    
    ssize_t sent = send(socket_fd_, message.c_str(), message.length(), MSG_NOSIGNAL);
    if (sent < 0) {
        LOG_ERROR("Erro ao enviar mensagem");
        disconnect();
        return false;
    }
    
    return true;
}

void ChatClient::set_message_callback(std::function<void(const std::string&)> callback) {
    message_callback_ = callback;
}

void ChatClient::disconnect() {
    if (!is_connected_.exchange(false)) {
        return;
    }
    
    if (socket_fd_ >= 0) {
        shutdown(socket_fd_, SHUT_RDWR);
        close(socket_fd_);
        socket_fd_ = -1;
    }
    
    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }
    
    LOG_INFO("Desconectado do servidor");
}

} // namespace chat
