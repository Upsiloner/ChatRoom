#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<arpa/inet.h>
#include<sys/socket.h>
 
#define BUF_SIZE 1024
#define NAME_SIZE 20

char name[NAME_SIZE]="[DEFAULT]";   // default name is "[DEFAULT]"
char msg[BUF_SIZE]; 
 
void error_handing(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

// used for sending message
void *send_msg(void *arg){
    int sock=*((int*)arg);
    char name_msg[NAME_SIZE+BUF_SIZE];
    while(1){
        // the message you enter will go into msg
        fgets(msg,BUF_SIZE,stdin);
        // input p or P to quit
        if(!strcmp(msg,"q\n") || !strcmp(msg,"Q\n")){
            printf("Disconnected......\n");
            close(sock);
            exit(0);
        }
        // send your name and message into name_msg, and send to the server
        sprintf(name_msg,"%s %s",name,msg);
        //printf("have send:%s", name_msg);
        write(sock,name_msg,strlen(name_msg));
    }
    return NULL;
}

// used for receiving message
void *recv_msg(void *arg){
    int sock=*((int*)arg);
    char name_msg[NAME_SIZE+BUF_SIZE];
    int str_len;
    while(1){
        str_len=read(sock,name_msg,NAME_SIZE+BUF_SIZE-1);
        // set 0 can move the excess data
        name_msg[str_len]=0;
        // cout the message to the console
        fputs(name_msg, stdout);
    }
    return NULL;
}

// Login screen
void screen_info(int sock, char *id){
    char* room_id = id;
    char n[20], p[20], res[20];
    int len;
    write(sock, room_id, sizeof(room_id));
    while(1){
        printf("==== Tourist landing ====\n");
        
        printf("Input your username: ");
        fgets(n,20,stdin);
        // remove the buffer data
        printf("Input your password: ");
        fgets(p,20,stdin);
        write(sock,n,20);
        write(sock,p,20);
        len=strlen(n);
        n[len-1]=0;
        sprintf(name,"[%s]", n);

        len=read(sock,res,1);
        res[1]=0;
        if(len>0){
            if(res[0]=='2') {
                printf("Success enroll!\n");
                break;
            }
            else if(res[0]=='1') {
                printf("Success login!\n");
                break;
            }
            else {
                printf("[Warn] wrong password or background\n");
                continue;
            }
        }
        else printf("Error reading data\n");
    }
    len=read(sock,res,1);
    if(res[0]=='3') {
        printf("Data initialization succeeded\n");
    }
    else printf("[Warn] Data initialization error\n");
    
}

int main(int argc, char* argv[]){
    if(argc!=4){
        printf("Usage: %s <IP> <port> <room_id>\n", argv[0]);
        exit(1);
    }

    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread, screen_th;
    void *thread_return;
    int str_len, recv_len, recv_cnt;

    // socket, Connection-oriented��SOCK_STREAM��
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock==-1){
        error_handing("socket() error");
    }

    // initialize the address information
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
    serv_addr.sin_port=htons(atoi(argv[2]));

    // Begin a connection request
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
        error_handing("connect() error"); 
    else puts("Connected......");

    screen_info(sock, argv[3]);

    // The thread used to send and receive messages
    pthread_create(&snd_thread,NULL,send_msg,(void*)&sock);
    pthread_create(&rcv_thread,NULL,recv_msg,(void*)&sock);
    pthread_join(snd_thread,&thread_return);
    pthread_join(rcv_thread,&thread_return);

    close(sock);
    return 0;
}