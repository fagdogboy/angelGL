VulkanTest: main.cpp
	g++ -std=c++20 -O2 -o VulkanTest main.cpp -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

.PHONY: test clean

test: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest
