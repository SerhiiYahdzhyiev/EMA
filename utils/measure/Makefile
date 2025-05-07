CC = gcc

CFLAGS = -I${EMA_INSTALL_DIR}/include -L${EMA_INSTALL_DIR}/lib
LFlAGS = -lEMA

all: measure

measure: measure.c
	$(CC) -g $^ $(CFLAGS) -o $@ $(LFLAGS)
