#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <time.h>
#include <pthread.h>

#define N_THREADS 6
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
}message;

typedef struct client_addr_t{
    char addr[INET_ADDRSTRLEN];
    int port;
}client_addr_t;


typedef struct player_changer_t{
    message * m;
    client_addr_t* clients_data;
    int* active_client;
    int* client;
}player_changer_t;

typedef struct client_struct_t{
    int* client_fd;
    int* key;
}client_struct_t;

typedef struct update_ball_t{
    message * m;
    paddle_position *paddle; //= &paddle
    bool* play_state;
    bool* move_ball; 
    struct sockaddr_in* client_addr;
	socklen_t *size_addr;
    int *scores;

}update_ball_t;

typedef struct communication_t{
    bool* move_ball; 
    int* key;
    message * m;
    char * adress_keyboard;
    paddle_position* paddle;
    bool* play_state;
    update_ball_t* update_ball_thread;
}communication_t;