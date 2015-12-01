CC=g++

CFLAGS=-framework OpenGL -I/usr/local/include/ `pkg-config --cflags glfw3` `pkg-config --cflags opencv`
LDFLAGS=-framework OpenGL `pkg-config --libs glfw3` `pkg-config --libs opencv`

TARGET=view_generator old_view_generator

all: $(TARGET)

view_generator: view_generator.cpp view_post_proc.h
	$(CC) $< -o $@ -g $(CLFAGS) $(LDFLAGS)


old_view_generator: view_generator_old.cpp
	$(CC) $< -o $@ -framework OpenGL -framework GLUT `pkg-config --cflags --libs opencv`

clean:
	rm $(TARGET)
