# SuperPong

message types:
connect == 0
disconnect==1
Paddle_move ==2
Board_update ==3 


struct vai levar ball e paddle(might no be needed as it sends on the vector and can access with client number) + paddles restantes 
+ numero de cliente(enviado na primeira connect message pelo server que tbm inicia o paddle)
e guardado no client para sempre que enviar mensagem + dar highlight no score_board e no próprio paddle

o numero de client pode ser ajustado para "ordem de conexão" mas no primeiro ciclo o client recebe o seu client_number ? 

FAZER 
funcção que dá update aos dados co client que contactou/juntou

dois counters de clients (criados desde sempre// ativos )


major diff : 
ball moves on server and moves after x messages of certain type (
    m.msg_type == 3{
        contador_move_ball ++;
        if (contador move_ball+1 == client){
            //(simul_ball_move pbbly )
            move_ball();
            contador_move_ball = 0;
        }
        update_board;(força m.msg_type = 3)
        send board
    }
        fazer contador quando recebe um paddle move incrementa 
        e verifica condição para move_ball
            condição == (n_movimentos+1 == client )numero de moves = numero de players connectados 

client move paddle changes 
verifica se vai estar em contacto com outro paddle se sim não se move
If a ball touches a paddle, the corresponding user will get a point added to the score.

client if( m.msg_type==3)
    update_board(draws,all paddles(can can only draw own paddle here actually) and ball);
    also implies updating the score_board incase ball_move && point
    if (key == q/r conditions) and prints with the timer etc... 

    if (move_paddle(...))send_paddle_move 


func: 

server elimina 1 cliente após 30 sec de inatividade

criar uma opçao para 2 modos de jogo ( paddles movem se no y ou nao )
modo infinito ??
