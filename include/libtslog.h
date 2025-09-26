#ifndef LIBTSLOG_H
#define LIBTSLOG_H

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <chrono>
#include <thread>

namespace tslog {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

class ThreadSafeLogger {
private:
    std::unique_ptr<std::ofstream> log_file_;
    std::mutex log_mutex_;
    LogLevel min_level_;
    bool console_output_;
    
    std::string get_timestamp() const;
    std::string get_thread_id() const;
    std::string level_to_string(LogLevel level) const;
    
public:
    ThreadSafeLogger(const std::string& filename, 
                     LogLevel min_level = LogLevel::INFO,
                     bool console_output = true);
    
    ~ThreadSafeLogger();
    
    // Métodos principais de logging
    void log(LogLevel level, const std::string& message);
    void simple_log(const std::string& message); // Adicionando método de log simples sem níveis
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);
    
    // Configuração
    void set_min_level(LogLevel level);
    void set_console_output(bool enable);
    
    // Flush manual
    void flush();
};

// Singleton global para facilitar uso
class Logger {
private:
    static std::unique_ptr<ThreadSafeLogger> instance_;
    static std::mutex instance_mutex_;
    
public:
    static void initialize(const std::string& filename, 
                          LogLevel min_level = LogLevel::INFO,
                          bool console_output = true);
    
    static ThreadSafeLogger& get_instance();
    
    // Métodos estáticos para facilitar uso
    static void debug(const std::string& message);
    static void simple_log(const std::string& message); // Adicionando método estático simples
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    static void critical(const std::string& message);
};

} // namespace tslog

// Macros para facilitar uso
#define LOG_DEBUG(msg) tslog::Logger::debug(msg)
#define LOG_SIMPLE(msg) tslog::Logger::simple_log(msg) // Adicionando macro para log simples
#define LOG_INFO(msg) tslog::Logger::info(msg)
#define LOG_WARNING(msg) tslog::Logger::warning(msg)
#define LOG_ERROR(msg) tslog::Logger::error(msg)
#define LOG_CRITICAL(msg) tslog::Logger::critical(msg)

#endif // LIBTSLOG_H
