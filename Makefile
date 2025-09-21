sources := $(shell find src/ -iname "*.c")
headers := $(shell find src/ -iname "*.h")
build_path := build
executable := hypr-ws-listener
deps := json-c

build/hyprland-ws-listener: $(sources) $(headers)
	gcc -o $(build_path)/$(executable) $(sources) `pkg-config --cflags --libs $(deps)` -g

init:
	mkdir $(build_path)

run:
	gcc -o $(build_path)/$(executable) $(sources) `pkg-config --cflags --libs $(deps)` -g
	build/hyprland-ws-listener

clean:
	rm -rf $(build_path)/*
