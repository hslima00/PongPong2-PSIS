#include "header.h"
//Creates socket and verifies if it is well created 
void criar_socket(int * sock_fd) {
    * sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if ( * sock_fd == -1) {
        perror("socket: ");
        exit(-1);
    }
}
//Adds clients that connect for the first time to the server and saves their info in a array of clients
void add_client(client_addr_t * clients_data, int client, struct sockaddr_in client_addr, message * m) {
    struct sockaddr_in* ptr_to_addr = (struct sockaddr_in*)&client_addr;
    struct in_addr addr_to_store = ptr_to_addr->sin_addr;
    inet_ntop( AF_INET, &addr_to_store, clients_data[client].addr, INET_ADDRSTRLEN );
    clients_data[client].port = ntohs(client_addr.sin_port);
    m -> msg_type = 0; //connect message
}
//Initializes ball at a random position
void place_ball_random(ball_position_t * ball) {
    ball -> x = rand() % WINDOW_SIZE;
    ball -> y = rand() % WINDOW_SIZE;
    ball -> c = 'o';
    ball -> up_hor_down = rand() % 3 - 1; //  -1 up, 1 - down
    ball -> left_ver_right = rand() % 3 - 1; // 0 vertical, -1 left, 1 right
}
//Creates a window and draws a border
void create_window(WINDOW * my_win) {
    box(my_win, 0, 0);
    wrefresh(my_win);
    keypad(my_win, true);
}
//Initializes initscr( ), cbreak( ) and noecho( )
void initialize_ncurses() {
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
}
//Removes a client and updates the struct array of clients shifting it left
void remove_client(struct sockaddr_in * clients_data, int removed, int client, message * m) {
    for (int j = removed; j < client; j++) {
        m -> score[j] = m -> score[j + 1];
        clients_data[j].sin_addr = clients_data[j + 1].sin_addr;
    }
    m -> score[client] = -1; 
}
//Initializes all score values to -1
void init_score(int score[]) {
    for (int i = 1; i <= MAX_CLIENTS; i++) {
        score[i] = -1;
    }
    score[0] = 0;
}
/*When the first client connects to the server, it runs a certain routine, reseting the value of scores, placing ball in a random position,
sets the release condition FALSE*/
void first_client_routine(message * m, int * active_client, int * check_for_new_client) {
    init_score(m -> score); 
    place_ball_random( & m -> ball_position); 
    m -> point = TRUE; 
    m -> msg_type = 2; //send _ball
    * active_client = 0;
    * check_for_new_client = 0;
}

int main() {
    initialize_ncurses();

    int sock_fd;
    int check_for_new_client = 0;                                           //Control variable for new clients used in score evaluations
    int client = 0;                                                         //Total Number of Clients connected to server at given point
    int active_client;                                                      //Client ID that is currently in PLAY STATE
    struct sockaddr_in local_addr;                                          //structure describing an internet socket address
    struct sockaddr_in client_addr;                     //Struct containing the data from client adress
    message m;                                                              //Message sent/received to/for client
    
    pthread_t player_changer_thread;
    player_changer_t player_changer_struct;
    time_t start, end;                                                     //Time control var
    int timer;                                                              //Time control var
    client_addr_t clients_data[MAX_CLIENTS];                           //Array of structures that save the client data
    
    criar_socket( & sock_fd);

    //BIND SOCKET                                                           
    local_addr.sin_family = AF_INET;                                       
    local_addr.sin_port = htons(SOCK_PORT);                                 
    local_addr.sin_addr.s_addr = INADDR_ANY;
    socklen_t client_addr_size = sizeof(struct sockaddr_in);                //Size of the address
    bind(sock_fd, (struct sockaddr * ) & local_addr, sizeof(local_addr));   
    

    WINDOW * my_win = newwin(5, WINDOW_SIZE, 0, 0);
    create_window(my_win);

    while (1) {
        recvfrom(sock_fd, & m, sizeof(message), 0,
            (struct sockaddr * ) & client_addr, & client_addr_size);

        switch (m.msg_type) {
        case 0: // -> new client (connect)
            if (client == MAX_CLIENTS) { // FULL_SERVER
                m.msg_type = 5;
            } else {
                add_client(clients_data, client, client_addr, & m);
                if (client == 0) { //First client to connect(step C).
                    first_client_routine( & m, & active_client, & check_for_new_client);
                    time( & start);
                }
                client++;
                mvwprintw(my_win, 2, 1, "%d connected at the moment", client);
                wrefresh(my_win);
            }
            inet_pton( AF_INET ,clients_data[check_for_new_client].addr, &client_addr.sin_addr );
				client_addr.sin_port = htons(clients_data[check_for_new_client].port);
            sendto(sock_fd, & m, sizeof(m), 0, (const struct sockaddr * ) & client_addr, client_addr_size);
            break;
        case 1:
            //Release_ball
            if (m.point)
                m.score[active_client]++;
            m.msg_type = 3;
            for (int i = 0; i < client; i++) {
                inet_pton( AF_INET ,clients_data[i].addr, &client_addr.sin_addr );
				client_addr.sin_port = htons(clients_data[i].port);
                sendto(sock_fd, & m, sizeof(m), 0, (const struct sockaddr * ) & client_addr, client_addr_size);
            }
            m.point = FALSE;
            //Choose next Client               //
            if (active_client + 1 == client)   //
                active_client = -1;            //
            active_client++;                   //
           // m.allow_release = FALSE;           //
            time( & start);                    //
            m.msg_type = 2;                    //next client enters Play State
            //Send Ball
            inet_pton( AF_INET ,clients_data[active_client].addr, &client_addr.sin_addr );
		    client_addr.sin_port = htons(clients_data[active_client].port);             
            sendto(sock_fd, & m, sizeof(m), 0, (const struct sockaddr * ) & client_addr, client_addr_size);
            break;

        case 3: //Move_Ball (Step L)
            //Updates ball position to all clients that aren't in Play State
            if (m.point)
                m.score[active_client]++;
           // m.allow_release = FALSE;
            while (check_for_new_client != client) {
                m.score[check_for_new_client] = 0;
                check_for_new_client++;
                m.point = TRUE; //Forces score to TRUE
            }
            m.msg_type = 3;
            for (int i = 0; i < client; i++) {
                if (i != active_client)
                    inet_pton( AF_INET ,clients_data[i].addr, &client_addr.sin_addr );
				    client_addr.sin_port = htons(clients_data[i].port);
                    sendto(sock_fd, & m, sizeof(m), 0, (const struct sockaddr * ) & client_addr, client_addr_size);
            }
            m.msg_type = 2;
            time( & end);
            timer = difftime(end, start);
           // if (timer >= 10) m.allow_release = TRUE;
            inet_pton( AF_INET ,clients_data[active_client].addr, &client_addr.sin_addr );
		    client_addr.sin_port = htons(clients_data[active_client].port);
            sendto(sock_fd, & m, sizeof(m), 0, (const struct sockaddr * ) & client_addr, client_addr_size);
            
            break;
        case 4: //Client Disconnect
            if (active_client+1 != client){
                for (int j = active_client ; j < client; j++) { //shift left
                    m.score[j] = m.score[j + 1];
                    strcpy(clients_data[j].addr , clients_data[j + 1].addr);
                    clients_data[j ].port = clients_data[j + 1].port;
                }
            }    
            m.score[client] = -1; //mete ultima posição a -1(cliente removido)
            m.point =TRUE;
            client--;
            check_for_new_client--;
            mvwprintw(my_win, 2, 1, "%d connected at the moment", client);
            wrefresh(my_win);
            for (int i = 0; i < client; i++) { //Updatescoreboard for players
                inet_pton( AF_INET ,clients_data[i].addr, &client_addr.sin_addr );
				client_addr.sin_port = htons(clients_data[i].port);
                sendto(sock_fd, & m, sizeof(m), 0, (const struct sockaddr * ) & client_addr, client_addr_size);
            }
            m.msg_type = 2;
            if (active_client == client){
                active_client = 0;
            }
            inet_pton(AF_INET ,clients_data[active_client].addr, &client_addr.sin_addr);
			client_addr.sin_port = htons(clients_data[active_client].port);
            sendto(sock_fd, & m, sizeof(m), 0, (const struct sockaddr * ) & client_addr, client_addr_size);
            time( & start);
            break;
        }
    }
    close(sock_fd);
    endwin(); //End ncurses mode
    return 0;
}