TARGET	=	main

SRC 	=	src
BIN		=	bin

CC		=	gcc
STD 	=	c99

PROFILE	=	
LIBS	= -llept

CFLAGS	=	-Wall -pedantic -g $(PROFILE) -std=$(STD) -I$(SRC)


SOURCES	=	$(notdir $(wildcard $(SRC)/**/*.c $(SRC)/*.c))
OBJECTS =	$(patsubst %.c, $(BIN)/%.o, $(SOURCES))

all: $(BIN) $(OBJECTS) $(TARGET)

$(BIN):
	@echo " "
	@echo "[Create Folders]"
	[ ! -e $@ ] && mkdir $@
	
$(BIN)/%.o: $(SRC)/%.c
	@echo " "
	@echo "["$^"]"
	$(CC) $(CFLAGS) -c $< -o $@
	@if [ $$? != 0 ]; then rm $@; fi
	
$(BIN)/%.o: $(SRC)/%.h
	@echo " "
	@echo "["$^"]"
	$(CC) $(CFLAGS) -c $? -o $@
	@if [ $$? != 0 ]; then rm $@; fi

$(TARGET): $(OBJECTS)
	@echo " "
	@echo "[Linking]"
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

clean:
	-@rm $(BIN)/* $(TARGET) cachegrind.* *.out && echo "ALL CLEAN" || echo "ALL CLEAN"
	
memcheck:
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)
	
cachecheck:
	valgrind --tool=cachegrind ./$(TARGET)

profile:
	make clean all PROFILE=-pg
	gprof ./$(TARGET)
