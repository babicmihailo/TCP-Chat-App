#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>
#include <time.h>

#define DEFAULT_BUFLEN 512
#define SERVER_PORT    20186

_Bool Port(char* port);
char* InputUsername(char* allUsernames);
void Login(int sock);
void Menu(char* allUsernames, char* username, int sock);
void MenuSelection(char* allUsernames, char* username, int sock);
void Selection(char* allUsernames, char* username, char talkToUsername[], int numOption, int sock);
int UsernameAvailable(char* allUsernames, char delimiter, char* login);
void ActiveUsers(char* username, int numOption, int sock);
void TalkTo(char* username, char* allUsernames, char* talkToUsername, int sock);
void ChatSelection(char* username, char* allUsernames, char* talkToUsername, int sock);
void* SendDataThread(void* vargp);
void SendData(int sock);
void* ReceiveData(void* vargp);


int active = 1;


int main(int argc , char *argv[])
{
    system("clear");

    int sock, port;
    struct sockaddr_in server;

    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    if(!Port(argv[1]))
    {
        printf("Port must be an integer.\n");
        exit(1);
    }

    port = atoi(argv[1]);
    if(port == SERVER_PORT)
    {
        printf("Server is on this port, please use another one\n");
        exit(1);
    }
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connect failed");
        return 1;
    }

    puts("Connected");

    if(send(sock, &port, DEFAULT_BUFLEN, 0) < 0)
    {
        puts("Send failed");
        return 1;
    }

    Login(sock);

    close(sock);

    return 0;
}

_Bool Port(char* port) //Da li je port ceo broj
{
    char* end;
    strtol(port, &end, 10);
    return *end == '\0';
}

char* InputUsername(char* allUsernames) //Funkcija za vadjenje username-a iz login ulaza i provera da li postoji
{
    puts("format: login <username>");
    char* login = NULL;
    login = (char *)malloc(DEFAULT_BUFLEN);
    scanf("%[^\n]ms", login);
    getchar();
    int len = strlen(login);
    if (len < 7 || strncmp(login, "login ", 6) != 0 || len > 20)
    {
        system("clear");
        puts("You misspelled or too short/long string");
        free(login);
        InputUsername(allUsernames);
    }
    else
    {
        memmove(login, login + 6, len - 5);
        if (UsernameAvailable(allUsernames, '-', login) == 0)
        {
            system("clear");
            puts("User is already logged in");
            free(login);
            InputUsername(allUsernames);
        }
        else
        {
            login[len - 6] = '\0';
        }
    }
    return login;
}

void Login(int sock) //Login
{
    char allUsernames[DEFAULT_BUFLEN];
    while(1)
    {
        int size = recv(sock , allUsernames , DEFAULT_BUFLEN , 0);
        if(size == -1)
        {
            perror("recv failed");
            exit(1);
        }
        if(size == DEFAULT_BUFLEN)
        {
            break;
        }
    }

    char* login = InputUsername(allUsernames);
    char username[DEFAULT_BUFLEN];
    strcpy(username, login);
    free(login);

    if(send(sock, username, DEFAULT_BUFLEN, 0) < 0)
    {
        perror("Send failed");
    }
    printf("\nLogin sucessfull.");
    printf("\nWelcome %s.\n\n", username);
    puts("");
    Menu(allUsernames, username, sock);
}

void Menu(char* allUsernames, char* username, int sock) //Menu default
{
    printf("1. TalkTo (username)\n");
    printf("2. Users\n");
    printf("3. Logout\n\n");
    printf("%s> ", username);

    MenuSelection(allUsernames, username, sock);
}

void MenuSelection(char* allUsernames, char* username, int sock) //Biranje opcije iz menija
{
    char option[DEFAULT_BUFLEN];
    scanf("%[^\n]ms", option);
    getchar();

    int numOption = 0;
    if (strcmp(option, "Users") == 0)
    {
        numOption =  1;
    }
    else if (strncmp(option, "TalkTo ", 7) == 0 && strlen(option) > 7)
    {
        memmove(option, option + 7, strlen(option) - 7 + 1);
        numOption = 2;
    }
    else if (strcmp(option, "Logout") == 0)
    {
        numOption = 3;
    }
    else
    {
        Menu(allUsernames, username, sock);
    }

    if(send(sock, &numOption, DEFAULT_BUFLEN, 0) < 0)
    {
        perror("Send failed");
    }

    Selection(allUsernames, username, option, numOption, sock);
}

void Selection(char* allUsernames, char* username, char* talkToUsername, int numOption, int sock) //Prosledjivanje serveru funkcije i izvrsavanje sa klijentske strane
{
    if (numOption == 1)
    {
        ActiveUsers(username, numOption, sock);
    }
    else if (numOption == 2)
    {
        TalkTo(username, allUsernames, talkToUsername, sock);
    }
    else if (numOption == 3)
    {
        Login(sock);
    }
    else
    {
        perror("Invalid action");
    }
}

int UsernameAvailable(char* allUsernames, char delimiter, char* login) // Prima listu svih usera sa servera saljem ih kao jedan dugacak string oa ih ovde rasclanjujem i proveravam dostupnost
{
    char** result = NULL;
    char* temp;
    int count = 0;

    temp = strtok(allUsernames, &delimiter);
    while (temp != NULL)
    {
        result = realloc(result, sizeof(char*) * ++count);
        if (result == NULL) exit(-1);
        result[count-1] = temp;
        temp = strtok(NULL, &delimiter);
    }

    result = realloc(result, sizeof(char*) * (count+1));
    result[count] = 0;

    for(int i=0; i < count; i++)
    {
        if (strcmp(login, result[i]) == 0)
        {
            return 0;
        }
    }
    free(result);

    return 1;
}

void ActiveUsers(char* username, int numOption, int sock) //Printuje aktivne korisnike
{
    char userData[DEFAULT_BUFLEN];
    int size;
    while(1)
    {
        size = recv(sock , userData , DEFAULT_BUFLEN , 0);
        if(size == -1)
        {
            perror("Ovaj recv failed");
            exit(1);
        }
        if(size == DEFAULT_BUFLEN)
        {
            break;
        }
    }

    puts(userData);

    userData[size] = '\0';
    puts("Online Users:");
    puts("---------------------------");
    printf("%s", userData);
    puts("---------------------------");

    char allUsernames[DEFAULT_BUFLEN];
    while(1)
    {
        int size = recv(sock , allUsernames , DEFAULT_BUFLEN , 0);
        if(size == -1)
        {
            perror("recv failed");
            exit(1);
        }
        if(size == DEFAULT_BUFLEN)
        {
            break;
        }
    }
    Menu(allUsernames, username, sock);
}

void TalkTo(char* username, char* allUsernames, char* tempTalkToUsername, int sock) //TalkTo salje zahtev za povezivanje sa drugim korisnikom
{
    char talkToUsername[DEFAULT_BUFLEN];
    strcpy(talkToUsername, tempTalkToUsername);
    int size = 0;

    if(send(sock, talkToUsername, DEFAULT_BUFLEN, 0) < 0)
    {
        perror("Send failed");
    }

    int state = 0;
    while(1)
    {
        size = recv(sock , &state , DEFAULT_BUFLEN, 0);
        if(size == -1)
        {
            perror("recv failed");
            exit(1);
        }
        if(size == DEFAULT_BUFLEN)
        {
            break;
        }
    }

    if (state == 2)
    {
        printf("Waiting for %s to send TalkTo request\n", tempTalkToUsername);
        time_t timer = time(NULL);
        while (difftime(time(NULL), timer) < 15)
        {
            size = recv(sock , &state , DEFAULT_BUFLEN, 0);
            if(size == -1)
            {
                perror("recv failed");
                exit(1);
            }
            if(size == DEFAULT_BUFLEN)
            {
                break;
            }
        }
    }

    if (state == 1)
    {
        printf("Connecting to user %s...\n", talkToUsername);
        ChatSelection(username, allUsernames, talkToUsername, sock);
    }
    else
    {
        puts("Request interrupted or user busy");
    }
    Menu(allUsernames, username, sock);
}

void ChatSelection(char* username, char* allUsernames, char* talkToUsername, int sock) //Pa data je i jedina opcija al ajd napravio sam funkciju
{
    puts("1. Data (Start data transfer between connected users).");
    char option[DEFAULT_BUFLEN];
    scanf("%[^\n]ms", option);
    getchar();

    if (strcmp(option, "Data") == 0)
    {
        puts("Conversation started");

        pthread_t* sendThread;
        pthread_t* receiveThread;

        int *sockp = malloc(DEFAULT_BUFLEN);
        sockp = &sock;

        sendThread = (pthread_t*)malloc(sizeof(pthread_t));
        receiveThread = (pthread_t*)malloc(sizeof(pthread_t));

        pthread_create(sendThread, NULL, SendDataThread, (void *)(sockp));
        pthread_create(receiveThread, NULL, ReceiveData, (void *)(sockp));

        pthread_detach(*receiveThread);
        pthread_join(*sendThread, NULL);


        free(sendThread);
        free(receiveThread);
        free(sockp);
        printf("All done, reseting\n");
        active = 1;
    }
    else
    {
        ChatSelection(username, allUsernames, talkToUsername, sock);
    }
}

void* SendDataThread(void* vargp) //thread za slanje podataka
{
    int sock = *((int *) vargp);
    SendData(sock);
    return NULL;
}

void SendData(int sock)
{
    char data[DEFAULT_BUFLEN];
    scanf("%[^\n]ms", data);
    getchar();
    int size;

    while(active)
    {
        size = send(sock, data, DEFAULT_BUFLEN, 0);
        if(size < 0)
        {
            perror("Send failed");
            exit(1);
        }
        printf("\033[2K\033[1A");
        printf("Sent> %s\n", data);
        char *disconnect = malloc(DEFAULT_BUFLEN);
        disconnect = strstr(data, "Disconnect");
        if (disconnect)
        {
            active = 0;
            puts("Detected Disconnect signal, returning to menu");
            break;
        }

        SendData(sock);
    }
}

void* ReceiveData(void* vargp) //thread za primanje podataka
{
    int sock = *((int *)vargp);
    int size = 0;
    while(active)
    {
        char data[DEFAULT_BUFLEN];
        while(active)
        {
            size = recv(sock , data , DEFAULT_BUFLEN, 0);
            if(size == -1)
            {
                perror("recv failed");
                exit(1);
            }
            if (size == DEFAULT_BUFLEN)
            {
                break;
            }
        }
        printf("Received> %s\n", data);
        char *disconnect = malloc(DEFAULT_BUFLEN);
        disconnect = strstr(data, "Disconnect");
        if (disconnect)
        {
            active = 0;
            puts("Detected Disconnect signal, returning to menu");
            break;
        }
    }
    return NULL;
}

