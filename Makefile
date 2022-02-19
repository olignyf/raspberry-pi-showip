TYPE=APPLICATION

DEPEND=dependencies

SRC=$(wildcard toolbox-*.c)

CFLAGS+=-D_FILE_OFFSET_BITS=64 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include

LDFLAGS+=-L./libs

OBJS = standalone.o
OBJS+= toolbox-filesystem.o
OBJS+= toolbox-line-parser.o
OBJS+= toolbox-char-array.o
OBJS+= toolbox-flexstring.o
OBJS+= toolbox-text-buffer-reader.o
OBJS+= toolbox-tree.o
OBJS+= toolbox.o

TARGET=standalone

all: $(DEPEND) $(TARGET) library

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

library: 
	gcc -Wall `pkg-config --cflags gtk+-2.0 lxpanel` -shared -fPIC showip.c toolbox.c toolbox-char-array.c toolbox-line-parser.c toolbox-text-buffer-reader.c -o showip.so `pkg-config --libs lxpanel`

clean:
	rm -f $(OBJS) showip.so $(TARGET) $(DEPEND)

$(DEPEND): $(SRC)
	@echo 'Creating dependencies files'
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $(SRC) \
		| sed '\''s@\(.*\)\.o[ :]@\1.o: Makefile@g'\'' \
		>$(DEPEND)'

include $(DEPEND)
