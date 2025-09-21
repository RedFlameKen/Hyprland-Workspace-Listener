sources := $(shell find src/ -iname "*.c")
headers := $(shell find src/ -iname "*.h")
deps := json-c

build/hyprland-ws-listener: $(sources) $(headers)
	gcc -o build/hyprland-ws-listener $(sources) `pkg-config --cflags --libs $(deps)` -g

init:
	mkdir build

run:
	gcc -o build/hyprland-ws-listener $(sources) `pkg-config --cflags --libs $(deps)` -g
	build/hyprland-ws-listener

clean:
	rm -rf build/*
