OBJECTS = lockServerMessage.c cJSON.c hashmap.c

all: $(OBJECTS)
	gcc -Wall $(OBJECTS) server.c -o server
	gcc -Wall $(OBJECTS) client.c -o client

clean:
	rm -f server client
