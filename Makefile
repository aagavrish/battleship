GCC=gcc
FLAGS=-Wall -Werror -Wextra

SERVER_DIR=server
CLIENT_DIR=client
SHARED_DIR=shared

server_compile:
	$(GCC) $(FLAGS) -o LaunchServer $(SERVER_DIR)/*.c $(SHARED_DIR)/*.c -lm

client_compile:
	$(GCC) $(FLAGS) -o LaunchClient $(CLIENT_DIR)/*.c $(SHARED_DIR)/*.c

doc:
	doxygen Doxyfile

clean:
	rm -f LaunchServer
	rm -f LaunchClient

clean_doc:
	rm -rf docs
