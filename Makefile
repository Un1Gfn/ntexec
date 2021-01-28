MAKEFLAGS+=--no-builtin-rules
MAKEFLAGS+=--no-builtin-variables

SSH_DEPLOY_TO?=192.168.1.13
DESTDIR?=/usr/local

CONF:=.ntexec
CLIENT:=ntexec.out
SERVER:=win_server.exe

MM:=x86_64-w64-mingw32-gcc
CC:=gcc

# https://stackoverflow.com/a/34971392
# gcc -Wall -Wextra -Q --help=warning | grep -Fv [enabled] | less -SRM +%
W:= \
-Wdeprecated-declarations \
-Winit-self \
-Winline \
-Wmaybe-uninitialized \
-Wshadow \
-Wuninitialized

# https://www.gnu.org/software/make/manual/html_node/Options_002fRecursion.html
# $(MFLAGS) has some other meaning
M_FLAGS:=-std=c17 -g -O0 -Wall -Wextra $(W) $(M_FLAGS)
CFLAGS:=-std=gnu17 -g -O0 -Wall -Wextra $(W) $(CFLAGS)

# default: ;
# default: $(CLIENT)
# default: $(SERVER)
default: install

clean:
	@rm -fv *.exe *.out *.gch
	@sudo rm -rfv $(DESTDIR)/bin/$(CLIENT)
	ssh $(SSH_DEPLOY_TO) del C:\\Users\\%USER%\\win_server.exe &
	ssh $(SSH_DEPLOY_TO) del C:\\Users\\%USER%\\win_test.exe &

$(CLIENT): CFLAGS+=-DCONF=\"$(CONF)\"
$(CLIENT): sock.c def.h
%.out: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(filter %.c , $^ ) $(LDLIBS)

$(SERVER): LDLIBS+=-lws2_32   # /usr/x86_64-w64-mingw32/lib/libws2_32.a
$(SERVER): LDLIBS+=-liphlpapi # /usr/x86_64-w64-mingw32/lib/libiphlpapi.a
$(SERVER): def.h
%.exe: %.c
	$(MM) $(M_FLAGS) $(LDFLAGS) -o $@ $(filter %.c , $^ ) $(LDLIBS)

install:
	@sudo /bin/true
	@$(MAKE) -B $(CLIENT) $(SERVER)
	sudo cp -v $(CLIENT) $(DESTDIR)/bin/$(CLIENT)
	scp $(SERVER) $(SSH_DEPLOY_TO):C:\\Users\\%USER%\\$(SERVER) &

release: 
release:
	@env LDFLAGS=-static $(MAKE) -B $(CLIENT) $(SERVER)
	./release.sh $(CLIENT) $(SERVER)

# cscope:
# 	cscope -I/usr/x86_64-w64-mingw32/include/ -1 $(id) $(file)

# https://stackoverflow.com/questions/3141738/duplicating-stdout-to-stderr
# stress:
# 	./$(CLIENT) 192.168.1.13 "$(</dev/urandom tr -dc "[:alnum:]" | head -c1024 | tee /dev/stderr)"
# 	./$(CLIENT) 192.168.1.13 "$(</dev/urandom tr -dc "[:alnum:]" | head -c1025)"

# test: $(CLIENT)
# 	env CONF="$(CONF)" CLIENT="$(CLIENT)" ./test.sh 
