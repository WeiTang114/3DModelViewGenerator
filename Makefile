CC=g++

OS := $(shell uname)
ifeq ($(OS),Darwin)
  CFLAGS=-I/usr/local/include/ `pkg-config --cflags glfw3` `pkg-config --cflags opencv`
  LDFLAGS= `pkg-config --libs glfw3` `pkg-config --libs opencv`
else
  CFLAGS=-std=c++11 -I/usr/local/include/ `pkg-config --cflags opencv` -DDEBUG

  # `pkg-config --libs opencv` will get "-lippicv not found" error
  LDFLAGS= -L/usr/local/lib -lopencv_cudabgsegm -lopencv_cudaobjdetect -lopencv_cudastereo -lopencv_shape -lopencv_stitching -lopencv_cudafeatures2d -lopencv_superres -lopencv_cudacodec -lopencv_videostab -lopencv_cudaoptflow -lopencv_cudalegacy -lopencv_calib3d -lopencv_features2d -lopencv_objdetect -lopencv_highgui -lopencv_videoio -lopencv_photo -lopencv_imgcodecs -lopencv_cudawarping -lopencv_cudaimgproc -lopencv_cudafilters -lopencv_video -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_cudaarithm -lopencv_core -lopencv_cudev -lGLEW -lGL -lconfig++
endif


TARGET=view_generator old_view_generator

all: $(TARGET)

view_generator: view_generator.cpp view_post_proc.h
	$(CC) $< -o $@ -g $(CFLAGS) $(LDFLAGS)


old_view_generator: view_generator_old.cpp
	$(CC) $< -o $@ -framework OpenGL -framework GLUT `pkg-config --cflags --libs opencv`

clean:
	rm $(TARGET)
