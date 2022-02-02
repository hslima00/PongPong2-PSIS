#include "myqueue.h"
#include <stdlib.h>

node_t* head = NULL;
node_t* tail = NULL;
void enqueue(int *client_socket) {
    node_t *newnode = malloc(sizeof(node_t));
    newnode->client_socket = client_socket;
    newnode->next = NULL;
    if (tail == NULL){
        head = newnode;
    }else {
        tail->next = newnode;
    }
    tail = newnode;
}

//returns NULL if the queue 1s empty.
//Returns the pointer to a ciert socket, 1f there is one to get

int* dequeue(){
    if(head == NULL){
        return NULL;
    }
    else{
        int * result = head->client_socket;
        node_t *temp=head;
        head = head->next;
        if (head== NULL){tail = NULL;}
        free(temp);
        return result;
    }
}