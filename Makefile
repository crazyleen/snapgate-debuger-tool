
TOPDIR = $(shell pwd)
BUILDDIR ?= $(TOPDIR)/build
PREFIX ?= $(TOPDIR)/bin

#CROSS_COMPILE ?= arm-none-linux-gnueabi-
#CROSS_COMPILE ?= arm-arago-linux-gnueabi-
#CROSS_COMPILE ?= arm-linux-
CC=$(CROSS_COMPILE)gcc
CFLAGS = -g -Wall
LDFLAGS := 
LIBS :=
INCLUDES := -I$(TOPDIR)

VPATH = $(BUILDDIR)

#export 
export CC
export CFLAGS
export INCLUDES
export TOPDIR


APP=snapgate-debuger-client snapgate-debuger-server
all: $(APP) 

OBJECTSCLIENT = packet.o client_functions.o file_transfer.o client.o
snapgate-debuger-client:	${OBJECTSCLIENT}
	@@cd $(BUILDDIR) && cd $(BUILDDIR) && $(CC) $(INCLUDES)  ${CFLAGS}  $^ -o $@ ${LDFLAGS}

OBJECTSSERVER = packet.o server_functions.o file_transfer.o server.o xpopen.o mspdebug.o util.o
snapgate-debuger-server:	${OBJECTSSERVER}
	@@cd $(BUILDDIR) && cd $(BUILDDIR) && $(CC) $(INCLUDES)  ${CFLAGS}  $^ -o $@ ${LDFLAGS} -lpthread
	
.c.o:
	@$(CC) -c $(CFLAGS) $(INCLUDES)   $< -o $(BUILDDIR)/$@

install:
	@cd $(BUILDDIR) && cp  $(APP) $(PREFIX)
	

clean:
	@cd $(BUILDDIR) && rm -f *.o $(APP)
	@cd $(PREFIX) && rm -f $(APP)

.PHONY: clean install
