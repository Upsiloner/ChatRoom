CC=g++
CFLAGS=-Wall -g
TH=-lpthread
PA=-lmysqlclient -L/var/lib/mysql

files:
	$(CC) $(CFLAGS) sqlhelper.h sqlhelper.cpp server.cpp -o server $(TH) $(PA)
	$(CC) $(CFLAGS) client.cpp -o client $(TH)

clean: 
	rm server client
 