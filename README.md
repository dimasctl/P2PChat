Este é um trabalho da matéria de Redes cursada na USP no segundo semestre de 2024

# Chat P2P

## Ideia do Projeto
    Desenvolver um sistema de troca de mensagens (chat) fazendo uso de comunicação P2P (peer to peer)
entre dois dispositivos, buscando segurança e privacidade por meio de criptografia (toda mensagem
enviada é criptografada) e um servidor que auxilia a busca dos IP's de outras máquinas (buscando
armazenar a menor quantidade de informação possível)

---

## Funcionamento
    O sistema é composto por duas partes: Servidor e Cliente. O Servidor é responsável por
armazenar os IP's dos clientes e auxiliar na conexão P2P entre eles, enquanto o Cliente é responsável
por enviar e receber mensagens, além de manter a conexão com o servidor.


---

### Cliente:

#### Inicialização:
    O cliente, ao ser inicializado, verifica se existe um arquivo de backup/configuração. Caso não
exista, ele cria esse arquivo e armazena alguns valores gerados aleatoriamente: ID de usuário e chaves
de criptografia próprias. Ele então começa a enviar para o servidor uma mensagem de keep-alive a cada
10 segundos.
		
#### Descobrindo outro cliente:
    Para se conectar com outro cliente (chamaremos ele de cliente2), é necessário saber o ID do
cliente2. Esse ID é enviado para o servidor, e caso o cliente2 esteja online, o servidor retorna seu
IP, e o cliente toma pra si estabelecer uma conexão P2P via TCP (Protocolo de Controle de Transmissão)
com o cliente2. Caso o cliente2 não esteja online, o servidor retorna um código de erro indicando se
o cliente2 existe ou não, junto com seu último horário online.

#### Fazendo conexão P2P:
    Com o IP do cliente2, o cliente envia alguns dados do Usuário para o cliente2, que terá a opção
de aceitar ou não a conexão. Caso o Usuário do cliente2 aceite, o cliente2 retorna, por sua vez, com
dados de seu usuário. Caso contrário, o cliente2 retorna uma mensagem de conexão negada.


---

## Lista de tarefas:

- [x] Implementar a biblioteca de criptografia
- [x] Fazer o multithreading do cliente
- [ ] Implementar a comunicação P2P
- [ ] Implementar o servidor
- [ ] Implementar a comunicação entre cliente e servidor
- [ ] Definir o protocolo de comunicação com mais exatidão
