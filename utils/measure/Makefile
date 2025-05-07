CC = gcc

CFLAGS = -I${EMA_INSTALL_DIR}/include -L${EMA_INSTALL_DIR}/lib
LDFLAGS = -lEMA

all: ema_measure

ema_measure: measure.c
	$(CC) -Wall -O2 $^ $(CFLAGS) -o $@ $(LDFLAGS)

clean:
	rm -f ema_measure
