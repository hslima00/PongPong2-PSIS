#include "header.h"

pthread_mutex_t mux_curses;
/*thread safety */
void safe_waddch(WINDOW * win, int pos_x, int pos_y, int ch){

    pthread_mutex_lock(&mux_curses);    
        wmove(win, pos_y, pos_x);
        waddch(win,ch);   
        wrefresh(win);
    pthread_mutex_unlock(&mux_curses);    

}


void safe_mvwprintw(WINDOW * win, int pos_y, int pos_x, char * str){
    pthread_mutex_lock(&mux_curses); 
    mvwprintw(win, pos_y,pos_x,str);
    wrefresh(win);
    pthread_mutex_unlock(&mux_curses);    
}

/*thread safety*/

/* creates the socket to with the correct type (AF_INET) use and checks if the creation was sucessful */ 
void criar_socket(int * sock_fd) {
  * sock_fd = socket(AF_INET, SOCK_DGRAM, 0); //creates the socket
  if ( * sock_fd == -1) { //checks if the creation was sucessful
    perror("socket: ");
    exit(-1);
    
  }

}

/*initializes ncurses*/
void initialize_ncurses() {
  initscr();
  cbreak();
  keypad(stdscr, TRUE);
  noecho();
  curs_set(0);
}

/* creates the 4  used windows and draws a border for each  each window has it's own purpose, see declaration*/
void create_windows(WINDOW * my_win, WINDOW * message_win, WINDOW * score_win, WINDOW * controls_and_info) {
  
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
   
}

/*initial paddle position*/
void new_paddle(paddle_position * paddle, int length) {
  paddle -> x = WINDOW_SIZE / 2;
  paddle -> y = WINDOW_SIZE - 2;
  paddle -> length = length;
}

/*draws or deletes the paddle given in the main my_win(my_win)  */
void draw_paddle(WINDOW * my_win, paddle_position * paddle, int draw) {

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

/* moves the paddle checking if the move  is valid */
void move_paddle(paddle_position * paddle, int direction, WINDOW * message_win) {
  safe_mvwprintw(message_win, 3, 1, "                        ");
  if (direction == KEY_UP || direction == 'w') {
    if (paddle -> y != 1) {
      paddle -> y--;
      safe_mvwprintw(message_win, 3, 1, "UP movement\0");
    } else safe_mvwprintw(message_win, 3, 1, "Hitting BOX limit\0");
  }
  if (direction == KEY_DOWN || direction == 's') {
    if (paddle -> y != WINDOW_SIZE - 2) {
      paddle -> y++;
      safe_mvwprintw(message_win, 3, 1, "DOWN movement\0");
    } else safe_mvwprintw(message_win, 3, 1, "Hitting BOX limit\0");
  }
  if (direction == KEY_LEFT || direction == 'a') {
    if (paddle -> x - paddle -> length != 1) {
      paddle -> x--;
      safe_mvwprintw(message_win, 3, 1, "LEFT movement\0");
    } else safe_mvwprintw(message_win, 3, 1, "Hitting BOX limit\0");
  }
  if (direction == KEY_RIGHT || direction == 'd') {
    if (paddle -> x + paddle -> length != WINDOW_SIZE - 2) {
      paddle -> x++;
      safe_mvwprintw(message_win, 3, 1, "RIGHT movement\0");
    } else safe_mvwprintw(message_win, 3, 1, "Hitting BOX limit\0");
  }
}

/*moves the ball   simulating bouncing effect on walls*/
void move_ball(ball_position_t * ball) {

  int next_x = ball -> x + ball -> left_ver_right;
  if (next_x == 0 || next_x == WINDOW_SIZE - 1) {
    ball -> up_hor_down = rand() % 3 - 1;
    ball -> left_ver_right *= -1;
  } else {
    ball -> x = next_x;
  }

  int next_y = ball -> y + ball -> up_hor_down;
  if (next_y == 0 || next_y == WINDOW_SIZE - 1) {
    ball -> up_hor_down *= -1;
    ball -> left_ver_right = rand() % 3 - 1;

  } else {
    ball -> y = next_y;
  }
}

/*draws or deletes the ball*/
void draw_ball(WINDOW * my_win, ball_position_t * ball, int draw) {
  int ch;
  if (draw) {
    ch = ball -> c;
  } else {
    ch = ' ';
  }
  safe_waddch(my_win,ball->x ,ball->y,ch);
}
 /* simulates ball position to check if hitting edges or paddle and bouncing accordingly */
void simulate_ball_position(paddle_position * paddle, message * m, WINDOW * message_win, int * scores) {
  ball_position_t* simul_ball;
  simul_ball =  malloc (sizeof (ball_position_t));
  simul_ball->c = m -> ball_position.c;
  simul_ball->left_ver_right = m -> ball_position.left_ver_right;
  simul_ball->up_hor_down = m -> ball_position.up_hor_down;
  simul_ball->x = m -> ball_position.x;
  simul_ball->y = m -> ball_position.y;
  move_ball(  simul_ball);

  int condition_1 = (((simul_ball->x <= paddle -> x + paddle -> length) && (simul_ball->x >= paddle -> x - paddle -> length)) );
  if (condition_1 ) {
    if(paddle -> y == simul_ball->y){
      scores[0]++;
      m->point =TRUE;
      if( simul_ball->x == WINDOW_SIZE -2 || simul_ball->x == 1){//left or right
        if(simul_ball->y <= WINDOW_SIZE/2)m->ball_position.up_hor_down=1;
        else m->ball_position.up_hor_down=-1;
        if (simul_ball->x == WINDOW_SIZE -2 ) m->ball_position.left_ver_right =1;
        else m->ball_position.left_ver_right =-1;
      } 
      else  m->ball_position.left_ver_right *=-1;
      m->ball_position.y += m-> ball_position.up_hor_down;
      m->ball_position.x += m-> ball_position.left_ver_right;
    }
    if ((paddle -> y == WINDOW_SIZE -3 )&& (simul_ball->y == WINDOW_SIZE -2)){
      m -> ball_position.y = WINDOW_SIZE -4;
      m -> ball_position.up_hor_down = -1;
    }
    else if ((paddle -> y == 2) && (simul_ball->y == 1)){
      m -> ball_position.y = 3;
      m -> ball_position.up_hor_down = 1;
    } 
    
  }  
  else if (simul_ball->y == paddle -> y && !(simul_ball->up_hor_down == 0)) scores[1]++;
 
  pthread_mutex_lock(&mux_curses);   
  mvwprintw(message_win, 2, 1, "SCORED - %d \t|\tMISSED - %d", scores[0], scores[1]);
  wrefresh(message_win);
  pthread_mutex_unlock(&mux_curses);
  free(simul_ball);
}
/*updates the score board being printed in the score_win (score_win)on the right side of the main win(my_win*/
void update_scoreboard(int score[], WINDOW * score_win) {
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

/*updates ball and paddle position*/
void update_paddle(WINDOW * my_win, paddle_position * paddle, int key, WINDOW * message_win) {
  draw_paddle(my_win, paddle, FALSE); //deletes paddle
  move_paddle(paddle, key, message_win); //calculates new paddle position according to user input
  //if it does, changes direction of ball
  draw_paddle(my_win, paddle, TRUE);
  pthread_mutex_lock(&mux_curses);
  wrefresh(my_win);
  wrefresh(message_win);
  pthread_mutex_unlock(&mux_curses);
} //delete previous ball, calculate new position and draws it 

void * update_ball_thread(void* arg){
  update_ball_t *update_ball_struct = (update_ball_t*) arg;
  
  //update_ball_t *update_ball_struct = malloc(sizeof(*update_ball_t));
  // malloc
  here:
  sleep(1);
  if (update_ball_struct->play_state == TRUE){ 
    draw_ball(update_ball_struct->my_win, &update_ball_struct->m->ball_position, FALSE); //deletes ball   
    simulate_ball_position(update_ball_struct->paddle, update_ball_struct->m, update_ball_struct->message_win, update_ball_struct->scores); //simulates if ball will hit paddle or not,
    move_ball( &update_ball_struct->m->ball_position);
    draw_ball(update_ball_struct->my_win, &update_ball_struct->m->ball_position, TRUE);
  }
  else {goto here;}
  return NULL;
}

void * quit_func(void* arg){
  quit_t *quit_struct = (quit_t *) arg;
  //malloc 
  int key;
  pthread_mutex_lock(&mux_curses);
  while((key = mvwgetch((quit_struct->my_win),1,1)) == ERR){//mudei to test
    if(key == 'q'){ 
      pthread_mutex_unlock(&mux_curses);
      quit_struct->m->msg_type = 4; // disconnect message
      sendto(*quit_struct->sock_fd, & quit_struct->m , sizeof(message), 0,
          (const struct sockaddr * ) & quit_struct->server_addr,  sizeof(quit_struct->server_addr)); //send the move ball message
      close(*quit_struct->sock_fd);
      endwin(); // End curses mode
      exit(0);
    }
    else {
      *quit_struct->key= key;
      pthread_mutex_unlock(&mux_curses);
      sleep(0.1);}
  }
  return NULL;
}

// if released clears message window 
void clear_paddle_nd_msg_window(WINDOW * my_win, paddle_position * paddle, WINDOW * message_win) {
  
  draw_paddle(my_win, paddle, FALSE);
  pthread_mutex_lock(&mux_curses);
  mvwprintw(message_win, 1, 1, "                                        \0");
  mvwprintw(message_win, 2, 1, "                                        \0");
  mvwprintw(message_win, 3, 1, "                                        \0");
  mvwprintw(message_win, 4, 1, "                                        \0");
  pthread_mutex_unlock(&mux_curses);
}

/*routine to start playe state*/
void start_play_state(WINDOW * my_win, paddle_position * paddle, message * m, WINDOW * message_win) {
  
  draw_paddle(my_win, paddle, TRUE); // draws paddle in the defined position (mode TRUE=draw/ mode FALSE= delete)
  draw_ball(my_win, & m -> ball_position, TRUE);
  pthread_mutex_lock(&mux_curses);
  mvwprintw(message_win, 1, 1, "PLAY STATE");
  wrefresh(my_win);
  wrefresh(message_win);
  pthread_mutex_unlock(&mux_curses);
  m -> point = FALSE;
  //m -> msg_type = 3;
}

/*routine for success connection*/
void connect_message(WINDOW * message_win) {
  pthread_mutex_lock(&mux_curses);
  mvwprintw(message_win, 1, 1, "SUCESS IN CONNECTION WAIT \0");
  wrefresh(message_win);
  mvwprintw(message_win, 1, 1, "                                        \0");
  pthread_mutex_unlock(&mux_curses);
}


/* routine to delete and redraw ball*/
void movement_message(WINDOW * my_win, message * m) {
  //pthread_mutex_lock(&mux_curses);
  wrefresh(my_win);
  draw_ball(my_win, & m -> ball_position, TRUE);
  wrefresh(my_win);
  draw_ball(my_win, & m -> ball_position, FALSE);
  //pthread_mutex_unlock(&mux_curses);
}

int main(int argc, char * argv[]) {
  if (argc < 2) {
    printf("to run client supply the server adress after ./CLIENT.proj \n");
    exit(0);
  }
  char * adress_keyboard = argv[1];
  int sock_fd, condition, key = -1;
  int scores[2];      // counts the balls missed and scored troughout all the game(keeps even if released)
  scores[0] = 0;
  scores[1] = 0;
  message m;
  paddle_position paddle;
  struct sockaddr_in server_addr;
  bool first_iteration =TRUE;
  criar_socket( & sock_fd);

  //define safe thread to use ncurses
  
  pthread_mutex_init(&mux_curses, NULL);
  pthread_t ball_thread,quit_thread;
  update_ball_t update_ball_struct;
  quit_t quit_struct;
 

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SOCK_PORT);
  if (inet_pton( AF_INET, adress_keyboard, & server_addr.sin_addr) < 1) {
    //inet_pton - convert IPv4 and IPv6 addresses from text to binary form
    printf("no valid address: \n");
    exit(-1);
  }
  
  // send connection message

  m.msg_type = 0; //connect;
  sendto(sock_fd, & m, sizeof(message), 0,
    (const struct sockaddr * ) & server_addr, sizeof(server_addr)); //send the connection message

  initialize_ncurses();
  WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0); // main window for game
  WINDOW * max_player_win = newwin(4, WINDOW_SIZE, 0, 0); //window displayed when max players are exceed 
  WINDOW * message_win = newwin(5, WINDOW_SIZE + 25, WINDOW_SIZE, 0); // message window with infomation if in PLAY_STATE
  WINDOW * score_win = newwin(WINDOW_SIZE, 24, 0, WINDOW_SIZE + 1); //Score window at the right side of the game with player scores
  WINDOW * controls_and_info = newwin(6, WINDOW_SIZE + 25, WINDOW_SIZE + 5, 0); // message window with infomation if in PLAY_STATE
  new_paddle( & paddle, PADLE_SIZE); // initializes all paddle arguments 
  //TODO_9
  // prepare the movement message  
  nodelay(my_win, TRUE);
  update_ball_struct.m= &m;
  update_ball_struct.message_win = message_win;
  
  update_ball_struct.my_win = my_win;
  update_ball_struct.paddle = & paddle;
  update_ball_struct.scores= scores;
  
  update_ball_struct.play_state =FALSE;
  
 
  do {
    recv(sock_fd, & m, sizeof(message), 0);
    if (m.msg_type == 5) {
      safe_mvwprintw(max_player_win, 1, 1, "MAX PLAYER NUMBER EXCEEDED, PROCESS WILL BE KILLED IN 5s");
      sleep(5);
      break;
    }
    if (first_iteration){
      create_windows(my_win, message_win, score_win, controls_and_info);
      quit_struct.my_win = my_win;
      quit_struct.sock_fd = &sock_fd;
      quit_struct.m =&m;
      quit_struct.server_addr= &server_addr;
      quit_struct.key = &key;
      pthread_create(&quit_thread, NULL, quit_func,(void*)(&quit_struct));
      pthread_create(&ball_thread, NULL, update_ball_thread, (void *)( &update_ball_struct));
      first_iteration = FALSE;
    }
    
    if (m.msg_type == 0) {
      connect_message(message_win );
      update_ball_struct.play_state =FALSE;
      continue;
    }

    if (m.point) update_scoreboard(m.score, score_win);
    switch (m.msg_type) {
    case 2: // send _ball == playstate
      do {
        update_ball_struct.play_state = TRUE;
        start_play_state(my_win, & paddle, & m, message_win);
        //newwwww
    /*need to create the struct to pass all needed info */
       // pthread_create(&ball_thread, NULL, update_ball_thread, (void *)( &update_ball_struct));
        m.msg_type =3;
        //key = wgetch(my_win);
        condition = (key == KEY_LEFT || KEY_RIGHT || KEY_UP || KEY_DOWN || 'a' || 's' || 'd' || 'w');
        if (condition) update_paddle(my_win, & paddle, key, message_win);
        key = 0;
      } while ((!(condition)) && (m.msg_type != 1));

      sendto(sock_fd, & m, sizeof(message), 0,
        (const struct sockaddr * ) & server_addr, sizeof(server_addr)); //send the move ball message
      //wrefresh(message_win);
      break;
    case 3: //move_ball
      update_ball_struct.play_state =FALSE;
      movement_message(my_win, & m);
      break;

    default:
    pthread_mutex_lock(&mux_curses);
      wrefresh(message_win);
    pthread_mutex_unlock(&mux_curses);
    }
  } while (key != 27);
  close(sock_fd);
  endwin(); // End curses mode	
  return 0;
}