TARGETS = gol

CC_C = $(CROSS_TOOL)gcc
CFLAGS = -O3 -fopenmp -Wall -g -std=c99 -Werror -pthread 

all: clean $(TARGETS)

$(TARGETS):
	$(CC_C) $(CFLAGS) $@.c -o $@ `pkg-config --cflags --libs gtk+-3.0`

clean:
	rm -f $(TARGETS)
