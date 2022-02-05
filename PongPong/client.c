#include "header.h"


WINDOW * my_win ;// main window for game
//WINDOW * max_player_win;//window displayed when max players are exceed 
WINDOW * message_win; // message window with infomation if in PLAY_STATE
WINDOW * score_win ; //Score window at the right side of the game with player scores
WINDOW * controls_and_info ; // message window with infomation if in PLAY_STATE

pthread_mutex_t mux_curses;

direction_t random_direction(){
    return  random()%4;
}
void safe_waddch(WINDOW* win, int pos_x, int pos_y, int ch){

    pthread_mutex_lock(&mux_curses);    
        wmove(win, pos_y, pos_x);
        waddch(win,ch);   
        wrefresh(win);
    pthread_mutex_unlock(&mux_curses);    

}


/* routine to delete and redraw ball*/
void movement_message( message * m) {
  pthread_mutex_lock(&mux_curses);
  wrefresh(my_win);
  pthread_mutex_unlock(&mux_curses);
  safe_waddch(my_win,m->ball_position.x,m->ball_position.y, 'o');
  pthread_mutex_lock(&mux_curses);
  waddch(my_win,' ');
  pthread_mutex_unlock(&mux_curses);
}


void safe_mvwprintw(WINDOW * win, int pos_y, int pos_x, char * str){
    pthread_mutex_lock(&mux_curses); 
    mvwprintw(win, pos_y,pos_x,str);
    wrefresh(win);
    pthread_mutex_unlock(&mux_curses);    
}

void move_paddle(paddle_position * paddle, int direction) {
  safe_mvwprintw(message_win, 2, 1, "                  ");
  if (direction == KEY_UP || direction == 'w') {
    if (paddle -> y != 1) {
      paddle -> y--;
      safe_mvwprintw(message_win, 2, 1, "UP movement\0");
    } else safe_mvwprintw(message_win, 2, 1, "Hitting BOX limit\0");
  }
  if (direction == KEY_DOWN || direction == 's') {
    if (paddle -> y != WINDOW_SIZE - 2) {
      paddle -> y++;
      safe_mvwprintw(message_win, 2, 1, "DOWN movement\0");
    } else safe_mvwprintw(message_win, 2, 1, "Hitting BOX limit\0");
  }
  if (direction == KEY_LEFT || direction == 'a') {
    if (paddle -> x - paddle -> length != 1) {
      paddle -> x--;
      safe_mvwprintw(message_win, 2, 1, "LEFT movement\0");
    } else safe_mvwprintw(message_win, 2, 1, "Hitting BOX limit\0");
  }
  if (direction == KEY_RIGHT || direction == 'd') {
    if (paddle -> x + paddle -> length != WINDOW_SIZE - 2) {
      paddle -> x++;
      safe_mvwprintw(message_win, 2, 1, "RIGHT movement\0");
    } else safe_mvwprintw(message_win, 2, 1, "Hitting BOX limit\0");
  }
}

void draw_paddle( paddle_position * paddle, int draw) {

  int ch;
  if (draw) {
    ch = '=';
  } else {
    ch = ' ';// to delete a paddle draw a ' ' over it and it will be overwritten, therefore deleted
  }
  int start_x = paddle -> x - paddle -> length;
  int end_x = paddle -> x + paddle -> length;
  for (int x = start_x; x <= end_x; x++) {// draws the ch on the  window making the paddle from the left edge to the right edge
  
    safe_waddch(my_win,x,paddle -> y, ch);
  }
}

void update_paddle( paddle_position * paddle, int key) {
  draw_paddle(paddle, FALSE); //deletes paddle
  move_paddle(paddle, key); //calculates new paddle position according to user input
  //if it does, changes direction of ball
  draw_paddle( paddle, TRUE);
} //delete previous ball, calculate new position and draws it 

void new_position(ball_position_t* ball){
    int next_x = ball->x +ball->left_ver_right;
    int next_y = ball->y + ball->up_hor_down;

    if (next_x == 0 || next_x == WINDOW_SIZE - 1) {
        ball->up_hor_down = rand() % 3 - 1;
        ball->left_ver_right *= -1;
    } else {
        ball -> x = next_x;
    }

  if (next_y == 0 || next_y == WINDOW_SIZE - 1) {
    ball -> up_hor_down *= -1;
    ball -> left_ver_right = rand() % 3 - 1;

  } else {
    ball -> y = next_y;
  }
}

void * move_ball_thread(void * arg){
    update_ball_t* update_ball = arg;
    

    

    //safe_waddch(my_win,update_ball->paddle->x,update_ball->paddle->y,'=');
    while (*update_ball->play_state)
    {
        sleep(1);
        /*deletes old place */
        safe_waddch(my_win, update_ball->m->ball_position.x, update_ball->m->ball_position.y, ' ');

        /* claculates new direction */
        new_position(&update_ball->m->ball_position);
       
        int condition_1 = (((update_ball->m->ball_position.x <= update_ball->paddle->x+ update_ball->paddle->length) && 
                            (update_ball->m->ball_position.x >= update_ball->paddle->x- update_ball->paddle->length)) );
        if (condition_1 ) {
            if(update_ball->m->ball_position.y == update_ball->paddle->y){
                update_ball->scores[0] ++;
                update_ball->m->point =TRUE;
                if( update_ball->m->ball_position.x == WINDOW_SIZE -2 || update_ball->m->ball_position.x == 1){//left or right
                    if(update_ball->m->ball_position.y <= WINDOW_SIZE/2)update_ball->m->ball_position.up_hor_down=1;
                    else  update_ball->m->ball_position.up_hor_down=-1;
                    if (update_ball->m->ball_position.x == WINDOW_SIZE -2 ) update_ball->m->ball_position.left_ver_right =1;
                    else update_ball->m->ball_position.left_ver_right=-1;
                } 
                else  update_ball->m->ball_position.up_hor_down*=-1;
                update_ball->m->ball_position.y +=  update_ball->m->ball_position.up_hor_down;
                update_ball->m->ball_position.x += update_ball->m->ball_position.left_ver_right;
                }
                if ((update_ball->paddle->y == WINDOW_SIZE -3 )&& (update_ball->m->ball_position.y== WINDOW_SIZE -2)){
                update_ball->m->ball_position.y = WINDOW_SIZE -4;
                 update_ball->m->ball_position.up_hor_down = -1;
                }
                else if ((update_ball->paddle->y== 2) && (update_ball->m->ball_position.y == 1)){
                update_ball->m->ball_position.y = 3;
                 update_ball->m->ball_position.up_hor_down = 1;
                } 
            }  
        else if (update_ball->m->ball_position.y == update_ball->paddle->y && !( update_ball->m->ball_position.up_hor_down== 0))update_ball->scores[1]++;
        
        /* draw mark on new position */
        safe_waddch(my_win, update_ball->m->ball_position.x, update_ball->m->ball_position.y, update_ball->m->ball_position.c| A_BOLD);
       // safe_mvwprintw(message_win,2,2*n," l");
       
        pthread_mutex_lock(&mux_curses); 
         mvwprintw(message_win, 3, 1, "SCORED - %d \t|\tMISSED - %d", update_ball->scores[0], update_ball->scores[1]);
        wrefresh(message_win);
        pthread_mutex_unlock(&mux_curses); 
        if (update_ball->m->msg_type==4){
            pthread_exit(NULL);
        }
        
    }
   return NULL;

}

void * communication_thread(void *arg){
   
  communication_t* communication = (communication_t*) arg;
  struct sockaddr_in server_addr;
  pthread_t thread;
  bool start_move = TRUE;
  //socklen_t size_addr;
	int sock_fd= socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SOCK_PORT);
  if (inet_pton( AF_INET,communication->adress_keyboard, & server_addr.sin_addr) < 1) {
    //inet_pton - convert IPv4 and IPv6 addresses from text to binary form
    printf("no valid address: \n");
    exit(-1);
  }

   communication->m->msg_type = 0; //connect;
  sendto(sock_fd, communication->m, sizeof(message), 0,
    (const struct sockaddr * ) & server_addr, sizeof(server_addr)); //send the connection message

    while(1){
      recv(sock_fd, communication->m, sizeof(message), 0);
      
      if (communication->m->msg_type == 5) {
        safe_mvwprintw(message_win, 1, 1, "MAX PLAYER NUMBER EXCEEDED, PROCESS WILL BE KILLED IN 5s");
        sleep(5);
        endwin();			/* End curses mode		  */
        //pthread_exit(NULL);
        exit(0);
      break;
    }
      if(communication->m->msg_type==2){
        *communication->play_state =TRUE;
        if(start_move){
          pthread_create(&thread, NULL, move_ball_thread, (void *) (communication->update_ball_thread));
          start_move =FALSE;
        }
      }
      else(start_move =TRUE);
      if(communication->m->msg_type==3)movement_message(communication->m);
      if (*communication->key == 'q')pthread_exit(NULL);
      if(*communication->move_ball == TRUE){
        communication->m->msg_type = 3;
        sendto(sock_fd, &communication->m, sizeof(message), 0,
            (const struct sockaddr * ) & server_addr, sizeof(server_addr)); //send the connection message
      }
      //sleep(0.1);
    }

    return NULL;
}
/*initializes ncurses*/
void initialize_ncurses() {
  initscr();
  cbreak();
  keypad(my_win, TRUE);
  noecho();
  curs_set(0);
}

// if released clears message window 
void clear_paddle_nd_msg_window(paddle_position * paddle) {
  draw_paddle(paddle, FALSE);
  pthread_mutex_lock(&mux_curses);
  mvwprintw(message_win, 1, 1, "                                        \0");
  mvwprintw(message_win, 2, 1, "                                        \0");
  mvwprintw(message_win, 3, 1, "                                        \0");
  mvwprintw(message_win, 4, 1, "                                        \0");
  pthread_mutex_unlock(&mux_curses);
}

/* creates the 4  used windows and draws a border for each  each window has it's own purpose, see declaration*/
void create_windows() {
  pthread_mutex_lock(&mux_curses); 
  box(my_win, 0, 0);
  wrefresh(my_win);
  keypad(my_win, true);
  box(message_win, 0, 0);
  wrefresh(message_win);
  box(score_win, 0, 0);
  mvwprintw(score_win, 1, 2, "SCOREBOARD");
  wrefresh(score_win);
  box(controls_and_info, 0, 0);
  mvwprintw(controls_and_info, 1, 1, "IF ON PLAY STATE: ");
  mvwprintw(controls_and_info, 2, 1, "Use the arrows or wasd to control the paddle\0");
  mvwprintw(controls_and_info, 3, 1, "Press \tq to leave the game  ");
  mvwprintw(controls_and_info, 4, 1, "Press \tr to release the ball after at least 10 secs ");
  wrefresh(controls_and_info);
  pthread_mutex_unlock(&mux_curses); 
}
/*updates the score board being printed in the score_win (score_win)on the right side of the main win(my_win*/
void update_scoreboard(int score[]) {
  int temp_score[MAX_CLIENTS], temp_max = 0, k = 0, clt_on_score = -1;
  for (int j = 0; j < MAX_CLIENTS; j++) {
    temp_score[j] = -1;
    safe_mvwprintw(score_win, j + 2, 1, "                      \0");
    if (score[clt_on_score + 1] != -1) {
      ++clt_on_score;
      temp_score[clt_on_score] = score[clt_on_score];
    }
  }
  for (int l = 0; l <= clt_on_score; l++) {
    for (int j = clt_on_score; j >= 0; j--) {
      if (temp_score[j] >= temp_max) {
        temp_max = temp_score[j];
        k = j;
      }
    }
    pthread_mutex_lock(&mux_curses);   
    mvwprintw(score_win, l + 2, 1, " %dº | PLAYER %d |%d\0 ", l + 1, k, temp_score[k]);
    wrefresh(score_win);
    pthread_mutex_unlock(&mux_curses);
    temp_score[k] = -1;
    temp_max = 0;
  }
  

}

void new_paddle(paddle_position * paddle, int length) {
  paddle -> x = WINDOW_SIZE / 2;
  paddle -> y = WINDOW_SIZE - 2;
  paddle -> length = length;
}

/*routine to start playe state*/
void start_play_state( paddle_position * paddle, message * m) {
  
  draw_paddle(paddle, TRUE); // draws paddle in the defined position (mode TRUE=draw/ mode FALSE= delete)
  safe_waddch(my_win,m->ball_position.x,m->ball_position.y, 'o');
  safe_mvwprintw(message_win, 1, 1, "PLAY STATE");
  m -> point = FALSE;
  //m -> msg_type = 3;
}

int main(int argc, char * argv[])
{	
   if (argc < 2) {
    printf("to run client supply the server adress after ./CLIENT.proj \n");
    exit(0);
  }


  message m;
  m.msg_type =2;
  /*place ball random
  m.ball_position.c='o';
  m.ball_position.x = rand() % WINDOW_SIZE;;
  m.ball_position.y =  rand() % WINDOW_SIZE;;
  m.ball_position.left_ver_right = rand() % 3 - 1;
  
  m.ball_position.up_hor_down = rand() % 3 - 1;
  place ball random*/
  int scores[2];
  scores[0]= 0;
  scores[1]=0;
  paddle_position paddle;
  new_paddle( & paddle, PADLE_SIZE); // initializes all paddle arguments  
  bool test = FALSE;
  bool move_ball;
  int key =0;
  /*initializing update ball  arguments struct*/
  update_ball_t update_ball_thread;
  update_ball_thread.m = &m;
  update_ball_thread.paddle = &paddle;
  update_ball_thread.play_state = &test;
  update_ball_thread.paddle =&paddle;
  update_ball_thread.scores = scores;
  update_ball_thread.move_ball = &move_ball;
  /*initializing update ball arguments struct*/
  initialize_ncurses();
  pthread_mutex_init(&mux_curses, NULL);
  /* creates a window and draws a border */
  my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0); // main window for game
  //max_player_win = newwin(4, WINDOW_SIZE, 0, 0); //window displayed when max players are exceed 
  message_win = newwin(5, WINDOW_SIZE + 25, WINDOW_SIZE, 0); // message window with infomation if in PLAY_STATE
  score_win = newwin(WINDOW_SIZE, 24, 0, WINDOW_SIZE + 1); //Score window at the right side of the game with player scores
  controls_and_info = newwin(6, WINDOW_SIZE + 25, WINDOW_SIZE + 5, 0); // message window with infomation if in PLAY_STATE
  create_windows();
    
  safe_waddch(my_win, m.ball_position.x, m.ball_position.y, m.ball_position.c|A_BOLD);
  pthread_t server_t_id;
  communication_t communication;
  communication.key= &key;
  communication.m = &m;
  communication.move_ball =&move_ball;
  communication.adress_keyboard = argv[1];
  communication.paddle = &paddle;
  communication.play_state = &test;
  communication.update_ball_thread = &update_ball_thread;
  pthread_create(&server_t_id, NULL, communication_thread, (void*)&communication);
  
  
  
  int n =0 ; 
  char message[100];
  do
  {
    key = wgetch(my_win);
      n++;
      int condition = (key == KEY_LEFT || KEY_RIGHT || KEY_UP || KEY_DOWN || 'a' || 's' || 'd' || 'w');
      if (condition&& (m.msg_type== 2)) update_paddle( & paddle, key);
      safe_mvwprintw(message_win, 1,1,message);
      if (m.point )update_scoreboard(m.score);
  }while(key != 'q');
  m.msg_type=4;
  endwin();			/* End curses mode		  */
  //pthread_exit(NULL);
  exit(0);
	return 0;
}