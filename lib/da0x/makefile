# Generated with github.com/da0x/mmpp
# Binary:
uuid: .obj .obj/main.cpp.o
	g++ -std=c++20 .obj/main.cpp.o -o uuid

.obj:
	mkdir .obj

.obj/main.cpp.o: main.cpp
	g++ -std=c++20 -o .obj/main.cpp.o -c main.cpp

run: uuid
	./uuid

clean:
	rm -rfv .obj

install: uuid
	sudo cp -v uuid /usr/local/bin/

uninstall:
	sudo rm -v /usr/local/bin/uuid
