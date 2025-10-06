#include "chat_client.h"
#include "libtslog.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string server_host = "127.0.0.1";
    int server_port = 8080;
    std::string client_id = "Cliente";
    
    if (argc > 1) {
        server_host = argv[1];
    }
    if (argc > 2) {
        server_port = std::atoi(argv[2]);
    }
    if (argc > 3) {
        client_id = argv[3];
    }
    
    // Inicializar logger (sem saída no console para não interferir no chat)
    tslog::Logger::initialize("client.log", tslog::LogLevel::INFO, false);
    
    std::cout << "========================================" << std::endl;
    std::cout << "   Cliente de Chat TCP - Etapa 2" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Servidor: " << server_host << ":" << server_port << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    chat::ChatClient client(server_host, server_port);
    
    if (!client.connect(client_id)) {
        std::cerr << "Erro ao conectar ao servidor!" << std::endl;
        return 1;
    }
    
    std::cout << "Conectado! Digite suas mensagens (ou 'sair' para desconectar):" << std::endl;
    std::cout << std::endl;
    
    std::string message;
    bool should_exit = false;
    
    while (!should_exit && client.is_connected()) {
        if (!std::cin.good()) {
            std::cout << "\nErro na entrada. Encerrando..." << std::endl;
            break;
        }
        
        std::getline(std::cin, message);
        
        if (message == "sair" || message == "exit" || message == "quit") {
            std::cout << "\nSaindo..." << std::endl;
            client.disconnect();
            std::cout << "Desconectado." << std::endl;
            return 0;
        }
        
        if (!message.empty()) {
            if (!client.send_message(message)) {
                std::cout << "\nErro ao enviar mensagem. Conexão perdida." << std::endl;
                break;
            }
        }
    }
    
    if (client.is_connected()) {
        client.disconnect();
    }
    
    std::cout << "Desconectado." << std::endl;
    
    return 0;
}
