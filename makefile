OBJS = animatedwallpaper.o
CFLAGS = -Wall
INCLUDES = 
LIBS = -L/usr/include -lImlib2 -lX11
CC = gcc

animatedwallpaper:${OBJS}
	${CC} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LIBS}

clean:
	-rm -f *.o core *.core

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $< ${LIBS}
