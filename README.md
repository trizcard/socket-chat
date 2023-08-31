# Group Chat - Servidor e Cliente (TCP Socket e Threads)

Autores:
Beatriz Cardoso de Oliveira - NUSP: 12566400
Heitor Tanoue de Mello - NUSP: 12547260

Este é um projeto de servidor e cliente em C++ para um chat em grupo. O servidor utiliza sockets TCP para comunicação assíncrona e emprega uma thread dedicada para cada cliente conectado, permitindo a comunicação simultânea com vários clientes. O uso de threads garante que o servidor possa lidar com múltiplas conexões de forma eficiente, sem bloquear outras operações.

## Como Funcionam as Threads para Clientes

Quando um cliente se conecta ao servidor, uma nova thread é criada para esse cliente específico. Essa thread fica responsável por receber as mensagens do cliente, processá-las e enviá-las para todos os outros participantes do grupo. Isso permite que vários clientes interajam com o servidor ao mesmo tempo, sem atrasar a comunicação dos outros.

## Socket TCP

O TCP (Transmission Control Protocol) é um protocolo de comunicação confiável amplamente utilizado em redes de computadores. Ele garante que os dados enviados entre os dispositivos cheguem na ordem correta e sem perdas. No contexto deste projeto, o TCP é utilizado para estabelecer a comunicação entre o servidor e os clientes.

O servidor cria um socket TCP que fica aguardando por conexões de entrada. Quando um cliente se conecta, o servidor aceita a conexão e cria uma nova thread para lidar com essa conexão. Essa thread utiliza o socket para enviar e receber mensagens do cliente de forma confiável, garantindo que as mensagens cheguem ao destino sem corrupção ou perda.

## Estrutura do Projeto

O projeto está organizado em pastas de acordo com suas funcionalidades:

- `server/`: Contém o código e funcionalidades do servidor.
  - `user/`: Gerencia a estrutura dos clientes que se conectam ao servidor.
  - `slashFunctions/`: Estrutura e formata as mensagens enviadas/recebidas.
  - `server/`: Lida com as funcionalidades principais do servidor, incluindo a comunicação via sockets TCP e o gerenciamento de threads para cada cliente.

- `client/`: Contém o código do cliente. Lida com a conexão do cliente ao servidor.

## Funcionalidades do Servidor

O servidor é responsável por gerenciar o chat em grupo, utilizando sockets TCP para comunicação, e os comandos dos clientes. Os comandos disponíveis são:

Comando no cliente:
- `/mute usuário`: Muta o usuário especificado (apenas para o cliente que emitiu o comando).
- `/unmute usuário`: Desmuta o usuário especificado.
- `/changename newUser`: Altera o nome do usuário que emitiu o comando.
- `/adminmute usuário`: Muta o usuário especificado para todos.
- `/adminunmute usuário`: Desmuta o usuário especificado para todos.
- `/help`: Mostra a lista de comandos disponíveis e suas funcionalidades.
- `/setadmin senha`: Concede privilégios de administrador ao usuário que emitiu o comando (senha necessária).

## Compilação e Execução

Para compilar e executar o servidor, siga os seguintes passos:

1. Navegue para a pasta `server/`.
2. Execute o comando `make` para compilar o servidor.
3. Execute o comando `make run` para iniciar o servidor.

Para compilar e executar o cliente, siga os seguintes passos:

1. Navegue para a pasta `client/`.
2. Execute o comando `make` para compilar o cliente.
3. Execute o comando `make run` para iniciar o cliente.
4. Insira o IP do servidor

## Observações

- A senha necessária para o comando `/setadmin` é armazenada no próprio código por fins práticos. Em uma implementação real, a senha deve ser armazenada de forma segura e protegida, talvez em um arquivo de configuração oculto.

Este projeto é uma implementação básica de um chat em grupo utilizando comunicação via sockets TCP e threads individuais para cada cliente. Sinta-se à vontade para expandir e melhorar suas funcionalidades de acordo com as suas necessidades.
