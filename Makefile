PORT=4000

default: server_m

help:
	@echo "Server"
	@echo
	@echo "Target rules:"
	@echo "    build      - Compiles and generates binary file"
	@echo "    run_server    - Starts a server"
	@echo "    run_client 	- Runs a client"
	@echo "    clean    - Clean the project by removing binaries"
	@echo "    help     - Prints a help message with target rules"

build: multi_thread.o
	gcc -o bin/server_m bin/multi_thread.o bin/fila.o bin/file.o bin/http.o bin/requests.o

multi_thread.o: src/multi_thread.c fila.o http.o
	gcc -o bin/multi_thread.o src/multi_thread.c -c -Wall

fila.o: src/fila.c src/fila.h
	gcc -o bin/fila.o src/fila.c -c -Wall

http.o: src/http.c src/http.h file.o requests.o
	gcc -o bin/http.o src/http.c -c -Wall

file.o: src/file.c src/file.h
	gcc -o bin/file.o src/file.c -c -Wall

requests.o: src/requests.c src/requests.h
	gcc -o bin/requests.o src/requests.c -c -Wall

run_server:
	./bin/server 5004

run_cliente:
	./bin/cliente localhost 5004

clean:
	@rm ./bin/*