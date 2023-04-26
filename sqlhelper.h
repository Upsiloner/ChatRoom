#ifndef SQL_HELPER
#define SQL_HELPER
 
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<mysql/mysql.h>
 
#define IP_ADDR "127.0.0.1"

#define UP_RIGHT 1000   // name and password are all right
#define CR_SUCCS 1001   // create new name and password successfully
#define PS_ERROR 1002   // have the name, but the password is error
#define HD_ERROR 1005   // handle error
#define HD_SUCCS 1006   // handle success
 
class SqlHelper {
public:
    SqlHelper();

    int login_judge(int clnt_sock);
    void write_content(char* id, char* msg);
    void clear_content(char* id);
    int record_init(char* id, char* init_buf);

private:
    MYSQL* conn;
    MYSQL_RES *result;
    MYSQL_ROW row;
};

#endif