LIBS    = -lnsl -lcurl -lstdc++ 
TESTLIBS    = -lpthread -lresolv -lnsl -lcurl -lefence

DEFS   = -D_REENTRANT -D_POSIX_C_SOURCE -D__EXTENSIONS__ -D_POSIX_PTHREAD_SEMANTICS -D_POSIX_MAPPED_FILES -D_SVID_SOURCE -D_XOPEN_SOURCE=500

CC     = gcc
CPP     = g++
CFLAGS = -Wall -pedantic -g3 
CPPFLAGS = -g -c 
C_CFILES = rhttp.c gethttp_clnt.c gethttp_xdr.c
C_HFILES = gethttp.h
P_CFILES = gethttp_proc.c gethttp_svc.c gethttp_xdr.c
P_CACHEFILES = cache.o
P_CPPFILES = cache.cpp
P_HFILES = gethttp.h
RPCGEN   = rpcgen
# RPCGEN_FLAGS = -M -C -K -1
RPCGEN_FLAGS = 

RPC_FILES = gethttp_svc.c gethttp_xdr.c gethttp.h gethttp_clnt.c

all: server client 

client: $(C_CFILES) $(C_HFILES)
	$(CC) -o $@ $(CFLAGS) $(DEFS) $(C_CFILES) $(LIBS) -g

server: $(P_CFILES) $(P_HFILES) $(P_CPPFILES)  $(P_CACHEFILES)
	$(CC) -o $@ $(CFLAGS) $(DEFS) $(P_CFILES) $(P_CACHEFILES) $(LIBS) -g

$(P_CACHEFILES) : $(P_CPPFILES)
	$(CPP) $(CPPFLAGS) $(P_CPPFILES)


$(RPC_FILES): gethttp.x
	$(RPCGEN) $(RPCGEN_FLAGS) $<

%.h : ;

clean:
	rm -f server client $(RPC_FILES) *.o

