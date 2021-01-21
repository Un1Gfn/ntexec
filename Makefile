CLIENT:=linux_client.out
SERVER:=win_server.exe

# default: ;
# default: $(CLIENT)
# default: $(SERVER)
default: $(CLIENT) $(SERVER)

MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables

MM:=x86_64-w64-mingw32-gcc
CC:=gcc

CFLAGS:=-std=gnu11 -g -O0 -Wall -Wextra -Wno-unused-parameter -Winline -Wdeprecated-declarations

T:=~/cgi/cgi-tmp

cscope:
# 	cscope -I/usr/x86_64-w64-mingw32/include/ -1 $(id) $(file)
	cscope -I/usr/x86_64-w64-mingw32/include/ -1 $(id) $(SERVER)

$(SERVER): LDLIBS:=-lws2_32
%.exe: %.c
	$(MM) $(CFLAGS) $(LDFLAGS) -o $@ $(filter %.c , $^ ) $(LDLIBS)
	@rm -fv $(T)/$@
	@cp -v $@ $(T)/$@

$(CLIENT):
%.out: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(filter %.c , $^ ) $(LDLIBS)

clean: 
	@rm -fv *.exe *.out  # *.h.gch
	@rm -fv $(T)/$(SERVER)
