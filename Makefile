

#CROSS_COMPILE ?= arm-none-linux-gnueabi-
#CROSS_COMPILE ?= arm-arago-linux-gnueabi-
#CROSS_COMPILE ?= arm-linux-
CC=$(CROSS_COMPILE)gcc
CFLAGS = -o1 -Wall
LDFLAGS := 
LIBS :=
INCLUDES := 

APP=snapgate-debuger-client snapgate-debuger-server
all: $(APP) 

OBJECTSCLIENT = packet.o client_functions.o file_transfer.o client.o
snapgate-debuger-client:	${OBJECTSCLIENT}
	@$(CC) $(INCLUDES)  ${CFLAGS}  $^ -o $@ ${LDFLAGS}

OBJECTSSERVER = packet.o server_functions.o file_transfer.o server.o xpopen.o mspdebug.o util.o
snapgate-debuger-server:	${OBJECTSSERVER}
	@$(CC) $(INCLUDES)  ${CFLAGS}  $^ -o $@ ${LDFLAGS} -lpthread
	
.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

install:
	@cp snapgate-debuger-client mspdebug-snapgate
	@cp snapgate-debuger-server mspdebug-server

clean:
	@rm -f *.o $(APP)
	@rm -f $(APP) mspdebug-snapgate mspdebug-server

.PHONY: clean install
