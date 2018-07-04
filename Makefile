CC=g++

OS := $(shell uname)
ifeq ($(OS),Darwin)
  CFLAGS=-I/usr/local/include/ `pkg-config --cflags glfw3` `pkg-config --cflags opencv`
  LDFLAGS= `pkg-config --libs glfw3` `pkg-config --libs opencv`
else
  CFLAGS=-std=c++11 -I/usr/local/include/ `pkg-config --cflags opencv` -DDEBUG
  LDFLAGS= -L/usr/local/lib -lGLEW -lGL -lglfw -lconfig++ -lGLU `pkg-config --libs opencv`
endif


TARGET=view_generator # old_view_generator

all: $(TARGET)

view_generator: view_generator.cpp view_post_proc.h
	$(CC) $< -o $@ -g $(CFLAGS) $(LDFLAGS)


old_view_generator: view_generator_old.cpp
	$(CC) $< -o $@ -framework OpenGL -framework GLUT `pkg-config --cflags --libs opencv`

clean:
	rm $(TARGET)
