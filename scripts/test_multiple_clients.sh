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
echo "Iniciando 10 clientes simulados..."
echo "Cada cliente enviará até 15 mensagens"
echo "Clientes 3, 5 e 8 sairão mais cedo (após 8 mensagens)"
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

simulate_client 1 15
sleep 0.3

simulate_client 2 15
sleep 0.3

# Cliente 3 sai mais cedo
simulate_client 3 8
sleep 0.3

simulate_client 4 15
sleep 0.3

# Cliente 5 sai mais cedo
simulate_client 5 8
sleep 0.3

simulate_client 6 15
sleep 0.3

simulate_client 7 15
sleep 0.3

# Cliente 8 sai mais cedo
simulate_client 8 8
sleep 0.3

simulate_client 9 15
sleep 0.3

simulate_client 10 15

echo "Clientes iniciados!"
echo "Aguardando conclusão do teste..."
echo ""

# Aguardar todos os processos filhos terminarem
wait

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
echo "Estatísticas do teste:"
echo "  - Total de clientes: 10"
echo "  - Clientes que completaram 15 mensagens: 7"
echo "  - Clientes que saíram mais cedo: 3 (clientes 3, 5 e 8)"
echo ""
echo "Verifique os logs:"
echo "  - server.log: Logs do servidor"
echo "  - client.log: Logs dos clientes"
echo ""
