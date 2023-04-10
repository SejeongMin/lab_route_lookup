SRC = my_route_lookup.c io.c utils.c
CFLAGS = -Wall -O3
DEBUG = -g -fsanitize=address
NAME = my_route_lookup
# TODO: remove -O3 and add -g for debugging option

all: my_route_lookup

my_route_lookup: $(SRC)
	gcc $(CFLAGS) $(SRC) -o $(NAME) -lm

.PHONY: clean

clean:
	rm -f $(NAME)

#RL Lab 2020 Switching UC3M

# TODO: use valgrind for the final submission
