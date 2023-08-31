# Group Chat - Servidor e Cliente (atualizado)

Este é um projeto de servidor e cliente em C++ para um chat em grupo baseado em sockets TCP. O servidor utiliza conexões via socket TCP para comunicação com os clientes, e foi implementado com uma abordagem que utiliza threads separadas para lidar com cada cliente individualmente. O servidor recebe mensagens dos clientes e as transmite para todos os participantes do grupo, incluindo formatação, nome de usuário e horário da mensagem. Além disso, o sistema suporta comandos especiais que os clientes podem usar para executar ações adicionais.

## Estrutura do Projeto

O projeto está organizado em pastas de acordo com suas funcionalidades:

- `server/`: Contém o código e funcionalidades do servidor.
  - `user/`: Gerencia a estrutura dos clientes que se conectam ao servidor.
  - `slashFunctions/`: Estrutura e formata as mensagens enviadas/recebidas.
  - `server/`: Lida com as funcionalidades principais do servidor.

- `client/`: Contém o código do cliente. Lida com a conexão do cliente ao servidor.

## Funcionalidades do Servidor

O servidor é responsável por gerenciar o chat em grupo e os comandos dos clientes. Cada cliente que se conecta ao servidor é tratado por uma thread separada, permitindo que várias conversas ocorram simultaneamente. Os comandos disponíveis são:

Comando no cliente:
- `/mute usuário`: Muta o usuário especificado (apenas para o cliente que emitiu o comando).
- `/unmute usuário`: Desmuta o usuário especificado.
- `/changename novoUsuário`: Altera o nome do usuário que emitiu o comando.
- `/adminmute usuário`: Muta o usuário especificado para todos.
- `/adminunmute usuário`: Desmuta o usuário especificado para todos.
- `/help`: Mostra a lista de comandos disponíveis e suas funcionalidades.
- `/setadmin senha`: Concede privilégios de administrador ao usuário que emitiu o comando (senha necessária).
- `/userlist`: Mostra a lista de todos os usuários conectados ao servidor.

Comando no servidor:
- `/close`: Fecha o servidor.

## Compilação e Execução

Para compilar e executar o servidor, siga os seguintes passos:

1. Navegue para a pasta `server/`.
2. Execute o comando `make` para compilar o servidor.
3. Execute o comando `make run` para iniciar o servidor.

Para compilar e executar o cliente, siga os seguintes passos:

1. Navegue para a pasta `client/`.
2. Execute o comando `make` para compilar o cliente.
3. Execute o comando `make run` para iniciar o cliente.
4. Insira o IP do servidor.

## Observações

- A senha necessária para o comando `/setadmin` é armazenada no próprio código por fins práticos. Em uma implementação real, a senha deve ser armazenada de forma segura e protegida, talvez em um arquivo de configuração oculto.

Este projeto é uma implementação básica de um chat em grupo com funcionalidades de administração, agora utilizando sockets TCP e threads para cada cliente. Sinta-se à vontade para expandir e melhorar suas funcionalidades de acordo com as suas necessidades.
