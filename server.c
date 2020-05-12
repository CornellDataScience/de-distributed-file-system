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
#include <limits.h>
void *connection_handler(void *);

#define LOCK_NOT_IN_USE 0
#define LOCK_READING 1 // in use
#define LOCK_WRITING 2 // in use

typedef struct lock
{
    int status;
    int client_id;                  //client using this lock
    char file_path[1024];           //path this lock is locking
    int waiting_buffer[UINT32_MAX]; //other clients waiting on this lock
    int num_waiting;
    pthread_mutex_t mutex_lock; //mutex lock
    pthread_cond_t lock_cv;     //condition variable

} lock_t;

typedef struct client_info
{
    int sockfd;
    char *message;

} client_info_t;

Hashmap *lock_map;    // file to lock_id
Hashmap *lock_status; // path to the status (lock in use, not in use)
int hash(void *str)
{
    char *s = (char *)str;
    int hash = 5381;
    int c;
    while ((c = *s++) != 0)
        hash = (hash * 31 + c) % INT_MAX;

    return hash;
}

bool equals(void *input1, void *input2)
{
    return strcmp((char *)input1, (char *)input2) == 0;
}

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

// send message with status "status"
void send_msg(message_t *msg, message_t lock_msg, int status)
{
    strcpy(msg->file_path, lock_msg.file_path);
    printf("SENDING SUCCESS VALUE OF %d", status);
    msg->isSuccess = status;
    msg->messageType = lock_msg.messageType;
}

message_t acquire_lock(message_t lock_msg, int client_id)
{
    lock_t *ret;

    // if it doesn't exist, then create the new lock, add the mapping from id to file in lock_map
    // set status to in use
    // client asks for a lock on a file, specified by file path
    // check lock_map to get id
    // to get lock status of a client's request, we need to check message's mode
    message_t msg;
    lock_t *lock;
    if ((lock = (lock_t *)hashmapGet(lock_status, lock_msg.file_path)) == NULL)
    {
        // create new lock
        lock = malloc(sizeof(lock_t));
        pthread_mutex_lock(&lock->mutex_lock);
        printf("got lock\n");
        lock->num_waiting = 0;
        lock->status = get_status_from_mode(lock_msg.messageType);
        lock->client_id = client_id;
        strcpy(lock->file_path, lock_msg.file_path);
        // strcpy(msg.file_path, lock_msg.file_path);
        // msg.isSuccess = SUCCESS;
        // msg.messageType = lock_msg.messageType;
        printf("sending message\n");
        send_msg(&msg, lock_msg, SUCCESS);
        printf("message sent\n");
        hashmapPut(lock_status, lock->file_path, lock);
        printf("returning");
        return msg;
    }
    else // if file has already been locked before
    {
        if (lock->status == LOCK_NOT_IN_USE)
        {
            lock = malloc(sizeof(lock_t));
            pthread_mutex_lock(&lock->mutex_lock);
            lock->num_waiting = 0;
            lock->status = get_status_from_mode(lock_msg.messageType);
            lock->client_id = client_id;
            strcpy(lock->file_path, lock_msg.file_path);
            send_msg(&msg, lock_msg, SUCCESS);
            // // send message to client that locking was successful
        }
        else // add to lock's queue
        {
            // FIX THIS PART!
            pthread_cond_wait(&lock->lock_cv, &lock->mutex_lock);
            lock->waiting_buffer[lock->num_waiting++];

            send_msg(&msg, lock_msg, WAITING);
        }
    }
    return lock_msg;
}

message_t release_lock(message_t lock_msg, int client_id)
{
    lock_t *lock; // contains lock we want to release
    message_t msg;

    if ((lock = (lock_t *)hashmapGet(lock_status, lock_msg.file_path)) == NULL)
    {
        // if the lock doesn't exist
        // send message to the client that
        send_msg(&msg, lock_msg, NO_SUCCESS);
    }
    else
    {
        if (lock->client_id != client_id)
        { // client is not holding that lock
            send_msg(&msg, lock_msg, NO_SUCCESS);
        }
        else
        {
            // modify hashmap if needed
            lock->status = 0;
            pthread_mutex_unlock(&lock->mutex_lock);
            pthread_cond_signal(&lock->lock_cv);
            send_msg(&msg, lock_msg, SUCCESS);
        }
    }
    return msg;
}

//the thread function
int main(int argc, char *argv[])
{
    lock_status = hashmapCreate(10, &hash, equals);
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
    server.sin_port = htons(9000);

    //Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
    listen(socket_desc, 3);
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    pthread_t thread_id;
    client_info_t info;

    while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
    {
        puts("Connection accepted");
        if (pthread_create(&thread_id, NULL, connection_handler, (void *)&client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        pthread_join(thread_id, NULL);
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
    char client_message[2000];
    printf("receiving message\n");
    while ((read_size = recv(sock, client_message, 2000, 0)) > 0)
    {
        printf("received message\n");
        message_t new_msg;
        message_t msg = decodeMessage(client_message);
        if (msg.messageType == ACQUIRE_LOCK)
        {
            printf("I am here\n");
            new_msg = acquire_lock(msg, sock);
            printf("message value %d\n", new_msg.isSuccess);
            // sock is client id
        }
        else if (msg.messageType == RELEASE_LOCK)
        {
            new_msg = release_lock(msg, sock);
            // stuff
        }
        else if (msg.messageType == REQUEST_READ)
        {
            send_msg(&new_msg, msg, SUCCESS);
        }
        char buffer[1024];
        printf("MESSAGE HAS SUCCESS VALUE OF %d", new_msg.isSuccess);
        encodeMessage(new_msg, buffer);
        printf("About to write %s\n", buffer);
        write(sock, buffer, strlen(buffer));
        printf("done writing\n");
        // message = "Client accepted";
        // write(sock, message, strlen(message));

        // message = "Now type something and i shall repeat what you type \n";
        // write(sock, message, strlen(message));
        //Receive a message from client
        // while ((read_size = recv(sock, client_message, 2000, 0)) > 0)
        // {
        //     client_message[read_size] = '\0';
        //     message_t msg = decodeMessage(client_message);
        //     if (msg.messageType == ACQUIRE_LOCK)
        //     {
        //         new_msg = acquire_lock(msg, sock);
        //         // sock is client id
        //     }
        //     else if (msg.messageType == RELEASE_LOCK)
        //     {
        //         new_msg = release_lock(msg, sock);
        //         // stuff
        //     }
        //     char buffer[1024];
        //     encodeMessage(new_msg, buffer);
        //     //Send the message back to client
        //     // write(sock, client_message, strlen(client_message));
        //     write(sock, buffer, strlen(buffer));

        //     //clear the message buffer
        //     memset(client_message, 0, 2000);
        // }

        if (read_size == 0)
        {
            puts("Client disconnected");
            fflush(stdout);
        }
        else if (read_size == -1)
        {
            perror("recv failed");
        }
        printf("done with function\n");
    }
    return 0;
}