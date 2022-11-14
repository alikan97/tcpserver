server:
	gcc -o build/serverOut server/Channels.c server/Clients.c server/message.c server/main.c -std=gnu99 -pthread -lm -lrt
client:
	gcc -o build/clientOut client/client.c -std=gnu99 -pthread -lm -lrt
clean:
	rm -f build/*
