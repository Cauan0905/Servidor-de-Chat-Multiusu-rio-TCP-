#!/bin/bash

echo "========================================"
echo "  Teste de Múltiplos Clientes - Etapa 2"
echo "========================================"
echo ""

# Verificar se os binários existem
if [ ! -f "./bin/chat_server" ]; then
    echo "Erro: Servidor não compilado. Execute 'make' primeiro."
    exit 1
fi

if [ ! -f "./bin/chat_client" ]; then
    echo "Erro: Cliente não compilado. Execute 'make' primeiro."
    exit 1
fi

echo "Iniciando servidor em background..."
./bin/chat_server &
SERVER_PID=$!

# Aguardar servidor iniciar
sleep 2

echo "Servidor iniciado (PID: $SERVER_PID)"
echo ""
echo "Iniciando 3 clientes simulados..."
echo "Cada cliente enviará 5 mensagens"
echo ""

# Função para simular cliente
simulate_client() {
    local client_id=$1
    local num_messages=$2
    
    {
        for i in $(seq 1 $num_messages); do
            echo "Mensagem $i do Cliente $client_id"
            sleep 1
        done
        sleep 1
        echo "sair"
    } | ./bin/chat_client 127.0.0.1 8080 "Cliente$client_id" > /dev/null 2>&1 &
}

# Iniciar 3 clientes
simulate_client 1 5
CLIENT1_PID=$!

sleep 0.5
simulate_client 2 5
CLIENT2_PID=$!

sleep 0.5
simulate_client 3 5
CLIENT3_PID=$!

echo "Clientes iniciados!"
echo "Aguardando conclusão do teste..."
echo ""

# Aguardar clientes terminarem
wait $CLIENT1_PID 2>/dev/null
wait $CLIENT2_PID 2>/dev/null
wait $CLIENT3_PID 2>/dev/null

sleep 2

echo ""
echo "Clientes finalizados. Parando servidor..."
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo ""
echo "========================================"
echo "  Teste concluído!"
echo "========================================"
echo ""
echo "Verifique os logs:"
echo "  - server.log: Logs do servidor"
echo "  - client.log: Logs dos clientes"
echo ""
