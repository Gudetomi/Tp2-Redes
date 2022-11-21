	PORT=4004

default: help

help:
	@echo "Server"
	@echo
	@echo "Target rules:"
	@echo "    run_server_tcp      	- Compiles and run tcp server"
	@echo "    run_server_tcp_thread       - Compiles and run tcp server with thread"
	@echo "    run_server_tcp_mt      	- Compiles and run tcp server with multiple threads"
	@echo "    buid_tcp     		- Compiles tcp server"
	@echo "    build_thread      		- Compiles tcp server with thread"
	@echo "    build_mt      		- Compiles tcp server with multiple threads"
	@echo "    clean    			- Clean the project by removing binaries"
	@echo "    help     			- Prints a help message with target rules"

run_server_tcp: build_tcp
	./bin/server_tcp $(PORT)

run_server_tcp_thread: build_thread
	./bin/server_tcp_thread $(PORT)

run_server_tcp_mt: build_mt
	./bin/server_tcp_mt $(PORT)

run_server_select: build_select
	./bin/server_tcp_select $(PORT)

build_mt: multi_thread.o
	gcc -o bin/server_tcp_mt bin/multi_thread.o bin/fila.o bin/file.o bin/http.o bin/requests.o -lpthread

build_tcp: tcp.o
	gcc -o bin/server_tcp bin/tcp.o bin/file.o bin/http.o bin/requests.o

build_thread: tcp_thread.o
	gcc -o bin/server_tcp_thread bin/tcp_thread.o bin/file.o bin/http.o bin/requests.o -lpthread

build_select: tcp_select.o
	gcc -o bin/server_tcp_select bin/tcp_select.o bin/file.o bin/http.o bin/requests.o -lpthread


tcp_thread.o: src/server_tcp_thread.c http.o
	gcc -o bin/tcp_thread.o src/server_tcp_thread.c -c -Wall

tcp.o: src/server_tcp.c http.o
	gcc -o bin/tcp.o src/server_tcp.c -c -Wall

multi_thread.o: src/server_multi_thread.c fila.o http.o
	gcc -o bin/multi_thread.o src/server_multi_thread.c -c -Wall

tcp_select.o: src/server_tcp_select.c http.o
	gcc -o bin/tcp_select.o src/server_tcp_select.c -c -Wall

fila.o: src/fila.c src/fila.h
	gcc -o bin/fila.o src/fila.c -c -Wall

http.o: src/http.c src/http.h file.o requests.o
	gcc -o bin/http.o src/http.c -c -Wall

file.o: src/file.c src/file.h
	gcc -o bin/file.o src/file.c -c -Wall

requests.o: src/requests.c src/requests.h
	gcc -o bin/requests.o src/requests.c -c -Wall

clean:
	@rm ./bin/*