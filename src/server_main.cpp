#include "chat_server.h"
#include "libtslog.h"
#include <iostream>
#include <csignal>
#include <memory>

std::unique_ptr<chat::ChatServer> server_instance;

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n\nRecebido sinal de interrupção. Parando servidor..." << std::endl;
        if (server_instance) {
            server_instance->stop();
        }
    }
}

int main(int argc, char* argv[]) {
    int port = 8080;
    
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    
    // Inicializar logger
    tslog::Logger::initialize("server.log", tslog::LogLevel::INFO, true);
    
    std::cout << "========================================" << std::endl;
    std::cout << "   Servidor de Chat TCP - Etapa 2" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Porta: " << port << std::endl;
    std::cout << "Pressione Ctrl+C para parar o servidor" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Configurar handler de sinais
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Criar e iniciar servidor
    server_instance = std::make_unique<chat::ChatServer>(port);
    
    if (!server_instance->start()) {
        std::cerr << "Erro ao iniciar servidor!" << std::endl;
        return 1;
    }
    
    // Manter servidor rodando
    while (server_instance->is_running()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "\nServidor finalizado com sucesso!" << std::endl;
    
    return 0;
}
