# Makefile para o projeto Chat Server TCP - Etapa 1
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

# Targets principais
.PHONY: all clean test help run-test

all: libtslog test
	@echo ""
	@echo "✅ Compilação concluída com sucesso!"
	@echo ""
	@echo "📋 Para executar:"
	@echo "   make run-test    - Executa o teste do libtslog"
	@echo "   make help        - Mostra todos os comandos disponíveis"
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

$(BINDIR)/%: $(TESTDIR)/%.cpp $(OBJECTS)
	@echo "Compilando teste $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(OBJECTS) -o $@

# Executar teste do libtslog
run-test: $(BINDIR)/test_libtslog
	@echo ""
	@echo "🚀 Executando teste do libtslog..."
	@echo "================================================"
	./$(BINDIR)/test_libtslog
	@echo "================================================"
	@echo "✅ Teste concluído!"

# Limpeza
clean:
	@echo "Limpando arquivos compilados..."
	rm -rf $(OBJDIR) $(BINDIR)
	rm -f *.txt *.log

# Ajuda
help:
	@echo ""
	@echo "📋 Targets disponíveis:"
	@echo "  make           - Compila tudo e mostra instruções"
	@echo "  make run-test  - Executa o teste do libtslog"
	@echo "  make clean     - Remove arquivos compilados"
	@echo "  make help      - Mostra esta ajuda"
	@echo ""
	@echo "🎯 Uso típico:"
	@echo "  make && make run-test"
	@echo ""

# Dependências
$(OBJDIR)/libtslog.o: include/libtslog.h
$(BINDIR)/test_libtslog: include/libtslog.h $(OBJDIR)/libtslog.o
