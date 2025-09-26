#include "libtslog.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace tslog {

// Implementação ThreadSafeLogger
ThreadSafeLogger::ThreadSafeLogger(const std::string& filename, 
                                   LogLevel min_level,
                                   bool console_output)
    : min_level_(min_level), console_output_(console_output) {
    
    log_file_ = std::make_unique<std::ofstream>(filename, std::ios::app);
    if (!log_file_->is_open()) {
        throw std::runtime_error("Não foi possível abrir arquivo de log: " + filename);
    }
}

ThreadSafeLogger::~ThreadSafeLogger() {
    if (log_file_ && log_file_->is_open()) {
        log_file_->close();
    }
}

std::string ThreadSafeLogger::get_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string ThreadSafeLogger::get_thread_id() const {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

std::string ThreadSafeLogger::level_to_string(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

void ThreadSafeLogger::log(LogLevel level, const std::string& message) {
    if (level < min_level_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    std::string log_entry = "[" + get_timestamp() + "] " +
                           "[" + level_to_string(level) + "] " +
                           "[Thread:" + get_thread_id() + "] " +
                           message;
    
    // Escrever no arquivo
    if (log_file_ && log_file_->is_open()) {
        *log_file_ << log_entry << std::endl;
        log_file_->flush();
    }
    
    // Escrever no console se habilitado
    if (console_output_) {
        if (level >= LogLevel::ERROR) {
            std::cerr << log_entry << std::endl;
        } else {
            std::cout << log_entry << std::endl;
        }
    }
}

void ThreadSafeLogger::simple_log(const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    
    std::string log_entry = "[" + get_timestamp() + "] " +
                           "[Thread:" + get_thread_id() + "] " +
                           message;
    
    // Escrever no arquivo
    if (log_file_ && log_file_->is_open()) {
        *log_file_ << log_entry << std::endl;
        log_file_->flush();
    }
    
    // Escrever no console se habilitado
    if (console_output_) {
        std::cout << log_entry << std::endl;
    }
}

void ThreadSafeLogger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void ThreadSafeLogger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void ThreadSafeLogger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void ThreadSafeLogger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void ThreadSafeLogger::critical(const std::string& message) {
    log(LogLevel::CRITICAL, message);
}

void ThreadSafeLogger::set_min_level(LogLevel level) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    min_level_ = level;
}

void ThreadSafeLogger::set_console_output(bool enable) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    console_output_ = enable;
}

void ThreadSafeLogger::flush() {
    std::lock_guard<std::mutex> lock(log_mutex_);
    if (log_file_ && log_file_->is_open()) {
        log_file_->flush();
    }
}

// Implementação Logger (Singleton)
std::unique_ptr<ThreadSafeLogger> Logger::instance_ = nullptr;
std::mutex Logger::instance_mutex_;

void Logger::initialize(const std::string& filename, 
                       LogLevel min_level,
                       bool console_output) {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    instance_ = std::make_unique<ThreadSafeLogger>(filename, min_level, console_output);
}

ThreadSafeLogger& Logger::get_instance() {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    if (!instance_) {
        throw std::runtime_error("Logger não foi inicializado. Chame Logger::initialize() primeiro.");
    }
    return *instance_;
}

void Logger::debug(const std::string& message) {
    get_instance().debug(message);
}

void Logger::simple_log(const std::string& message) {
    get_instance().simple_log(message);
}

void Logger::info(const std::string& message) {
    get_instance().info(message);
}

void Logger::warning(const std::string& message) {
    get_instance().warning(message);
}

void Logger::error(const std::string& message) {
    get_instance().error(message);
}

void Logger::critical(const std::string& message) {
    get_instance().critical(message);
}

} // namespace tslog
