sources := $(shell find src/ -iname "*.c")
headers := $(shell find src/ -iname "*.h")

build/hyprland-ws-listener: $(sources) $(headers)
	gcc -o build/hyprland-ws-listener $(sources)

init:
	mkdir build

run:
	gcc -o build/hyprland-ws-listener $(sources)
	build/hyprland-ws-listener

clean:
	rm -rf build/*
