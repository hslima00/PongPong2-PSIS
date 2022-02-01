#include "header.h"

/* creates the socket to with the correct type (AF_INET) use and checks if the creation was sucessful */ 
void criar_socket(int *sock_fd){
    *sock_fd = socket(AF_INET, SOCK_DGRAM, 0);   //creates the socket
    if (*sock_fd == -1){                         //checks if the creation was sucessful
	    perror("socket: ");   
	    exit(-1);
    }

}

/*initializes ncurses*/
void initialize_ncurses(){
    initscr();		    	
	cbreak();				
    keypad(stdscr, TRUE);   
	noecho();
    curs_set(0);
}

/* creates the 4  used windows and draws a border for each  each window has it's own purpose, see declaration*/
void create_windows(WINDOW * my_win , WINDOW * message_win , WINDOW * score_win, WINDOW * controls_and_info){   
    box(my_win, 0 , 0);	
	wrefresh(my_win);
    keypad(my_win, true);
    box(message_win, 0 , 0);	
	wrefresh(message_win);
    box(score_win, 0 , 0);	
    mvwprintw(score_win, 1 , 2 ,"SCOREBOARD");
    wrefresh(score_win);
    box(controls_and_info, 0 , 0);	
    mvwprintw(controls_and_info, 2 , 1 ,"Use the arrows or wasd to control the paddle\0");
    mvwprintw(controls_and_info, 3 , 1 ,"Press \tq to leave the game  ");
    wrefresh(controls_and_info);
}

/*draws or deletes the paddle given in the main win(my_win)  */
void draw_paddle(WINDOW *my_win, paddle_position * paddle, bool draw, bool this_client){
    int ch;
    if(draw){// if we want to draw assign the correct char to make so
        if (this_client)ch = '=' ;// if the client is drawing it's own paddle use '=' instead of '_'
        else ch = '_';
    }else{
        ch = ' ';// to delete a paddle draw a ' ' over it and it will be overwritten, therefore deleted
    }
    int start_x = paddle->x - paddle->length;
    int end_x = paddle->x + paddle->length;
    for (int x = start_x; x <= end_x; x++){// draws the ch on the  window making the paddle from the left edge to the right edge
        wmove(my_win, paddle->y, x);
        waddch(my_win,ch);
    }
    wrefresh(my_win);
}

/* moves the paddle checking if the move is valid(not moving to a already ocuppied position) and if the paddle hits the ball*/
void move_paddle (message * m, int direction, WINDOW *message_win ){
    mvwprintw(message_win, 3,1,"                        ");
    wrefresh(message_win);
    bool invalid_move = FALSE;// control variable to check if valid move
    bool condition_1=FALSE, condition_2=FALSE;//control variables to check conditions for valid move
    paddle_position simul_paddle;// simulated ball to assess if the move is valid
    simul_paddle.x= m->cinfo[m->client_contacting].paddle_position.x;
    simul_paddle.y= m->cinfo[m->client_contacting].paddle_position.y;
    if (direction == KEY_UP || direction == 'w'){
        if (simul_paddle.y  != 1){
            simul_paddle.y --;
            mvwprintw(message_win, 3,1,"UP movement\0");
        }
        else mvwprintw(message_win, 3,1,"Hitting BOX limit\0"); 
    }
    if (direction == KEY_DOWN || direction == 's'){
        if (simul_paddle.y != WINDOW_SIZE-2){
            simul_paddle.y ++;
            mvwprintw(message_win, 3,1,"DOWN movement\0");
        }
        else mvwprintw(message_win, 3,1,"Hitting BOX limit\0"); 
    }
    if (direction == KEY_LEFT || direction == 'a'){
        if (simul_paddle.x - PADDLE_SIZE  != 1){
            simul_paddle.x--;
            mvwprintw(message_win, 3,1,"LEFT movement\0");
        }
        else mvwprintw(message_win, 3,1,"Hitting BOX limit\0"); 
    }
    if (direction == KEY_RIGHT || direction == 'd'){
        if (simul_paddle.x + PADDLE_SIZE != WINDOW_SIZE-2){
            simul_paddle.x++;
            mvwprintw(message_win, 3,1,"RIGHT movement\0");
        }
        else mvwprintw(message_win, 3,1,"Hitting BOX limit\0"); 
    }
    for (int i = 0; i < MAX_CLIENTS; i++)
    {   
        if(m->cinfo[i].score == -1)break;// if score == -1, then this client is yet to be created(meaning we already checked for all)
        else if(i == m->client_contacting) continue;// skip check for this client, given it's the one being simulated
        else if (simul_paddle.y == m->cinfo[i].paddle_position.y){// if on the same y (height)
            condition_1 = ((simul_paddle.x+ PADDLE_SIZE<=m->cinfo[i].paddle_position.x + PADDLE_SIZE )&& //right edge simul_paddle <= right edge paddle[i] AND
                    (simul_paddle.x +PADDLE_SIZE  >=m->cinfo[i].paddle_position.x - PADDLE_SIZE));//right edge simul_paddle >= left edge paddle[i]
            
            condition_2 =((simul_paddle.x - PADDLE_SIZE<=m->cinfo[i].paddle_position.x + PADDLE_SIZE ) &&//left edge  simul_paddle <= right edgepaddle[i] AND
                        (simul_paddle.x -PADDLE_SIZE  >=m->cinfo[i].paddle_position.x - PADDLE_SIZE));//left edge  simul_paddle >= left edge  paddle[i]
            
            invalid_move = ((condition_1  || condition_2) );  // if on the same height and touching paddle
        }	
        if (invalid_move)break;
    }
    
    if (!invalid_move){// if valid move saves simul paddle onto the real paddle
        m->cinfo[m->client_contacting].paddle_position.x = simul_paddle.x;
        m->cinfo[m->client_contacting].paddle_position.y = simul_paddle.y;
    }
    else  {
        mvwprintw(message_win, 3,1,"                        ");
        mvwprintw(message_win, 3,1,"Hitting another paddle\0");
    }
    m->point = ((( (m->ball_position.x <= m->cinfo[m->client_contacting].paddle_position.x + PADDLE_SIZE) &&       //paddle right edge 
            (m->ball_position.x >= m->cinfo[m->client_contacting].paddle_position.x - PADDLE_SIZE)) ) &&          //paddle left edge 
            (m->cinfo[m->client_contacting].paddle_position.y == m->ball_position.y));                           //if between both edges and on the same y  == hitting ball

    if ((m->point) && (m->ball_position.up_hor_down== 0)){// if hitting ball and ball is horizontal
        if (direction == KEY_DOWN || direction == 's') m->ball_position.up_hor_down = 1;//if moved up forces ball to go up
        else if (direction == KEY_UP || direction == 'w')m->ball_position.up_hor_down = -1;//if  moved down forces ball to go down
    }
    wrefresh(message_win);
}

/*draws or deletes the ball given in the main win(my_win)  */
void draw_ball(WINDOW *my_win, ball_position_t * ball, bool draw){
    int ch;
    if(draw){// to draw make ch ='o' else ch = ' '
        ch = 'o';
    }else{// to delete a paddle draw a ' ' over it and it will be overwritten, therefore deleted
        ch = ' ';
    }
    wmove(my_win, ball->y, ball->x);
    waddch(my_win,ch);
    wrefresh(my_win);
}

/*updates the score board being printed in the score_win (score_win)on the right side of the main win(my_win*/
void update_scoreboard(client_info_t * cinfo, WINDOW * score_win, int client_contacting){
    int temp_score[MAX_CLIENTS][2], temp_max = 0, k = 0 , clt_on_score = -1;// variables
    for (int j = 0; j < MAX_CLIENTS; j++){
        temp_score[j][1] = -1;
        mvwprintw(score_win, j+2 , 1 ,"                      \0"); //deletes the line
        wrefresh(score_win);
        if(cinfo[clt_on_score +1 ].score!= -1){// if score == -1 then client is yet to create
            ++ clt_on_score;
            temp_score[clt_on_score][0] = cinfo[clt_on_score].score; // saves the client score to the temp_score_[clt_on_score][0]
            temp_score[clt_on_score][1] = cinfo[clt_on_score].client_ID;// saves the client client_ID to the temp_score_[clt_on_score][1]
        } 
    }

    for (int l = 0; l <= clt_on_score; l++){
        for (int j = clt_on_score ;j >= 0; j--){// checks for top 1 temp_score score value 
            if (temp_score[j][0] >= temp_max ) {
                temp_max = temp_score[j][0];
                k = j;
            }
        } 

        /* if this is this client score adds a --> before the score*/
        if(temp_score[k][1] == cinfo[client_contacting].client_ID)mvwprintw(score_win, l+2 , 1 ,"-->%dº | PLAYER %d |%d\0 ", l+1 , temp_score[k][1] ,temp_score[k][0]);
        else mvwprintw(score_win, l+2 , 3 ," %dº | PLAYER %d |%d\0 ", l+1 , temp_score[k][1] ,temp_score[k][0]);// prints the scores in a position | player id | score form
        wrefresh(score_win);
        temp_score[k][0] = -1;// forces temp_score [k] to -1 making sure it won't be bigger than temp_max
        temp_max = 0;// resets temp_max to 0
    } 
    
}

/*update the paddle position using paddle related function, to do so deletes the paddle moves it and then redraws it(updating the paddle) information*/
void update_paddle(WINDOW *my_win, message * m, paddle_position * paddle, int key,WINDOW *message_win){
    draw_paddle(my_win,paddle,FALSE,TRUE);// deletes the paddle
    move_paddle (m, key, message_win);// moves the paddle
    paddle->x =m->cinfo[m->client_contacting].paddle_position.x;// updates paddle position saved on client so it can be properly deleted afterwards
    paddle->y =m->cinfo[m->client_contacting].paddle_position.y; 
    draw_paddle(my_win,paddle,TRUE,TRUE);// draws paddle in the new position
     
}

/*copies ball information from um ball_position_t to another*/
void copy_ball_info(ball_position_t* ball_to_overwrite, ball_position_t *ball_to_copy_from){
    ball_to_overwrite->c= ball_to_copy_from->c;
    ball_to_overwrite->left_ver_right= ball_to_copy_from->left_ver_right;
    ball_to_overwrite->up_hor_down= ball_to_copy_from->up_hor_down;
    ball_to_overwrite->x= ball_to_copy_from->x;
    ball_to_overwrite->y= ball_to_copy_from->y;
}

/*deletes all previous paddles and the previous ball and redraws everything after updating their position*/
void update__all_paddles_and_ball(WINDOW *my_win, message * m, paddle_position * paddles, bool start,ball_position_t *previous_ball){
    if(!start)draw_ball(my_win, previous_ball,FALSE);// deletes the ball unless it's the first call 
    copy_ball_info(previous_ball,&m->ball_position);// updates ball info on the client
    draw_ball(my_win, previous_ball,TRUE);// draws ball in new position
    for (int j = 0; j < MAX_CLIENTS; j++){// runs for all paddles
        if(m->cinfo[j].score!= -1){
            if(!start){// makes sure that if it's the first call drawing the padles not to delete any because the information on them would be trash
                draw_paddle(my_win , &paddles[j], FALSE,FALSE);// deletes paddle
            }
            paddles[j].length = PADDLE_SIZE;//updates paddle info on the client
            paddles[j].x= m->cinfo[j].paddle_position.x;//updates paddle info on the client
            paddles[j].y= m->cinfo[j].paddle_position.y;//updates paddle info on the client
            if (j == m->client_contacting)draw_paddle(my_win , &m->cinfo[j].paddle_position, TRUE, TRUE);// if paddle is from this client makes sure it draws '=' instead of '_'
            else draw_paddle(my_win , &m->cinfo[j].paddle_position, TRUE, FALSE);// draws paddle on new position
        }
        else break;
    }
    
}

/*routine that writes PLAY STATE  and makes calls the function to draw all paddles making sure it knows it's the first call*/
void start_play_state(WINDOW * my_win,  message * m ,WINDOW * message_win, paddle_position *paddles, ball_position_t * previous_ball){
    mvwprintw(message_win, 1,1,"PLAY STATE");// prints on message_win PLAY STATE
    update__all_paddles_and_ball(my_win,m,paddles,TRUE,previous_ball);// draws all paddles
    wrefresh(message_win);
}

/*routine to update board, calls update scoreboard and update_all_paddles_and_balls making sure it knows it's not the first call*/
void update_board(message *m, WINDOW * score_win, WINDOW * my_win,paddle_position *paddles,ball_position_t *previous_ball ){
    update_scoreboard(m->cinfo, score_win,m->client_contacting);// updates scoreboard
    update__all_paddles_and_ball(my_win,m,paddles,FALSE,previous_ball);// deletes previous paddles and balls and redraws them in new position updating the previous positions to the new ones
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("to run client supply the server adress after ./CLIENT.proj \n");
        exit(0);
    } 
    //Var declaration
    char *adress_keyboard= argv[1];// gets address from the function call as an argument
    int sock_fd , condition, key = -1;      
    message m;
    paddle_position paddles[MAX_CLIENTS];
    ball_position_t previous_ball;
    struct sockaddr_in server_addr;
    bool sucess_connect = TRUE;
    criar_socket(&sock_fd);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SOCK_PORT);
    if( inet_pton(AF_INET, adress_keyboard, &server_addr.sin_addr) < 1){
        //inet_pton - convert IPv4 and IPv6 addresses from text to binary form
		printf("no valid address: \n");
		exit(-1);
	}
    
    // send connection message
    m.msg_type = 0;  //connect;
    sendto(sock_fd, &m, sizeof(message), 0,
          (const struct sockaddr *)&server_addr, sizeof(server_addr)); //send the connection message to the server
    
    initialize_ncurses();
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);// main window for game
    WINDOW * max_player_win = newwin(4, WINDOW_SIZE, 0, 0);//window displayed when max players are exceed 
    WINDOW * message_win = newwin(5, WINDOW_SIZE + 25, WINDOW_SIZE, 0);// message window with infomation if in PLAY_STATE
    WINDOW * score_win = newwin(WINDOW_SIZE , 24 , 0 , WINDOW_SIZE+1);//Score window at the right side of the game with player scores
    WINDOW * controls_and_info= newwin(5, WINDOW_SIZE + 25, WINDOW_SIZE+5, 0);// message window with infomation if in PLAY_STATE
 
    
    do{
        recv(sock_fd, &m, sizeof(message), 0);// recieve message from the server
        if(m.msg_type==4){// if message is server is full  then prints a message and closes in 5 seconds
            mvwprintw(max_player_win, 1,1,"MAX PLAYER NUMBER EXCEEDED, PROCESS WILL BE KILLED IN 5s");
            wrefresh(max_player_win);
            sleep(5);
            break;
        }
        else if (sucess_connect){ // if first message creates the windows, starts play_state and updates the score_board
            create_windows(my_win,message_win, score_win, controls_and_info);
            start_play_state(my_win, &m, message_win, paddles, &previous_ball);
            update_scoreboard(m.cinfo, score_win, m.client_contacting);
            sucess_connect = FALSE;// closes the condition forever after first message
            }
        else update_board(&m, score_win, my_win,paddles,&previous_ball);// if not full and not first message updates the board
        m.msg_type = 2;
        do{  
            key = wgetch(my_win);// gets key from user
            condition = (key == KEY_LEFT ||  KEY_RIGHT || KEY_UP || KEY_DOWN ||  'a'|| 's' ||'d' ||  'w');
            if(key == 'q'){
                m.msg_type = 1; // disconnect message
                sendto(sock_fd, &m, sizeof(message), 0,
                    (const struct sockaddr *)&server_addr, sizeof(server_addr));
                close(sock_fd);// closes the socket
                endwin();// End curses mode	
                return 0;
            }
            else if(condition) update_paddle(my_win, &m,&paddles[m.client_contacting], key, message_win);
        }while(!condition);// while key is not one of the specified ones gets another key             
        m.msg_type = 2;
        sendto(sock_fd, &m, sizeof(message), 0,
                (const struct sockaddr *)&server_addr, sizeof(server_addr)); //send the move ball message
        wrefresh(message_win);
    }while(key !=27);
   
    close(sock_fd);// closes the socket
    endwin();// End curses mode	
    return 0;    
}