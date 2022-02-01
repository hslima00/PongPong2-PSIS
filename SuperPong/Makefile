################################################################################
# Makefile
# Descrição:
# forma simples e intuitiva de compilar o código, de fácil edição.
################################################################################
# Nomes de variaveis usadas ao longo do Makefile 
################################################################################

# variaveis do compilador
CC = gcc
MODE = -g
CFLAGS = -Wall -pedantic -Wextra

# Variaveis para limpeza de ficheiros inerentes a geracao do executavel 
C_TRASH = *.o *.so
OBJS_CTRASH = $(addprefix , $(C_TRASH))
SO_CTRASH = $(addprefix , $(C_TRASH))

# Nomes para os executaveis 
SERVER_NAME = SERVER.proj
CLIENT_NAME = CLIENT.proj

PROG_NAME = $(SERVER_NAME) $(CLIENT_NAME) 
# Nomes para as fontes e objetos 
SERVER_OBJS = server.o 
SERVER_SOURCES =server.c 

CLIENT_OBJS = client.o
CLIENT_SOURCES = client.c 

################################################################################
################################################################################
.PHONY: default server SERVER CLIENT client 
default: $(PROG_NAME)

server: SERVER
SERVER: $(SERVER_NAME)

client: CLIENT
CLIENT: $(CLIENT_NAME)

################################################################################
# Compilacao e geracao de executaveis                               
################################################################################
$(SERVER_NAME):  $(SERVER_OBJS) 
	$(CC) $(MODE)  $(CFLAGS) -o $@  $(SERVER_OBJS) -lcurses -lncurses

$(CLIENT_NAME):  $(CLIENT_OBJS)
	$(CC) $(MODE) -L. $(CFLAGS) -o $@ $(CLIENT_OBJS) -lcurses -lncurses

server.o: $(SERVER_SOURCES)
	$(CC) $(MODE) $(CFLAGS) -c $< -o $@ -lcurses -lncurses

client.o: $(CLIENT_SOURCES)
	$(CC) $(MODE) $(CFLAGS) -c $< -o $@ -lcurses -lncurses


################################################################################
# limpeza de ficheiros inerentes a geracao do executavel  
################################################################################
.PHONY: clean cleanall
 
cleanall: clean cleanExe  

clean:
	rm -rf $(C_TRASH)  

cleanExe:
	rm -rf $(PROG_NAME)

