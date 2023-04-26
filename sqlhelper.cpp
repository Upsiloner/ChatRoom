#include "sqlhelper.h"

SqlHelper::SqlHelper(){
    // --- create the database --- 
    conn = mysql_init(NULL); // initial connect
    if (conn == NULL) {
        printf("mysql_init() failed\n");
        exit(0);
    }
    // connect to the database
    if (mysql_real_connect(conn, IP_ADDR, "root", "zhangyu123", "chatroom_info", 0, NULL, 0) == NULL) { 
        printf("mysql_real_connect() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        exit(0);
    } 


    /* Before you begin, you need to:
       >> CREATE DATABASE IF NOT EXISTS chatroom_info;
       >> use chatroom_info;
       >> CREATE TABLE IF NOT EXISTS users (name VARCHAR(20), password INT);
       >> CREATE TABLE IF NOT EXISTS histor (int INT, content VARCHAR(1024));
    */

    printf("Success connect to mysql\n");
}

int SqlHelper::login_judge(int clnt_sock){
    char n[20], p[20], query[100];
    int len, num_rows, real_len;

    len=read(clnt_sock,n,20);
    real_len=strlen(n);
    n[real_len-1]=0;
    //fputs(n, stdout);
    // char str[10];
    // sprintf(str, "%d", len);
    // fputs(str, stdout);
    len=read(clnt_sock,p,20);
    real_len=strlen(p);
    p[real_len-1]=0;
    //fputs(p, stdout);

    sprintf(query, "SELECT * FROM users WHERE name = '%s'", n); 

    if (mysql_query(conn, query)) {  // Execute the query
        printf("mysql_query() failed: %s\n", mysql_error(conn));
        return HD_ERROR;   
    }
    result = mysql_store_result(conn);  // get the result
    num_rows = mysql_num_rows(result);  // get the number of that name

    if(num_rows==0){
        // if there is no that name, insert the name and password.
        sprintf(query, "INSERT INTO users (name, password) VALUES ('%s', %s)", n, p); 
        if (mysql_query(conn, query)) {
            printf("mysql_query() failed: %s\n", mysql_error(conn));
            return HD_ERROR;
        }
        mysql_free_result(result);
        return CR_SUCCS;
    }
    else{
        // Iterate through the query results. have a same name, found = true
        while ((row = mysql_fetch_row(result)) != NULL) {
            if (strcmp(row[1], p) == 0) {
                mysql_free_result(result);
                return UP_RIGHT;
            }
        } 
    }
    mysql_free_result(result);  // free the result

    //while ((c = getchar()) != '\n' && c != EOF);
    return PS_ERROR;
}



void SqlHelper::write_content(char* id, char* msg) {
    char query[1024];
    sprintf(query, "INSERT INTO history (id, content) VALUES (%d, '%s')", atoi(id), msg); 
    //printf("%s--\n", query);
    if (mysql_query(conn, query)) {
        printf("mysql_query() failed: %s\n", mysql_error(conn));
        return;
    }
}

void SqlHelper::clear_content(char* id) {
    char query[1024];
    sprintf(query, "DELETE FROM history WHERE id = %d", atoi(id)); 
    if (mysql_query(conn, query)) {
        printf("mysql_query() failed: %s\n", mysql_error(conn));
        return;
    }
}


int SqlHelper::record_init(char* id, char* init_buf) {
    char query[1024];
    strcpy(init_buf, "");
    sprintf(query, "SELECT content FROM history WHERE id = %d", atoi(id)); 
    if (mysql_query(conn, query)) {
        printf("mysql_query() failed: %s\n", mysql_error(conn));
        return HD_ERROR;
    }
    result = mysql_use_result(conn);
    while ((row = mysql_fetch_row(result)) != NULL) {
        strcat(init_buf, row[0]);
        strcat(init_buf, "\n");
        //printf("%s\n", row[0]);
    }
    mysql_free_result(result);
    return HD_SUCCS;
}
