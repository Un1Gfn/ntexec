MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables

SSH_DEPLOY_TO?=192.168.1.13
DESTDIR?=/usr/local

CONF:=.ntexec
CLIENT:=ntexec.out
SERVER:=win_server.exe

MM:=x86_64-w64-mingw32-gcc
CC:=gcc

CFLAGS:=-std=gnu11 -g -O0 -Wall -Wextra -Winline -Wdeprecated-declarations
MFLAGS:=-std=c11 -g -O0 -Wall -Wextra -Winline -Wdeprecated-declarations
LDFLAGS:=-static

# default: ;
# default: $(CLIENT)
# default: $(SERVER)
default: $(CLIENT) $(SERVER)

cscope:
	cscope -I/usr/x86_64-w64-mingw32/include/ -1 $(id) $(file)

$(SERVER): LDLIBS+=-lws2_32   # /usr/x86_64-w64-mingw32/lib/libws2_32.a
$(SERVER): LDLIBS+=-liphlpapi # /usr/x86_64-w64-mingw32/lib/libiphlpapi.a
%.exe: %.c def.h
	$(MM) $(MFLAGS) $(LDFLAGS) -o $@ $(filter %.c , $^ ) $(LDLIBS)
	scp $@ $(SSH_DEPLOY_TO):C:\\Users\\%USER%\\$@ &

$(CLIENT): CFLAGS+=-DCONF=\"$(CONF)\"
%.out: %.c def.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(filter %.c , $^ ) $(LDLIBS)
	@sudo rm -rfv $(DESTDIR)/bin/$(CLIENT)
	@sudo cp -v $(CLIENT) $(DESTDIR)/bin/$(CLIENT)

test: $(CLIENT)
	env CONF="$(CONF)" CLIENT="$(CLIENT)" ./test.sh 

release: $(CLIENT) $(SERVER)
	./release.sh $(CLIENT) $(SERVER)

clean: 
	@rm -fv *.exe *.out  # *.h.gch
	ssh $(SSH_DEPLOY_TO) del C:\\Users\\%USER%\\win_server.exe &
	ssh $(SSH_DEPLOY_TO) del C:\\Users\\%USER%\\win_test.exe &

# https://stackoverflow.com/questions/3141738/duplicating-stdout-to-stderr
# stress:
# 	./$(CLIENT) 192.168.1.13 "$(</dev/urandom tr -dc "[:alnum:]" | head -c1024 | tee /dev/stderr)"
# 	./$(CLIENT) 192.168.1.13 "$(</dev/urandom tr -dc "[:alnum:]" | head -c1025)"
