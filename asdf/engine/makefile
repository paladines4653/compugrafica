CC     = gcc
CFLAGS = -Wall -c
LFLAGS = -Wall -o
CLIBS  = `sdl-config --cflags` -IGL -IGLU -Iopenal -Ivorbisfile -ISDL_image -ISDL_ttf -Iassimp
LLIBS  = `sdl-config --libs` -lGL -lGLU -lopenal -lvorbisfile -lSDL_image -lSDL_ttf -lassimp
NAME   = ejercicio

$(NAME): $(NAME).o
	$(CC) $(LFLAGS) $(NAME) $(NAME).o $(LLIBS)

$(NAME).o: $(NAME).c
	$(CC) $(CFLAGS) $(NAME).c $(CLIBS)

clean:
	rm -f *.c~ *.o $(NAME)
