#include "libtslog.h"
#include <thread>
#include <vector>
#include <chrono>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <atomic>

std::mutex round_mutex;
std::condition_variable round_cv;
std::atomic<int> threads_ready{0};
std::atomic<int> current_round{1};
const int num_threads = 5;

void worker_thread(int thread_id, int num_messages) {
    for (int round = 1; round <= num_messages; ++round) {
        {
            std::unique_lock<std::mutex> lock(round_mutex);
            threads_ready++;
            
            // Se sou a última thread a chegar nesta rodada
            if (threads_ready == num_threads) {
                current_round = round;
                threads_ready = 0;
                round_cv.notify_all();
            } else {
                // Esperar até todos chegarem
                round_cv.wait(lock, [round] { return current_round == round; });
            }
        }
        
        std::string message = "Thread " + std::to_string(thread_id) + 
                             " - Mensagem " + std::to_string(round);
        
        LOG_SIMPLE(message);
        
        // Pequeno delay para visualização
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    try {
        std::cout << "=== Teste da Biblioteca libtslog ===" << std::endl;
        std::cout << "Demonstrando logging thread-safe com sincronização por rodadas\n" << std::endl;
        std::cout << "Inicializando logger..." << std::endl;
        
        // Inicializar o logger
        tslog::Logger::initialize("test_log.txt", tslog::LogLevel::DEBUG, true);
        
        LOG_SIMPLE("Iniciando teste de concorrência do libtslog");
        
        const int messages_per_thread = 10;
        
        std::vector<std::thread> threads;
        
        std::cout << "Criando " << num_threads << " threads, cada uma com " 
                  << messages_per_thread << " mensagens..." << std::endl;
        std::cout << "Executando por rodadas: todas fazem msg1, depois msg2, etc...\n" << std::endl;
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(worker_thread, i + 1, messages_per_thread);
        }
        
        // Aguardar todas as threads terminarem
        for (auto& t : threads) {
            t.join();
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time);
        
        LOG_SIMPLE("Teste concluído em " + std::to_string(duration.count()) + "ms");
        
        std::cout << "\n=== Teste Concluído ===" << std::endl;
        std::cout << "Verifique o arquivo 'test_log.txt' para ver os logs." << std::endl;
        std::cout << "Tempo total: " << duration.count() << "ms" << std::endl;
        std::cout << "Total de mensagens: " << (num_threads * messages_per_thread) << std::endl;
        std::cout << "\nNota: Usando formato simples sem níveis de log para esta versão." << std::endl;
        std::cout << "O logger também suporta 5 níveis: DEBUG, INFO, WARNING, ERROR, CRITICAL" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Erro durante o teste: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
