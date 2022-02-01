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

#define SOCK_PORT 4000
#define MAX_CLIENTS 10

#define WINDOW_SIZE 30
#define PADDLE_SIZE 3

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

typedef struct client_info_s
{
    char client_address_s[INET_ADDRSTRLEN];     //Clients address (saved in server)
    int port;                                   //Clients port (saved in server)
    int score;                                  //Clients score (saved in server)
    int client_ID;                              //Clients ID (saved in server)
    struct paddle_position paddle_position_s;   //Paddle position of players (saved in server)
}client_info_s;

typedef struct client_info_t{
    int score;                          //Score of the [client_contacting] 
    paddle_position paddle_position;    //[client_contacting] current paddle position
    int client_ID;                      //Index that can change always < 10 and basicly is the position of the matrix score to look at
}client_info_t;

typedef struct message 
{  
    int client_contacting;              //Client that is sending the message to the server
    int msg_type;                       //0-Connect   1-Disconnect  2-Paddle_move 3-Board_update  4-Server is full  
    client_info_t cinfo[MAX_CLIENTS];   //Struct[MAX_CLIENTS] saves relevant info about the clients in the message
    ball_position_t ball_position;      //Ball Position
    bool point;                         //If the client score point = TRUE, else point = FALSE
}message;