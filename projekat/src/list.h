#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_BUFLEN 512

struct Node
{
    int port;
    int socket;
    int socketChat;
    int active;
    char* username;
    struct Node* next;
    struct Node* prev;
};

void AddUser(struct Node** head, int port, int socket, int socketChat, int active, char* username)
{
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));

    newNode->port = port;
    newNode->socket = socket;
    newNode->socketChat = socketChat;
    newNode->active = active;

    newNode->username = (char*)malloc(DEFAULT_BUFLEN);
    strcpy(newNode->username, username);

    newNode->next = (*head);
    newNode->prev = NULL;

    if ((*head) != NULL)
    {
        (*head)->prev = newNode;
    }

    (*head) = newNode;
}

void RemoveUser(struct Node** head, struct Node* node)
{
    if (node->prev != NULL)
    {
        node->prev->next = node->next;
    }
    else
    {
        *head = node->next;
    }

    if (node->next != NULL)
    {
        node->next->prev = node->prev;
    }
    free(node);
}

void PrintUsers(struct Node* head)
{
    struct Node* temp = head;
    while (temp != NULL)
    {
        printf("\nUsername> %s, Port> %d, Chat> %d, Socket> %d",
               temp->username, temp->port, temp->socketChat, temp->socket);
        temp = temp->next;
    }
    printf("\n");
}

char* UserData(struct Node* head)
{
    struct Node* temp = head;
    char *allUsers[DEFAULT_BUFLEN];
    char *port[DEFAULT_BUFLEN];
    char *socket[DEFAULT_BUFLEN];
    char *socketChat[DEFAULT_BUFLEN];
    int i = 0;
    while (temp != NULL)
    {
        port[i] = (char*)malloc(DEFAULT_BUFLEN);
        socket[i] = (char*)malloc(DEFAULT_BUFLEN);
        socketChat[i] = (char*)malloc(DEFAULT_BUFLEN);
        if(port[i] == NULL || socket[i] == NULL || socketChat[i] == NULL)
        {
            printf("UserData neki od intova[i]");
            exit(1);
        }
        sprintf(port[i], "%d", temp->port);
        sprintf(socket[i], "%d", temp->socket);
        sprintf(socketChat[i], "%d", temp->socketChat);
        allUsers[i] = (char*)malloc(DEFAULT_BUFLEN); //
        if(allUsers[i] == NULL)
        {
            printf("UserData allUsers[i]");
            exit(1);
        }
        strcpy(allUsers[i], "Username: ");
        strcat(allUsers[i], temp->username);
        strcat(allUsers[i], " Port: ");
        strcat(allUsers[i], port[i]);
        strcat(allUsers[i], " Socket: ");
        strcat(allUsers[i], socket[i]);
        strcat(allUsers[i], " Talking with: ");
        strcat(allUsers[i], socketChat[i]);
        free(port[i]);
        free(socket[i]);
        free(socketChat[i]);
        temp = temp->next;
        i++;
    }

    char *result = (char*)malloc(i * DEFAULT_BUFLEN);
    if(result == NULL)
    {
        printf("UserData result");
        exit(1);
    }
    for(int j = 0;j < i; ++j)
    {
        strcat(result, allUsers[j]);
        free(allUsers[j]);
        strcat(result, "\n");
    }

    return result;
}

char* AllUsernames(struct Node* head)
{
    struct Node* temp = head;
    char *allUsers[DEFAULT_BUFLEN];
    int i = 0;

    while (temp != NULL)
    {
        allUsers[i] = (char*)malloc(DEFAULT_BUFLEN);
        if(allUsers[i] == NULL)
        {
            printf("AllUsernames allUsers[i]");
            exit(1);
        }
        strcpy(allUsers[i], temp->username);
        temp = temp->next;
        i++;
    }
    char *result = NULL;
    result = (char*)malloc(i*DEFAULT_BUFLEN);
    if(result == NULL)
    {
        printf("AllUsernames result");
        exit(1);
    }
    for(int j = 0;j < i; ++j)
    {
        strcat(result, allUsers[j]);
        free(allUsers[j]);
        strcat(result, "-");
    }

    int len = strlen(result);
    if (len > 0)
    {
        memmove(&result[len-1], &result[len], 1);
    }

    return result;
}
