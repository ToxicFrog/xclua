# Lua interpreter and compiler
LUA=lua
LUAC=luac

# include paths
I=-I. -I/usr/local/include
# library paths
L=-L/usr/local/lib
# library names
l=-llua

# C compiler and linker
CC=gcc
LD=gcc

# flags for optimized compilation and linking
CCFLAGS=-O2 -shared -Wall -std=gnu99 -c
LDFLAGS=-O2 -shared -Wall -std=gnu99
STRIP=strip

# flags for debugging compilation and linking
#CCFLAGS=-g -shared -Wall -std=gnu99 -c -DDEBUG
#LDFLAGS=-g -shared -Wall -std=gnu99
#STRIP=@true


OBJS=xclua.o xclua_callbacks.o xclua_context.o xclua_hooks.o xclua_io.o xclua_list.o xclua_misc.o

all: linux

linux: xclua.so

windows:
	@make \
	"CCFLAGS=$(CCFLAGS) -DWIN32" \
	xclua.dll

my-windows:
	@make \
	"CCFLAGS=$(CCFLAGS) -DWIN32" \
	"LUA=../../bin/mingw/lua" \
	"LUAC=../../bin/mingw/luac" \
	"L=-L../../lib" \
	"I=-I. -I../../include/lua" \
	"l=-llua-mingw" \
	clean xclua.dll

xclua.so: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $L $l
	$(STRIP) --strip-unneeded $@

xclua.dll: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $L $l
	$(STRIP) --strip-unneeded $@

xclua.lb: xclua.lua
	$(LUAC) -o xclua.lc xclua.lua
	$(LUA) lua2c.lua xclua_lua < xclua.lc > xclua.lb

%.o:
	$(CC) $(CCFLAGS) -o $@ $*.c $I

clean:
	rm -f *.lc *.lb *.o *.dll *.so

package: xclua.tar.gz

xclua.tar.gz: *.c *.h *.lua README COPYING Makefile
	tar czvf $@ README COPYING Makefile *.c *.h *.lua

.SECONDARY:
.DELETE_ON_ERROR:
.PHONY: all windows linux package

xclua.o: xclua.c xclua.h xclua_callbacks.h xclua_hooks.h xclua_io.h xclua_misc.h xclua.lb
xclua_callbacks.o: xclua_callbacks.c xclua_callbacks.h xclua.h
xclua_context.o: xclua_context.c xclua_context.h xclua.h
xclua_hooks.o: xclua_hooks.c xclua_hooks.h xclua.h xclua_callbacks.h
xclua_io.o: xclua_io.c xclua_io.h xclua.h
xclua_list.o: xclua_list.c xclua_list.h xclua.h
xclua_misc.o: xclua_misc.c xclua_misc.h xclua.h
