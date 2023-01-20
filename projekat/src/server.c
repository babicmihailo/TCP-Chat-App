#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "list.h"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT   20186
#define AVAILABLE      0
#define ACTIVE         1


struct Node* head = NULL;


void* clientThread(void* vargp);
void ActiveUsers(int clientSock, int port);
void Logout(int clientSock, int port);
void Selection(int numOption, int clientSock, int port);
void NumReceive(int clientSock, int port);
void NewUser(int clientSock, int port);
void TalkTo(int clientSock, int port);
void* DataSend(void* vargp);
void* DataReceive(void* vargp);

int main(int argc , char *argv[])
{
    system("clear");

    int socket_desc, c;
    struct sockaddr_in server , client;
    pthread_t* threadId;
    int* clientSock;

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);

    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    listen(socket_desc , 3);

    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    while(1)
    {
        clientSock = (int*)malloc(sizeof(int));
        *clientSock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);

        if (clientSock < 0)
        {
            perror("accept failed");
            return 1;
        }

        puts("Connection accepted");
        threadId = (pthread_t*)malloc(sizeof(pthread_t));
        pthread_create(threadId, NULL, clientThread, (void*)(clientSock));
        pthread_detach(*threadId);
    }

    return 0;
}

void* clientThread(void* vargp)
{
    int size, port;
    int clientSock = *((int*)vargp);

    while (1)
    {
        size = recv(clientSock , &port , sizeof(port) , 0);
        if(size == -1)
        {
            perror("recv failed");
            return NULL;
        }
        if(size == sizeof(port))
        {
            break;
        }
    }

    NewUser(clientSock, port);

    close(clientSock);
    return NULL;
}

void Selection(int numOption, int clientSock, int port) //Sa klijenta primam redni broj operacije koju je klijent izabrao
{
    if (numOption == 1)
    {
        ActiveUsers(clientSock, port);
    }
    else if (numOption == 2)
    {
        TalkTo(clientSock, port);
    }
    else if (numOption == 3)
    {
        Logout(clientSock, port);
    }
    else
    {
        perror("Invalid action");
    }
}

void ActiveUsers(int clientSock, int port) //Salje sve aktivne korisnike formatirane lepo
{
    struct Node* temp = head;
    char* tempUserData = UserData(temp);
    char userData[DEFAULT_BUFLEN];
    strcpy(userData, tempUserData);
    free(tempUserData);
    printf("User data: %s\n", userData);
    if(send(clientSock, userData, DEFAULT_BUFLEN, 0) < 0)
    {
        perror("Send failed");
        exit(1);
    }

    temp = head;
    PrintUsers(temp);
    temp = head;
    char* tempUsers = AllUsernames(temp);
    char allUsernames[DEFAULT_BUFLEN];
    strcpy(allUsernames, tempUsers);
    free(tempUsers);
    if(send(clientSock, allUsernames, DEFAULT_BUFLEN, 0) < 0)
    {
        perror("Send failed");
    };

    NumReceive(clientSock, port);
}

void Logout(int clientSock, int port) //Logovanje korisnika i brisanje iz liste
{
    struct Node* temp = head;
    while(temp != NULL)
    {
       if (temp->socket == clientSock)
       {
           printf("\nUser %s logged out.\n", temp->username);
           RemoveUser(&head, temp);
           break;
        }
        temp = temp->next;
    }

    PrintUsers(head);

    NewUser(clientSock, port);
}

void TalkTo(int clientSock, int port) //Pokretanje konekcije 2 klijenta
{
    char talkToUsername[DEFAULT_BUFLEN];
    while(1)
    {
        int size = recv(clientSock , talkToUsername , DEFAULT_BUFLEN , 0);
        if (size == -1)
        {
            perror("recv");
        }
        if(size == DEFAULT_BUFLEN)
        {
            break;
        }
    }


    struct Node* user = head;
    struct Node* talkTo = head;
    int state = 0;
    int userSocket = 0;
    int userSocketChat = 0;
    int talkToSocket = 0;
    int talkToSocketChat = 0;

    while (talkTo != NULL)
    {
        if (strcmp(talkTo->username, talkToUsername) == 0)
        {
            talkToSocket = talkTo->socket;
            talkToSocketChat = talkTo->socketChat;
            break;
        }
        talkTo = talkTo->next;
    }

    while (user != NULL)
    {
        if (user->socket == clientSock)
        {
            user->socketChat = talkToSocket;
            userSocket = user->socket;
            userSocketChat = user->socketChat;
            break;
        }
        user = user->next;
    }

    int size = 0;
    if (talkToSocketChat == userSocket && userSocketChat == talkToSocket) //drugi klijent ulazi ovde
    {
        state = 1;
        while(1)
        {
            size = send(userSocketChat, &state, DEFAULT_BUFLEN, 0);
            if(size < 0)
            {
                perror("Send failed");
                break;
            }
            if(size == DEFAULT_BUFLEN)
            {
                break;
            }
        }

    }
    else if (talkToSocketChat == AVAILABLE && userSocketChat == talkToSocket) //prvi klijent koji posalje zahtev ulazi ovde
    {
        state = 2;
    }
    else
    {
        state = 0;
        talkTo->socketChat = AVAILABLE;
        user->socketChat = AVAILABLE;
    }

    if(send(clientSock, &state, DEFAULT_BUFLEN, 0) < 0)
    {
        perror("Send failed");
    }
    if (state == 1) //klijenti su povezani pokrece se zahtev za komunikaciju
    {
        user->active = ACTIVE;
        talkTo->active = ACTIVE;

        pthread_t* sendThread;
        pthread_t* receiveThread;

        sendThread = (pthread_t*)malloc(sizeof(pthread_t));
        receiveThread = (pthread_t*)malloc(sizeof(pthread_t));

        pthread_create(sendThread, NULL, DataSend, (void *)(&clientSock));
        pthread_create(receiveThread, NULL, DataReceive, (void *)(&clientSock));

        pthread_detach(*sendThread);
        pthread_detach(*receiveThread);
        while (1)
        {
            if (user->active != ACTIVE || talkTo->active != ACTIVE)
            {
                pthread_cancel(*sendThread);
                pthread_cancel(*receiveThread);
                break;
            }
        }
        printf("Both users disconnected\n");
        user->active = 0;
        user->socketChat = AVAILABLE;

        talkTo->active = 0;
        talkTo->socketChat = AVAILABLE;


        free(sendThread);
        free(receiveThread);
        printf("Freed memory\n");
    }

    NumReceive(clientSock, port);
}

void* DataReceive(void* vargp) //thread za primanje podataka
{
    int size;
    int clientSock = *((int*)vargp);
    struct Node* user = head;
    int otherClientSock;

    while(1)
    {
        user = head;
        while (user != NULL)
        {
            if (user->socket == clientSock)
            {
                otherClientSock = user->socketChat;
                break;
            }
            user = user->next;
        }
        char data[DEFAULT_BUFLEN];
        while(1)
        {
            size = recv(otherClientSock , data , DEFAULT_BUFLEN , 0);
            if (size == -1)
            {
                perror("recv");
            }
            if(size == DEFAULT_BUFLEN)
            {
                break;
            }
        }
        printf("Data> %s\n", data);
        if(send(clientSock, data, DEFAULT_BUFLEN, 0) < 0)
        {
            perror("Send failed");
        }

        char *disconnect = malloc(DEFAULT_BUFLEN);
        disconnect = strstr(data, "Disconnect");
        if (disconnect)
        {
            user->active = 0;
            puts("Detected Disconnect signal, returning to menu");
            break;
        }
    }
    return NULL;
}

void* DataSend(void* vargp) //thread za slanje podataka
{
    int size;
    int clientSock = *((int*)vargp);
    struct Node* user = head;
    int otherClientSock;
    while(1)
    {
        user = head;
        while (user != NULL)
        {
            if (user->socket == clientSock)
            {
                otherClientSock = user->socketChat;
                break;
            }
            user = user->next;
        }
        char data[DEFAULT_BUFLEN];
        while(1)
        {
            size = recv(clientSock , data , DEFAULT_BUFLEN , 0);
            if (size == -1)
            {
                perror("recv");
            }
            if(size == DEFAULT_BUFLEN)
            {
                break;
            }
        }
        printf("Data> %s\n", data);
        if(send(otherClientSock, data, DEFAULT_BUFLEN, 0) < 0)
        {
            perror("Send failed");
        }

        char *disconnect = malloc(DEFAULT_BUFLEN);
        disconnect = strstr(data, "Disconnect");
        if (disconnect)
        {
            user->active = 0;
            puts("Detected Disconnect signal, returning to menu");
            break;
        }
    }
    return NULL;
}

void NumReceive(int clientSock, int port) //Broj funkcije koju je klijent izabrao
{
    int numOption = 0;
    while(1)
    {
        int size = recv(clientSock , &numOption , DEFAULT_BUFLEN , 0);
        if (size == -1)
        {
            perror("recv");
        }
        if(size == DEFAULT_BUFLEN)
        {
            break;
        }
    }
    Selection(numOption, clientSock, port);
}

void NewUser(int clientSock, int port) //dodaj novog korisnika
{
    struct Node* temp = head;
    char* tempUsers = AllUsernames(temp);
    char allUsernames[DEFAULT_BUFLEN];
    strcpy(allUsernames, tempUsers);
    free(tempUsers);

    if(send(clientSock, allUsernames, DEFAULT_BUFLEN, 0) < 0)
    {
        perror("Send failed");
    }

    char username[DEFAULT_BUFLEN];
    while(1)
    {
        int size = recv(clientSock , username , DEFAULT_BUFLEN , 0);
        if (size == -1)
        {
            perror("recv");
        }
            if(size == DEFAULT_BUFLEN)
        {
            break;
        }
    }
    AddUser(&head, port, clientSock, AVAILABLE, ACTIVE, username);
    printf("\nUser logged in: %s\n", username);
    NumReceive(clientSock, port);
}
