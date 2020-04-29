/*
    C socket server example, handles multiple clients using threads
    Compile
    gcc server.c -lpthread -o server
*/

#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h>   //for threading , link with lpthread
#include "lockServerMessage.h"
#include "hashmap.h"

void *connection_handler(void *);

#define LOCK_NOT_IN_USE 0
#define LOCK_READING 1 // in use
#define LOCK_WRITING 2 // in use

typedef struct lock
{
    int status;
    // int lock_id;                    // id of the lock
    int client_id;                        //client using this lock
    char file_path[1024];                 //path this lock is locking
    message_t waiting_buffer[UINT32_MAX]; //other lock_msgs waiting on this lock
    int num_waiting;                      //number of lock_msgs waiting on queue
} lock_t;

map_t lock_map;    // file to lock_id
map_t lock_status; // path to the status (lock in use, not in use)

int get_status_from_mode(int mode)
{
    switch (mode)
    {
    case REQUEST_READ:
        return LOCK_READING;
    case REQUEST_WRITE:
        return LOCK_WRITING;
    default:
        return LOCK_NOT_IN_USE;
    }
}

message_t acquire_lock(message_t lock_msg, int client_id)
{
    lock_t* ret;
    // if it doesn't exist, then create the new lock, add the mapping from id to file in lock_map
    // set status to in use
    // client asks for a lock on a file, specified by file path
    // check lock_map to get id
    // to get lock status of a client's request, we need to check message's mode

    message_t msg;
    printf("In acquire lock \n");
    // printf("key %s\n", lock_msg.file_path);
    // hashmap_put(lock_status, lock_msg.file_path, ret);
    if (hashmap_get(lock_status, lock_msg.file_path,(void**) &ret) == MAP_MISSING)
    // // this a new file
    {
        printf("hi\n");
        lock_t new_lock;
        // new_lock.status = get_status_from_mode(lock_msg.messageType);
        // new_lock.client_id = client_id;
        // strcpy(new_lock.file_path, lock_msg.file_path);
        // // printf("here\n");
        // // // message it was successful
        // strcpy(msg.file_path, lock_msg.file_path);
        // msg.isSuccess = SUCCESS;
        // msg.messageType = lock_msg.messageType;
        // printf("Return message\n");
        // return msg;
    }
    // else // if file has already been locked before
    // {
    //     if (ret.status == LOCK_NOT_IN_USE)
    //     {
    //         ret.status = get_status_from_mode(lock_msg.messageType);
    //         strcpy(msg.file_path, lock_msg.file_path);
    //         msg.isSuccess = SUCCESS;
    //         msg.messageType = lock_msg.messageType;
    //         // // send message to client that locking was successful
    //     }
    //     else // add to lock's queue
    //     {
    //         // if (ret.status == LOCK_READING)
    //         // {
    //         // }
    //         // else
    //         // {
    //         ret.waiting_buffer[ret.num_waiting++] = lock_msg;
    //         strcpy(msg.file_path, lock_msg.file_path);
    //         msg.isSuccess = WAITING;
    //         msg.messageType = lock_msg.messageType;
    //         // }
    //     }
    // }
    return lock_msg;
}

// void release_lock(message_t lock_msg, int client_id)
// {
// }

//the thread function
int main(int argc, char *argv[])
{
    lock_status = hashmap_new();
    int socket_desc, client_sock, c;
    struct sockaddr_in server, client;

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8889);

    //Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc, 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    pthread_t thread_id;

    while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
    {

        puts("Connection accepted");

        if (pthread_create(&thread_id, NULL, connection_handler, (void *)&client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }

    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int *)socket_desc;
    int read_size;
    char *message, client_message[2000];

    //Send some messages to the client
    // message = "Greetings! I am your connection handler\n";
    // write(sock, message, strlen(message));
    message = "Client accepted";
    write(sock, message, strlen(message));

    // message = "Now type something and i shall repeat what you type \n";
    // write(sock, message, strlen(message));

    message_t new_msg;
    //Receive a message from client
    while ((read_size = recv(sock, client_message, 2000, 0)) > 0)
    {
        //end of string marker
        printf("%d\n", sock);
        printf("%s\n", client_message);
        client_message[read_size] = '\0';
        message_t msg = decodeMessage(client_message);
        if (msg.messageType == ACQUIRE_LOCK)
        {
            new_msg = acquire_lock(msg, sock);
            // printf("Finish acquiring lock\n");
            // sock is client id
        }
        else if (msg.messageType == RELEASE_LOCK)
        {
            // stuff
        }

        // wants to aquire

        // message_t msg;
        // msg.messageType = 0;
        // strcpy(msg.file_path, "file123.txt");
        // msg.isSuccess = 1;
        char buffer[1024];
        encodeMessage(msg, buffer);
        //Send the message back to client
        // write(sock, client_message, strlen(client_message));
        write(sock, buffer, strlen(buffer));

        //clear the message buffer
        memset(client_message, 0, 2000);
    }

    if (read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if (read_size == -1)
    {
        perror("recv failed");
    }

    return 0;
}