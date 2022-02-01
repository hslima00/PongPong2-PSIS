# PongPong
__________________________________________
FEITO
1. função rem_cliente -> se o cliente mandar msg do disconnect elimina do scoreboard, tira do array de clients etc
2. função add_client -> verifica se estamos a passar o nr maximo de clients e adiciona ao scoreboard (atribui o nr de player e mete score a 0) 
4. server tem array/estrutura q guarda ID's dos clients e qual dos clientes está em Play State
__________________________________________

TODOs

3. funções paddle mov, ball mov etc 
5. qual é a tecla/condição que define a saida do Play State / release_ball
6. função q desenha scoreboard -> atualiza player e pontuação |Posição|Nº Player|Score| -> ORGANIZA O SCOREBOARD
7. 

6 tipos de mensagens diferentes: 
The messages exchanged between the server and client are:

• 0- Connect (from client to the server)
• 1- Release_ball (from the client to the server)
• 2- Send_ball (from the server to the client)
• 3- Move_ball (from client to server and from server to client)
• 4- Disconnect (from client to server)
______________________________________

# ###### CLIENT #######

DONE 
1. send connect message (step A

TODO 
1.	Criar janela, paddle, movimentação da bola (código auxiliar)
2. criar Play State (entra neste estado quando o server manda msg Send_Ball)
3. Quando outro cliente entra fica a aguardar por um Send_Ball
4. When client is in the Play state, the user can release the ball (step D) and stop controlling the paddle. (Release_Ball)
5. Quando entra em *Play State* calcula o movimento da bola e manda para o server 

PADDLE
Every time the paddle moves, the client performs the following operations (step K):

• Calculate paddle movement depending on pressed key can use "arrows" or "wasd"(on client)
• move the paddle;
• (might not depend on paddle movement) calculates and updates the new ball position(on message),
• sends the new ball position to the server with the message Move_ball.
• (update paddle position (?)not on this project that done in the move paddle)

__________________________________
# ###### SERVER #######
DONE
2.	store message with client info (step B)
3.	If this client is the first one to connect,the server sends as reply a Send_ball message (step C). DONE
4.  When receiving Release_Ball from client the server chooses one of the available clients and sends the Send_ball message (step E)    
    4.1 if the client that released the ball is the only one connected, it will receive the ball back; se houver mais escolhe outro client
5. Server manda *Send_ball* message para o client e ele entra em Play State
6. 

