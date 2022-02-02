#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <ncurses.h>
#include  <time.h>
#include <pthread.h>

#define SOCK_PORT 6969
#define MAX_CLIENTS 3
#define WINDOW_SIZE 30
#define PADLE_SIZE 3

typedef enum direction_t {UP, DOWN, LEFT, RIGHT} direction_t;

typedef struct ball_position_t{
    int x, y;
    int up_hor_down; //  -1 up, 0 horizontal, 1 down
    int left_ver_right; //  -1 left, 0 vertical,1 right
    char c;
} ball_position_t;

typedef struct paddle_position{
    int x, y;
    int length;
} paddle_position;

typedef struct message 
{   
    int msg_type; /* 0-connect   1-release_ball  2-send_ball 3-move_ball  4-disconnect 5-MAX PLAYERS EXCEED*/ 
    ball_position_t ball_position;
    bool point; 
    int score[MAX_CLIENTS]; //vetor que tem os scores de cada client
    bool allow_release;
}message;

typedef struct client_addr_t{
    char addr[INET_ADDRSTRLEN];
    int port;
}client_addr_t;