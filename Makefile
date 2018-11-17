REQALL =  message.o my_util.o
REQCLI = client.o 
SRVFILE = server.o
all:	$(REQCLI) $(SRVFILE) $(REQALL)
		g++ -g -o client $(REQCLI) $(REQALL) -lm -lpthread         
		g++ -g -o server $(SRVFILE) $(REQALL) -lm -lpthread         
message.o: message.c message.h
		g++ -g -c message.c		
my_util.o: my_util.c my_util.h
		g++ -g -c my_util.c		 
client.o: client.c message.h my_util.h
		g++ -g -c client.c 
server.o: server.c message.h my_util.h
		g++ -g -c server.c 
clean:
		rm client $(REQCLI) server $(SRVFILE) $(REQALL)