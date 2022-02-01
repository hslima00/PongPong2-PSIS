#include "header.h"
/* creates the socket to with the correct type (AF_INET) use and checks if the creation was sucessful */ 
void create_socket(int *sock_fd)
{
    *sock_fd = socket(AF_INET, SOCK_DGRAM, 0); //cria socket
    if (*sock_fd == -1)
    { //verifica se socket esta bem criada
        perror("socket: ");
        exit(-1);
    }
}
/*initializes ncurses*/
void initialize_ncurses()
{
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
}

/*Creates a window and draws a border*/
void create_window(WINDOW *my_win)
{
    box(my_win, 0, 0);
    wrefresh(my_win);
    keypad(my_win, true);
}

/*copies ball information from um ball_position_t to another*/
void copy_ball_info(ball_position_t* ball_to_overwrite, ball_position_t *ball_to_copy_from){
    ball_to_overwrite->c= ball_to_copy_from->c;
    ball_to_overwrite->left_ver_right= ball_to_copy_from->left_ver_right;
    ball_to_overwrite->up_hor_down= ball_to_copy_from->up_hor_down;
    ball_to_overwrite->x= ball_to_copy_from->x;
    ball_to_overwrite->y= ball_to_copy_from->y;
}

/*At the start of the game places the ball in a random position*/
void place_ball_random(ball_position_t *ball,ball_position_t *ball_s )
{
    ball->x = rand() % WINDOW_SIZE;
    ball->y = rand() % WINDOW_SIZE;
    ball->c ='o';
    ball->up_hor_down = rand() % 3 - 1;    //  -1 up, 1 - down
    ball->left_ver_right = rand() % 3 - 1; // 0 vertical, -1 left, 1 right
    copy_ball_info(ball_s, ball);//initialize ball position on server
}

/*initializes  paddle position making sure it's a valid position*/
bool new_paddle (message *m, int clients_online){ 
    bool invalid_paddle= FALSE;// control variable to check if valid paddle
    bool condition_1=FALSE, condition_2=FALSE;//control variables to check conditions for valid move
    paddle_position simul_paddle;// simulated paddle to assess if the move is valid
    simul_paddle.x = WINDOW_SIZE/2;
    int adder=0;
    do{
        adder ++;
        simul_paddle.y = WINDOW_SIZE-2 - adder;
        for (int i = 0 ; i <= clients_online; i++){
            if (simul_paddle.y == m->cinfo[i].paddle_position.y){
                condition_1 = ((simul_paddle.x+ PADDLE_SIZE<=m->cinfo[i].paddle_position.x + PADDLE_SIZE )&& //right edge simul_paddle <= right edge paddle[i] AND
                        (simul_paddle.x +PADDLE_SIZE  >=m->cinfo[i].paddle_position.x - PADDLE_SIZE));//right edge simul_paddle >= left edge paddle[i]
                
                condition_2 =((simul_paddle.x - PADDLE_SIZE<=m->cinfo[i].paddle_position.x + PADDLE_SIZE ) &&//left edge  simul_paddle <= right edgepaddle[i] AND
                            (simul_paddle.x -PADDLE_SIZE  >=m->cinfo[i].paddle_position.x - PADDLE_SIZE));//left edge  simul_paddle >= left edge  paddle[i]
                
                invalid_paddle = ((condition_1  || condition_2) );  // if on the same height and touching paddle 
            }
            if(invalid_paddle) break;
        }
    }while (invalid_paddle);
    
    //se não encontrar paddle já criada, entao vai passar as cenas do paddle simulado para o cliente 
    m->cinfo[clients_online].paddle_position.length = PADDLE_SIZE;
    m->cinfo[clients_online].paddle_position.x=simul_paddle.x;
    m->cinfo[clients_online].paddle_position.y=simul_paddle.y;
    return FALSE;
}


/*moves the ball  simulating if the next position is valid and if not simulating bouncing effect on paddles or walls*/
void move_ball(message *m, client_info_s *cinfo_s, int clients_online, ball_position_t * ball_s){
    ball_position_t next_ball; // simulates if ball will hit de window
    int limite_esq = 2; 
    int limite_dir = WINDOW_SIZE-2;
    int limite_topo = 2;
    int limite_fundo = WINDOW_SIZE-2;
    int next_y, next_x;
    next_ball.left_ver_right =m->ball_position.left_ver_right;
    next_ball.up_hor_down= m->ball_position.up_hor_down;
    next_ball.x=  m->ball_position.x;
    next_ball.y= m->ball_position.y;

    if(m->point){//if m-> point == TRUE means the ball is already hitting a paddle so we will just move the ball 
        next_x=ball_s->x;
        next_y=ball_s->y;
        
    }else{// if move ball happens because we had enought moves  
        next_x =ball_s->x + m->ball_position.left_ver_right;
        next_y =ball_s->y + m->ball_position.up_hor_down;
    }
    if( next_x == 0 || next_x == WINDOW_SIZE-1){
            next_ball.up_hor_down = rand() % 3 -1 ;
            next_ball.left_ver_right *= -1;
        }else{
            next_ball.x = next_x;
        }
        if( next_y == 0 || next_y == WINDOW_SIZE-1){
            next_ball.up_hor_down *= -1;
            next_ball.left_ver_right = rand() % 3 -1;
        }else{
            next_ball.y = next_y;
        }
    bool hit;
    //check if ball will colide with any paddle
    for(int j =0; j<clients_online; j++){                                               //if ball is between
        hit =( ( (next_ball.x <= cinfo_s[j].paddle_position_s.x + PADDLE_SIZE) &&       //paddle right edge AND
            (next_ball.x >= cinfo_s[j].paddle_position_s.x - PADDLE_SIZE) ) &&          //paddle left edge
            (cinfo_s[j].paddle_position_s.y == next_ball.y));                           // betwen the edges and on the same y(height) 

        if(hit){
            // updates score value on server and on the message
            m->cinfo[j].score++;
            cinfo_s[j].score++;
            if(next_ball.up_hor_down == 1 || next_ball.up_hor_down==-1){    //if ball is going up or down
                next_ball.up_hor_down*=-1;  //changes vertical movement direction
            }else(next_ball.left_ver_right*=-1);// if horizontal changes horizontal direction
            
            //if the next ball is hitting the box limit makes it bounce accordingly
            if(next_ball.y == limite_fundo) {//bottom
                next_ball.y = WINDOW_SIZE -3;
                next_ball.up_hor_down = -1;
                if(next_ball.x == limite_esq) next_ball.left_ver_right=-1;
                else if(next_ball.x == limite_dir) next_ball.left_ver_right=1; 

            }
            else if(next_ball.y == limite_topo) {//top
                next_ball.y = 3;
                next_ball.up_hor_down = 1;
                if(next_ball.x == limite_esq) next_ball.left_ver_right=-1;
                else if(next_ball.x == limite_dir) next_ball.left_ver_right=1;   
            }
            else if(next_ball.x == limite_esq) { //left 
                if (next_ball.left_ver_right!=0)next_ball.left_ver_right= 1;
                if (next_ball.up_hor_down== 0){
                    if(next_ball.y <= WINDOW_SIZE/2)next_ball.up_hor_down=1; //se a bola estiver na metade superior da janela
                    else next_ball.up_hor_down=-1;                           //se "  "  " inferior da janela
                }    
            }           
            else if(next_ball.x == limite_dir) {//right
                if (next_ball.left_ver_right!=0)next_ball.left_ver_right=-1;
                if (next_ball.up_hor_down== 0){
                    if(next_ball.y <= WINDOW_SIZE/2)next_ball.up_hor_down=1; 
                    else next_ball.up_hor_down=-1;
                }
            }
            
            next_ball.x += 2 * next_ball.left_ver_right;
            next_ball.y += 2 * next_ball.up_hor_down;
        }
    }
    //Save position to ball
    copy_ball_info(&m->ball_position,&next_ball);
    //Save ball position to server
    copy_ball_info(ball_s,&m->ball_position);

}

/*updates paddle information on server after a paddle move message from client*/
void paddle_move(paddle_position * client_paddle_after_move,paddle_position * server_saved_position){
    server_saved_position->x = client_paddle_after_move->x;
    server_saved_position->y = client_paddle_after_move->y;
}
   
/*function to update server information from a specific client when necessary*/
void update_client_info(client_info_s * cinfo_s,message * m, int client_to_update,struct sockaddr_in client_addr,bool remove ){
    //falta address 
    int client_to_get_info_from= client_to_update;
    if (!remove)  {// if adding a new client gets the address and saves it 
        struct sockaddr_in* ptr_to_addr = (struct sockaddr_in*)&client_addr;
        struct in_addr addr_to_store = ptr_to_addr->sin_addr;
        inet_ntop( AF_INET, &addr_to_store, cinfo_s[client_to_update].client_address_s, INET_ADDRSTRLEN );
        cinfo_s[client_to_update].port = ntohs(client_addr.sin_port);
    }
    cinfo_s[client_to_update].client_ID = m->cinfo[client_to_get_info_from].client_ID;
    cinfo_s[client_to_update].score = m->cinfo[client_to_get_info_from].score;
    cinfo_s[client_to_update].paddle_position_s.length =PADDLE_SIZE;
    cinfo_s[client_to_update].paddle_position_s.x = m->cinfo[client_to_get_info_from].paddle_position.x;
    cinfo_s[client_to_update].paddle_position_s.y  = m->cinfo[client_to_get_info_from].paddle_position.y;
}

/*updates the information on the message from server data so client can have up to date information on all other clients and on the ball*/
void update_message_info(client_info_s  * cinfo_s, message  * m,int clients_online, ball_position_t ball_s){
    for (int i = 0; i < clients_online; i++)
    {
        m->cinfo[i].client_ID = cinfo_s[i].client_ID;
        m->cinfo[i].paddle_position.length =cinfo_s[i].paddle_position_s.length;
        m->cinfo[i].paddle_position.x =cinfo_s[i].paddle_position_s.x;
        m->cinfo[i].paddle_position.y =cinfo_s[i].paddle_position_s.y;

    }
    for (int i = 0; i < MAX_CLIENTS; i++){
        m->cinfo[i].score = cinfo_s[i].score;
    }
    copy_ball_info(&m->ball_position,&ball_s);
    
}

/*initializes the message for a new client making sure it to put all is data on the message corresponding position*/
void add_client(message  * m, struct client_info_s  * cinfo_s, int  clients_online , struct sockaddr_in client_addr,ball_position_t ball_s){
    static int ID=0;
    ID++;
    m->client_contacting = clients_online -1;  //initializes client_contacting
    m->cinfo[m->client_contacting].client_ID = ID; //puts the client  ID on the message
    m->cinfo[m->client_contacting].score=0;
    m->msg_type = 3; 
    m->point = TRUE;
    copy_ball_info(&m->ball_position,&ball_s);
    new_paddle(m,clients_online-1);
    update_client_info( cinfo_s, m,  clients_online-1, client_addr,  FALSE);
    update_message_info(cinfo_s, m, clients_online, ball_s);
        
   
}

//Removes clients and wipes its data from server & message
void remove_client(message  * m, struct client_info_s  * cinfo_s ,int  clients_online)
{
    struct sockaddr_in nothing;// an empty sock_addr_in, because it's not needed in the update_client_info
    //cicle to remove one client
    for (int j = m->client_contacting; j < clients_online; j++)
    {
        m->cinfo[j].score= m->cinfo[j+1].score;
        m->cinfo[j].client_ID = m->cinfo[j+1].client_ID;
        strcpy(cinfo_s[j].client_address_s , cinfo_s[j+1].client_address_s);
        cinfo_s[j].port = cinfo_s[j+1].port;
        update_client_info(cinfo_s,m,j,nothing,TRUE);
    }
    cinfo_s[clients_online].score = -1;
    cinfo_s[clients_online].client_ID = 0;
}

/*initializes scores on server and message*/
void inicialize_score(message *m ,client_info_s * cinfo_s)
{
    //initializes scores  = -1(to avoid and force condition to print)
    for(int i = 0; i < MAX_CLIENTS; i++){
        m->cinfo[i].score=-1;
        cinfo_s[i].score=-1;
    }
}

/*routine for first client */
void first_client_routine(message * m, ball_position_t * ball_s, client_info_s * cinfo_s){
    inicialize_score(m, cinfo_s); //inicializa todas as posições do score a 0
    place_ball_random( &m->ball_position, ball_s); // inicializa a posição da bola
    m->cinfo[0].paddle_position.x=WINDOW_SIZE/2;
    m->cinfo[0].paddle_position.y=WINDOW_SIZE -2;
}

int main()
{
    initialize_ncurses(); 
    //---------Var declaration-----------
    int sock_fd;
    int clients_online = 0;                 //counts online clients
    struct sockaddr_in local_addr;          //structure describing an internet socket address
    struct sockaddr_in client_addr;         
    client_info_s cinfo_s[MAX_CLIENTS];     //Structure that will save relevant info from message in server
    message m;                              //Message 
    ball_position_t ball_s;                 //Structure that will save current ball position in server
    
    int move_ball_counter =0;               //Var used to move the ball after n paddle_moves 
    
    create_socket(&sock_fd);

    /*-----------BIND SOCKET-----------*/
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(SOCK_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    socklen_t client_addr_size = sizeof(struct sockaddr_in);
    bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
    /*---------------------------------*/

   
    WINDOW *my_win = newwin(5, WINDOW_SIZE, 0, 0); //Declares game window
    create_window(my_win);

    while (1)
    {
        
        recvfrom(sock_fd, &m, sizeof(message), 0,
                 (struct sockaddr *)&client_addr, &client_addr_size);
        
      switch (m.msg_type)
        {
        case 0: //m.msg_type = 0 -> Connect
            if ( clients_online == MAX_CLIENTS) //If the server is full
            { 
                m.msg_type = 4; //mgs_type=4 -> the server informs the client that it is full and will close it's window.
            }
            else
            {   
                clients_online ++;  //If the server isn't full, increment the number of online clients.
                if (clients_online == 1) //If its the first client to connect 
                { 
                    first_client_routine(&m,&ball_s,cinfo_s); //Prefroms a "first client" routine that initializes variables
                }
                add_client(&m, cinfo_s, clients_online, client_addr, ball_s); //Gives client an ID, initializes its message values, 
                mvwprintw(my_win, 2, 1, "%d connected at the moment", clients_online);
                wrefresh(my_win);
                
            }
            //Sends Board_update message  (msg_type = 3)
            sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&client_addr, client_addr_size);

            break;
         case 1: //Disconnect message
            remove_client(&m,cinfo_s,clients_online); 
            clients_online --;
            mvwprintw(my_win, 2, 1, "%d connected at the moment", clients_online);
            wrefresh(my_win);
            break; 

        case 2: //Paddle_move
            for (int i =0; i< clients_online;i++){
                if (m.cinfo[m.client_contacting].client_ID==cinfo_s[i].client_ID)m.client_contacting=i;
            }
            paddle_move(&m.cinfo[m.client_contacting].paddle_position,&cinfo_s[m.client_contacting].paddle_position_s);
            m.msg_type = 3;
            move_ball_counter ++;
            if ((move_ball_counter == clients_online) ||m.point){
                move_ball(&m, cinfo_s, clients_online, &ball_s);
                move_ball_counter = 0;
            }
            //update_message_info(cinfo_s, &m, clients_online, ball_s);
            for (int i = 0; i < clients_online; i++)
            {
                m.cinfo[i].client_ID = cinfo_s[i].client_ID;
                m.cinfo[i].paddle_position.length =cinfo_s[i].paddle_position_s.length;
                m.cinfo[i].paddle_position.x =cinfo_s[i].paddle_position_s.x;
                m.cinfo[i].paddle_position.y =cinfo_s[i].paddle_position_s.y;

            }
            for (int i = 0; i < MAX_CLIENTS; i++){
                m.cinfo[i].score = cinfo_s[i].score;
            }
            copy_ball_info(&m.ball_position,&ball_s);
            sendto(sock_fd, &m, sizeof(m), 0, (const struct sockaddr *)&client_addr, client_addr_size);
            break;      
        }
    }
    close(sock_fd);
    endwin(); //End ncurses mode

    return 0;
}
