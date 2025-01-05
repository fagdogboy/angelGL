angelGL: main.cpp
	g++ -std=c++20 -O2 -o app main.cpp glad.c -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -lasound -lGL

.PHONY: test clean

test: angelGL
	./app

clean:
	rm -f app
