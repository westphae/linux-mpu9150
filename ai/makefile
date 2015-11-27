# to install OpenGL and deps on RPi2 do
# apt-get install freeglut3-dev

SHELL = /bin/sh
CFLAGS+=-I../
ifeq ($(shell uname -s),Darwin)
    CC = g++
    LDFLAGS = -framework GLUT -framework OpenGL -framework Foundation
else
    CC = g++
    LDFLAGS = -lglut -lGL -lGLU -lm
endif

project0: main.cpp
	$(CC) $(CFLAGS) -o main main.cpp ogcCircleEvaluator.cpp -L../ -limu $(LDFLAGS)
clean:
	rm -rf main *.o

