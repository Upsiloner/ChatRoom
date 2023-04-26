#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h> 
#include<pthread.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<error.h>
#include "sqlhelper.h" 

#define BUF_SIZE 1024
#define MAX_CLNT 256

int clnt_cnt=0;     // index of every clients in clnt_socks
int clnt_socks[MAX_CLNT];   
char room_id[20];
char init_buf[4096];
pthread_mutex_t mutex;
pthread_cond_t cond;    // condition

bool ready = false;
SqlHelper sql;

void error_handing(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}


// judeg and send back the information about the login
void handle_user(int clnt_sock){
    int judge;

    read(clnt_sock,room_id,20);
    //fputs(room_id, stdout);

    while(1){
        judge = sql.login_judge(clnt_sock);
        if(judge==CR_SUCCS){
            write(clnt_sock,"2",1);
            break;
        }
        else if(judge==UP_RIGHT){
            write(clnt_sock,"1",1);
            break;
        }
        else if(judge==HD_ERROR){
            printf("Disconnected client fd [%d]\n", clnt_sock);
            close(clnt_sock);
            break;
        }
        else{
            write(clnt_sock,"0",1);
            continue;
        }
        if(judge==PS_ERROR){
            write(clnt_sock,"0",1);
        }
    }
    judge=sql.record_init(room_id, init_buf);
    if(judge==HD_SUCCS) {
        write(clnt_sock,"3",1);
        write(clnt_sock, init_buf, strlen(init_buf));

    }
    else write(clnt_sock,"4",1);
}

//broadcast the message
void send_msg(char*msg,int len){
    pthread_mutex_lock(&mutex);
    // send message to every clients in the clnt_socks
    for(int i=0;i<clnt_cnt;++i){
        write(clnt_socks[i],msg,len);
    }
    pthread_mutex_unlock(&mutex);
}


// handle the msssage from client
void *handle_clnt(void *arg){
    
    int clnt_sock=*((int*)arg);

    pthread_mutex_t out_mtx;
    pthread_mutex_init(&out_mtx, NULL);

    pthread_mutex_lock(&out_mtx); //lock
    handle_user(clnt_sock);
    ready=true;
    pthread_cond_signal(&cond);  // tell t_id thread now you can execute
    pthread_mutex_unlock(&out_mtx);  // unlock

    pthread_mutex_lock(&out_mtx); // lock
    while (!ready) {
        pthread_cond_wait(&cond, &out_mtx);  // waiting for thread user_th
    }
    int str_len=0, i=0;   // str_len: receive the length of the message
    char msg[BUF_SIZE];
    // if the server can receive the message, then broadcast the message
    while((str_len=read(clnt_sock,msg,sizeof(msg)))!=0){ 
        //printf("have received:%s", msg); 
        // get the content and judge
        while(msg[i]!=' ') ++i;
        if((msg[i+1]=='c' || msg[i+1]=='C') && msg[i+2]=='\n'){
            sql.clear_content(room_id);
            char *back = "Record cleanup succeeded\n";
            send_msg(back, strlen(back));
            continue;
        }
        send_msg(msg,str_len);
        // write chat contents to the sql
        msg[str_len-1]=0;
        sql.write_content(room_id, msg);
    }
    /* If break the while, means this chat is over. 
       Then update the clnt_socks. */
    printf("Disconnected client fd [%d]\n", clnt_sock);
    pthread_mutex_lock(&mutex);
    for(i=0;i<clnt_cnt;++i){
        if(clnt_sock==clnt_socks[i]){
            while(i++<clnt_cnt-1){
                clnt_socks[i]=clnt_socks[i+1];
            }
            break;
        }
    }
    --clnt_cnt; 
    pthread_mutex_unlock(&mutex);
    pthread_mutex_unlock(&out_mtx);  // unlock
    close(clnt_sock);
    return NULL;
}


int main(int argc, char* argv[]){
    // test the input format
    if(argc!=2){
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    void *thread_return;
    int option = 1;
    

    pthread_t t_id, user_th;
    pthread_mutex_init(&mutex, NULL);

    // socket, Connection-oriented��SOCK_STREAM��
    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1){
        error_handing("socket() error");
    }
    // initialize the address information
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    //serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_addr.s_addr=inet_addr(IP_ADDR);
    serv_addr.sin_port=htons(atoi(argv[1]));

    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if(bind(serv_sock,(struct sockaddr*) &serv_addr,sizeof(serv_addr))==-1)
        error_handing("bind() error");
    
    if(listen(serv_sock, 5)==-1)
        error_handing("listen() error");

    while(1){
        clnt_addr_size=sizeof(clnt_addr);
        clnt_sock=accept(serv_sock,(struct sockaddr*)&clnt_addr,&clnt_addr_size);
        printf("Connected client IP: %s, fd [%d]\n", inet_ntoa(clnt_addr.sin_addr), clnt_sock);

        pthread_mutex_lock(&mutex);
        // add file descriptor into clnt_socks which holds all descriptors
        clnt_socks[clnt_cnt++]=clnt_sock;
        pthread_mutex_unlock(&mutex);
     
        pthread_create(&t_id,NULL,handle_clnt,(void*)&clnt_sock);   
        pthread_detach(t_id);
        
    }
    close(serv_sock);
    return 0;
}
