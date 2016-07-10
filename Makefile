TYPE=APPLICATION

DEPEND=dependencies

SRC=$(wildcard *.c)

CFLAGS+=-D_FILE_OFFSET_BITS=64
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

all: $(DEPEND) $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET) $(DEPEND)

$(DEPEND): $(SRC)
	@echo 'Creating dependencies files'
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS) $(SRC) \
		| sed '\''s@\(.*\)\.o[ :]@\1.o: Makefile@g'\'' \
		>$(DEPEND)'

include $(DEPEND)
