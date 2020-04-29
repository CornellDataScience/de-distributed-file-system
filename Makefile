OBJECTS = lockServerMessage.c cJSON.c hashmap.c 
LINKERS = -pthread
all: $(OBJECTS)
	gcc -Wall -g $(OBJECTS) server.c -o server $(LINKERS)
	gcc -Wall $(OBJECTS) client.c -o client $(LINKERS)

clean:
	rm -f server client
