#include "header.h"



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
void create_windows(WINDOW * my_win, WINDOW * message_win, WINDOW * score_win, WINDOW * controls_and_info, pthread_mutex_t mux_curses) {
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
  
    wmove(my_win, paddle -> y, x);
    waddch(my_win, ch);
  }
  wrefresh(my_win);
}

/* moves the paddle checking if the move  is valid */
void move_paddle(paddle_position * paddle, int direction, WINDOW * message_win) {
  mvwprintw(message_win, 3, 1, "                        ");
  wrefresh(message_win);
  if (direction == KEY_UP || direction == 'w') {
    if (paddle -> y != 1) {
      paddle -> y--;
      mvwprintw(message_win, 3, 1, "UP movement\0");
    } else mvwprintw(message_win, 3, 1, "Hitting BOX limit\0");
  }
  if (direction == KEY_DOWN || direction == 's') {
    if (paddle -> y != WINDOW_SIZE - 2) {
      paddle -> y++;
      mvwprintw(message_win, 3, 1, "DOWN movement\0");
    } else mvwprintw(message_win, 3, 1, "Hitting BOX limit\0");
  }
  if (direction == KEY_LEFT || direction == 'a') {
    if (paddle -> x - paddle -> length != 1) {
      paddle -> x--;
      mvwprintw(message_win, 3, 1, "LEFT movement\0");
    } else mvwprintw(message_win, 3, 1, "Hitting BOX limit\0");
  }
  if (direction == KEY_RIGHT || direction == 'd') {
    if (paddle -> x + paddle -> length != WINDOW_SIZE - 2) {
      paddle -> x++;
      mvwprintw(message_win, 3, 1, "RIGHT movement\0");
    } else mvwprintw(message_win, 3, 1, "Hitting BOX limit\0");
  }
  wrefresh(message_win);
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
  wmove(my_win, ball -> y, ball -> x);
  waddch(my_win, ch);
}
 /* simulates ball position to check if hitting edges or paddle and bouncing accordingly */
void simulate_ball_position(paddle_position * paddle, message * m, WINDOW * message_win, int key, int * scores) {
 
  ball_position_t simul_ball;
  simul_ball.c = m -> ball_position.c;
  simul_ball.left_ver_right = m -> ball_position.left_ver_right;
  simul_ball.up_hor_down = m -> ball_position.up_hor_down;
  simul_ball.x = m -> ball_position.x;
  simul_ball.y = m -> ball_position.y;
  move_ball( & simul_ball);

  int condition_1 = (((simul_ball.x <= paddle -> x + paddle -> length) && (simul_ball.x >= paddle -> x - paddle -> length)) );
  if (condition_1 ) {
    if(paddle -> y == simul_ball.y || paddle->y == m->ball_position.y){
      scores[0]++;
      m->point =TRUE;
    }   
    if (( m->ball_position.y == paddle -> y)){
      if ((key =='a' ||key == KEY_LEFT) && (m->ball_position.x == paddle -> x - paddle -> length)){
        if (m->ball_position.x == 1) {//top
          if(m->ball_position.y <= WINDOW_SIZE/2)m->ball_position.up_hor_down=1;
          else m->ball_position.up_hor_down=-1;
        }
        else  m->ball_position.left_ver_right =-1;
      }
      else if((key =='d' ||key == KEY_RIGHT) && (m->ball_position.x == paddle -> x + paddle -> length)){
        if( m->ball_position.x == WINDOW_SIZE -3){//bottom
          if(m->ball_position.y <= WINDOW_SIZE/2)m->ball_position.up_hor_down=1;
          else m->ball_position.up_hor_down=-1;
        }
        else m->ball_position.left_ver_right =1;
      }
      else m->ball_position.up_hor_down=-1;
      m->ball_position.y += m-> ball_position.up_hor_down;
      m->ball_position.x += m-> ball_position.left_ver_right;
    }
    else if (simul_ball.y == paddle->y){
      if ((key =='a' ||key == KEY_LEFT) && (simul_ball.x == paddle -> x - paddle -> length)){
        if (simul_ball.x == 1) {//left
          if(simul_ball.y <= WINDOW_SIZE/2)m->ball_position.up_hor_down=1;
          else m->ball_position.up_hor_down=-1;
        }
      }
      else if((key =='d' ||key == KEY_RIGHT) && (simul_ball.x == paddle -> x + paddle -> length)){
        if( simul_ball.x == WINDOW_SIZE -2){//right
          if(simul_ball.y <= WINDOW_SIZE/2)m->ball_position.up_hor_down=1;
          else m->ball_position.up_hor_down=-1;
        }
         m->ball_position.left_ver_right =1;
      }
      else  m->ball_position.left_ver_right =-1;
      m->ball_position.y += m-> ball_position.up_hor_down;
      m->ball_position.x += m-> ball_position.left_ver_right;
    }
    if (((paddle -> y == WINDOW_SIZE -3 )&& (key == KEY_DOWN ||key == 's')) && ((simul_ball.y == WINDOW_SIZE -2)||(m ->ball_position.y == WINDOW_SIZE -2))){
      m -> ball_position.y = WINDOW_SIZE -4;
      m -> ball_position.up_hor_down = -1;
    }
    else if (((paddle -> y == 2) && (key == KEY_UP || key =='w')) && ((simul_ball.y == 1) ||(m ->ball_position.y ==  1))){
      m -> ball_position.y = 3;
      m -> ball_position.up_hor_down = 1;
    } 
    
  }  
  else if (simul_ball.y == paddle -> y && !(simul_ball.up_hor_down == 0)) scores[1]++;
  mvwprintw(message_win, 2, 1, "SCORED - %d \t|\tMISSED - %d", scores[0], scores[1]);
  wrefresh(message_win);
}
/*updates the score board being printed in the score_win (score_win)on the right side of the main win(my_win*/
void update_scoreboard(int score[], WINDOW * score_win) {
  int temp_score[MAX_CLIENTS], temp_max = 0, k = 0, clt_on_score = -1;
  for (int j = 0; j < MAX_CLIENTS; j++) {
    temp_score[j] = -1;
    mvwprintw(score_win, j + 2, 1, "                      \0");
    wrefresh(score_win);
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
    mvwprintw(score_win, l + 2, 1, " %dº | PLAYER %d |%d\0 ", l + 1, k, temp_score[k]);
    wrefresh(score_win);
    temp_score[k] = -1;
    temp_max = 0;
  }

}

/*updates ball and paddle position*/
void update_ball_and_paddle(WINDOW * my_win, message * m, paddle_position * paddle, int key, WINDOW * message_win, int * scores) {
  draw_paddle(my_win, paddle, FALSE); //deletes paddle
  move_paddle(paddle, key, message_win); //calculates new paddle position according to user input
  draw_ball(my_win, & m -> ball_position, FALSE); //deletes ball  
  simulate_ball_position(paddle, m, message_win, key, scores); //simulates if ball will hit paddle or not,
  //if it does, changes direction of ball
  draw_paddle(my_win, paddle, TRUE);
  move_ball( & m -> ball_position);
  draw_ball(my_win, & m -> ball_position, TRUE);
  wrefresh(my_win);
} //delete previous ball, calculate new position and draws it 



// if released clears message window 
void clear_paddle_nd_msg_window(WINDOW * my_win, paddle_position * paddle, WINDOW * message_win) {
  draw_paddle(my_win, paddle, FALSE);
  mvwprintw(message_win, 1, 1, "                                        \0");
  mvwprintw(message_win, 2, 1, "                                        \0");
  mvwprintw(message_win, 3, 1, "                                        \0");
  mvwprintw(message_win, 4, 1, "                                        \0");
}

/*routine to start playe state*/
void start_play_state(WINDOW * my_win, paddle_position * paddle, message * m, WINDOW * message_win) {
  mvwprintw(message_win, 1, 1, "PLAY STATE");
  if (m -> allow_release) mvwprintw(message_win, 1, 16, "|\tYOU CAN RELEASE");
  draw_paddle(my_win, paddle, TRUE); // draws paddle in the defined position (mode TRUE=draw/ mode FALSE= delete)
  draw_ball(my_win, & m -> ball_position, TRUE);
  wrefresh(my_win);
  wrefresh(message_win);
  m -> point = FALSE;
  m -> msg_type = 3;
}

/*routine for success connection*/
void connect_message(WINDOW * message_win, pthread_mutex_t mux_curses) {
   pthread_mutex_lock(&mux_curses);
  mvwprintw(message_win, 1, 1, "SUCESS IN CONNECTION WAIT \0");
  wrefresh(message_win);
  mvwprintw(message_win, 1, 1, "                                        \0");
  pthread_mutex_unlock(&mux_curses);
}


/* routine to delete and redraw ball*/
void movement_message(WINDOW * my_win, message * m) {
  wrefresh(my_win);
  draw_ball(my_win, & m -> ball_position, TRUE);
  wrefresh(my_win);
  draw_ball(my_win, & m -> ball_position, FALSE);
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
  criar_socket( & sock_fd);

  //define safe thread to use ncurses
  pthread_mutex_t mux_curses;

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

  do {
    recv(sock_fd, & m, sizeof(message), 0);
    if (m.msg_type == 5) {
      //pthread_mutex_lock(&mux_curses);
      mvwprintw(max_player_win, 1, 1, "MAX PLAYER NUMBER EXCEEDED, PROCESS WILL BE KILLED IN 5s");
      wrefresh(max_player_win);
      //pthread_mutex_unlock(&mux_curses); 
      sleep(5);
      break;
    }
    create_windows(my_win, message_win, score_win, controls_and_info, mux_curses);
    if (m.msg_type == 0) {
      connect_message(message_win, mux_curses);
      continue;
    }

    if (m.point) update_scoreboard(m.score, score_win);
    switch (m.msg_type) {
    case 2: // send _ball == playstate
      do {
        start_play_state(my_win, & paddle, & m, message_win);
        key = wgetch(my_win);
        condition = (key == KEY_LEFT || KEY_RIGHT || KEY_UP || KEY_DOWN || 'a' || 's' || 'd' || 'w');
        if (key == 'q') {
          m.msg_type = 4; // disconnect message
          sendto(sock_fd, & m, sizeof(message), 0,
            (const struct sockaddr * ) & server_addr, sizeof(server_addr));
          close(sock_fd);
          endwin(); // End curses mode	
          return 0;
        } else if (key == 'r' && m.allow_release) {
          m.msg_type = 1; // release message
          clear_paddle_nd_msg_window(my_win, & paddle, message_win);
        } else if (condition) update_ball_and_paddle(my_win, & m, & paddle, key, message_win, scores);
      } while ((!(condition)) && (m.msg_type != 1));

      sendto(sock_fd, & m, sizeof(message), 0,
        (const struct sockaddr * ) & server_addr, sizeof(server_addr)); //send the move ball message
      wrefresh(message_win);
      break;
    case 3: //move_ball
      movement_message(my_win, & m);
      break;

    default:
      wrefresh(message_win);
    }
  } while (key != 27);
  close(sock_fd);
  endwin(); // End curses mode	
  return 0;
}