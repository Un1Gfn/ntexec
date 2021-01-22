# T:=~/cgi/cgi-tmp
T?=192.168.1.13

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
MFLAGS:=-std=c11 -g -O0 -Wall -Wextra -Wno-unused-parameter -Winline -Wdeprecated-declarations

cscope:
# 	cscope -I/usr/x86_64-w64-mingw32/include/ -1 $(id) $(file)
	cscope -I/usr/x86_64-w64-mingw32/include/ -1 $(id) $(SERVER)

$(SERVER):
%.exe: LDLIBS+=-lws2_32 -liphlpapi
%.exe: %.c
	$(MM) $(MFLAGS) $(LDFLAGS) -o $@ $(filter %.c , $^ ) $(LDLIBS)
	scp $@ $T:C:\\Users\\%USER%\\$@ &
# 	@rm -fv $T/$@
# 	@cp -v $@ $T/$@

$(CLIENT):
%.out: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(filter %.c , $^ ) $(LDLIBS)

clean: 
	@rm -fv *.exe *.out  # *.h.gch
	ssh $T del C:\\Users\\%USER%\\win_server.exe &
	ssh $T del C:\\Users\\%USER%\\win_test.exe &
# 	@rm -fv $T/$(SERVER)

# https://stackoverflow.com/questions/3141738/duplicating-stdout-to-stderr
# stress:
# 	./linux_client.out 192.168.1.13 "$(</dev/urandom tr -dc "[:alnum:]" | head -c1024 | tee /dev/stderr)"
# 	./linux_client.out 192.168.1.13 "$(</dev/urandom tr -dc "[:alnum:]" | head -c1025)"
