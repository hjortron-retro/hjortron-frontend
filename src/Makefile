OBJS=\
	main.o \
	logger.o \
	romident.o \
	vfs.o \
	scraper.o \
	core.o \
	engine.o \
	scene.o \
	draw.o \
	config.o \
	main_scene.o \
	in_game_menu_scene.o \
	run_game_scene.o

LIBS=-ldl
CFLAGS=-g -Wall -I.\
	$(shell pkg-config -cflags alsa)\
	$(shell pkg-config -cflags sdl2)\
	$(shell pkg-config -cflags jansson)\
	$(shell pkg-config -cflags sqlite3)

LDFLAGS= $(LIBS)\
	$(shell pkg-config -libs alsa)\
	$(shell pkg-config -libs sdl2)\
	$(shell pkg-config -libs SDL2_ttf)\
	$(shell pkg-config -libs jansson)\
	$(shell pkg-config -libs sqlite3)

all: hjortron-frontend romident

hjortron-frontend: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)

romident: romident_tool.o romident.o
	$(CC) -o $@  $^


%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJS)
	rm hjortron-frontend
