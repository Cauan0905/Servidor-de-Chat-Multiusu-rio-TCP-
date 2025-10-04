# Makefile para o projeto Chat Server TCP - Etapa 2
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
INCLUDES = -Iinclude
SRCDIR = src
TESTDIR = test
OBJDIR = obj
BINDIR = bin

# Criar diretórios se não existirem
$(shell mkdir -p $(OBJDIR) $(BINDIR))

# Arquivos fonte
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Testes
TEST_SOURCES = $(wildcard $(TESTDIR)/*.cpp)
TEST_BINARIES = $(TEST_SOURCES:$(TESTDIR)/%.cpp=$(BINDIR)/%)

# Binários principais
SERVER_BIN = $(BINDIR)/chat_server
CLIENT_BIN = $(BINDIR)/chat_client

# Objetos necessários para servidor e cliente
SERVER_OBJECTS = $(OBJDIR)/libtslog.o $(OBJDIR)/chat_server.o $(OBJDIR)/server_main.o
CLIENT_OBJECTS = $(OBJDIR)/libtslog.o $(OBJDIR)/chat_client.o $(OBJDIR)/client_main.o

# Targets principais
.PHONY: all clean test help run-test server client run-server run-client run-server-bg test-multiple-clients

all: libtslog test server client
	@echo ""
	@echo "✅ Compilação concluída com sucesso!"
	@echo ""
	@echo "📋 Para executar:"
	@echo "   make run-test              - Executa o teste do libtslog"
	@echo "   make run-server            - Inicia o servidor de chat"
	@echo "   make run-client            - Inicia um cliente de chat"
	@echo "   make test-multiple-clients - Testa com múltiplos clientes automaticamente"
	@echo "   make help                  - Mostra todos os comandos disponíveis"
	@echo ""

# Biblioteca libtslog (como objeto)
libtslog: $(OBJECTS)
	@echo "Biblioteca libtslog compilada com sucesso!"

# Compilar objetos
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo "Compilando $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compilar testes
test: $(TEST_BINARIES)
	@echo "Testes compilados com sucesso!"

$(BINDIR)/%: $(TESTDIR)/%.cpp $(OBJDIR)/libtslog.o
	@echo "Compilando teste $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(OBJDIR)/libtslog.o -o $@

server: $(SERVER_BIN)
	@echo "Servidor compilado com sucesso!"

$(SERVER_BIN): $(SERVER_OBJECTS)
	@echo "Linkando servidor..."
	$(CXX) $(CXXFLAGS) $(SERVER_OBJECTS) -o $@

client: $(CLIENT_BIN)
	@echo "Cliente compilado com sucesso!"

$(CLIENT_BIN): $(CLIENT_OBJECTS)
	@echo "Linkando cliente..."
	$(CXX) $(CXXFLAGS) $(CLIENT_OBJECTS) -o $@

# Executar teste do libtslog
run-test: $(BINDIR)/test_libtslog
	@echo ""
	@echo "🚀 Executando teste do libtslog..."
	@echo "================================================"
	./$(BINDIR)/test_libtslog
	@echo "================================================"
	@echo "✅ Teste concluído!"

# Executar teste de múltiplos clientes
test-multiple-clients: $(SERVER_BIN) $(CLIENT_BIN)
	@echo ""
	@echo "🚀 Executando teste de múltiplos clientes..."
	@echo "================================================"
	@chmod +x scripts/test_multiple_clients.sh
	@./scripts/test_multiple_clients.sh
	@echo "================================================"
	@echo "✅ Teste de múltiplos clientes concluído!"

run-server: $(SERVER_BIN)
	@echo ""
	@echo "🚀 Iniciando servidor de chat..."
	@echo "================================================"
	./$(SERVER_BIN)

run-server-bg: $(SERVER_BIN)
	@echo ""
	@echo "🚀 Iniciando servidor de chat em background..."
	@echo "================================================"
	./$(SERVER_BIN) &
	@echo "✅ Servidor rodando em background (PID: $$!)"
	@echo "Para parar: pkill chat_server"
	@echo ""

run-client: $(CLIENT_BIN)
	@echo ""
	@echo "🚀 Iniciando cliente de chat..."
	@echo "================================================"
	./$(CLIENT_BIN)

# Limpeza
clean:
	@echo "Limpando arquivos compilados..."
	rm -rf $(OBJDIR) $(BINDIR)
	rm -f *.txt *.log

# Ajuda
help:
	@echo ""
	@echo "📋 Targets disponíveis:"
	@echo "  make                      - Compila tudo e mostra instruções"
	@echo "  make run-test             - Executa o teste do libtslog"
	@echo "  make test-multiple-clients - Testa com múltiplos clientes automaticamente"
	@echo "  make run-server           - Inicia o servidor de chat (porta 8080)"
	@echo "  make run-server-bg        - Inicia o servidor em background"
	@echo "  make run-client           - Inicia um cliente de chat"
	@echo "  make server               - Compila apenas o servidor"
	@echo "  make client               - Compila apenas o cliente"
	@echo "  make clean                - Remove arquivos compilados"
	@echo "  make help                 - Mostra esta ajuda"
	@echo ""
	@echo "🎯 Uso típico (manual):"
	@echo "  Terminal 1: make run-server"
	@echo "  Terminal 2: make run-client"
	@echo "  Terminal 3: make run-client"
	@echo ""
	@echo "🎯 Uso alternativo (um terminal):"
	@echo "  make run-server-bg && make run-client"
	@echo "  Para parar o servidor: pkill chat_server"
	@echo ""
	@echo "🎯 Teste automatizado:"
	@echo "  make test-multiple-clients"
	@echo ""

# Dependências
$(OBJDIR)/libtslog.o: include/libtslog.h
$(BINDIR)/test_libtslog: include/libtslog.h $(OBJDIR)/libtslog.o
$(SERVER_BIN): include/libtslog.h $(SERVER_OBJECTS)
$(CLIENT_BIN): include/libtslog.h $(CLIENT_OBJECTS)
