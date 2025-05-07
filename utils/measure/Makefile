CC = gcc

CFLAGS = -I${EMA_INSTALL_DIR}/include -L${EMA_INSTALL_DIR}/lib
LFLAGS = -lEMA

all: ema_measure

ema_measure: measure.c
	$(CC) -Wall -g $^ $(CFLAGS) -o $@ $(LFLAGS)

clean:
	rm timestamps.EMA*
	rm output.EMA*
	rm ema_measure
