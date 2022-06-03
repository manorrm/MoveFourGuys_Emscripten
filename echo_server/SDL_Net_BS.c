#include <stdio.h>
#include <stdlib.h>
#include "SDL_Net_BS.h"
#include <emscripten.h>
#include <string.h>
	

struct BS_Socket {
    const char* id;
    IPaddress* address;
};

EM_JS(void,closeAll,(),{
    const webSocket = require("./webSocket");
    webSocket.quit();
});

void SDLNet_BS_Quit(){
    closeAll();
}

int SDLNet_BS_ResolveHost(IPaddress *address, const char* host, int port){
    address->host = host;
    address->port = port;
    return 0;
}

const char *SDLNet_BS_ResolveIP(IPaddress *address){
    return address->host;
}

EM_JS(int, startSocket, (int port),{
    const webSocket = require('./webSocket.js');
    return webSocket.startSocket(port);
});

Socket SDLNet_BS_TCP_Open(IPaddress *address){
    if(startSocket(address->port) > 0){
        Socket s = malloc(sizeof(struct BS_Socket));  
        s->address = address;
        s->id = "SERVER";
        return s;  
    }
        return NULL;
}

EM_JS(void ,closeSocket,(const char* uuid),{
    const webSocket = require("./webSocket.js");
    
    webSocket.close(UTF8ToString(uuid));
});

void SDLNet_BS_TCP_Close(Socket sock){
    closeSocket(sock->id);
    free(sock->address);
    free(sock);
}

EM_JS(int, checkForNewConnection,(),{
    const storage = require("./storageArrays.js");
    
    if (storage.getConnectionArrayLength() > storage.getCheckingIndex()){
        return 1;
    }
    
    return -1;
})

EM_JS(char*, getClientID, (),{
    const storage = require("./storageArrays.js");
    
    const uuid = storage.getConnection().uuid; 
    if (typeof uuid === 'undefined'){
        const undefined = 'undefined';
        const lengthBytes = (lengthBytesUTF8(undefined)+1);
        const pointer = _malloc(lengthBytes);
        stringToUTF8(undefined, pointer, lengthBytes);

        return pointer;
    }

    const lengthBytes = (lengthBytesUTF8(uuid)+1);
    const pointer = _malloc(lengthBytes);
    stringToUTF8(uuid, pointer, lengthBytes);

    return pointer;
});

EM_JS(char*, getClientAddress, (),{
    const storage = require("./storageArrays.js");
    
    const address = storage.getConnection().address;
    if (typeof address === 'undefined'){
        const undefined = 'undefined';
        const lengthBytes = (lengthBytesUTF8(undefined)+1);
        const pointer = _malloc(lengthBytes);
        stringToUTF8(undefined, pointer, lengthBytes);

        return pointer;
    }
    
    const lengthBytes = (lengthBytesUTF8(address)+1);
    const pointer = _malloc(lengthBytes);
    stringToUTF8(address, pointer, lengthBytes);

    return pointer;
    
});

EM_JS(int, getClientPort, (), {
    const storage = require("./storageArrays.js");
    
    var port = storage.getConnection().port;
    if (typeof port === 'undefined') {
        return -1;
    }
    return port;        

});

EM_JS(void, incrementCheckingIndex ,() ,{
    const storage = require("./storageArrays.js");
    storage.incrementCheckingIndex();
})

Socket SDLNet_BS_TCP_Accept(){
    if (checkForNewConnection() < 0){
        return NULL;
    }

    const char* undefined = "undefined";

    char* uuid = getClientID();
    if (strcmp(uuid, undefined) == 0){
        free(uuid);
        printf("FAIL!\n");
        return NULL;
    }

    char* address = getClientAddress();
    if (strcmp(address, undefined) == 0){
        free(address);
        return NULL;
    }

    int port = getClientPort();
    if (port < 0){
        return NULL;
    }

    incrementCheckingIndex();

    IPaddress *ip;
    ip = malloc(sizeof(IPaddress));
    ip->host = address;
    ip->port = port;
    
    Socket s = malloc(sizeof(struct BS_Socket));  
    s->address = ip;
    s->id = uuid;
    
    return s;   
}

IPaddress *SDLNet_BS_TCP_GetPeerAddress(Socket sock){
    return sock->address;
}

EM_JS(int, sendJS, (const char* uuid, const char* data), {
    const webSocket = require("./webSocket.js");
    
    return webSocket.send(UTF8ToString(uuid), UTF8ToString(data));
});

int SDLNet_BS_TCP_Send(Socket sock, const char* data){
    int result = sendJS(sock->id, data);
    if (result < 0){
        return -1;
    }
    
    return strlen(data);
}

EM_JS(char*, recive, (const char* uuid, int maxlen),{
    const storage = require("./storageArrays.js");

    const message = storage.getClientsOldestMessage(UTF8ToString(uuid));
    
    if (message === 'noMessage' || 'fatalError'){
        const lengthBytes = (lengthBytesUTF8(message)+1);
        const pointer = _malloc(lengthBytes);
        stringToUTF8(message, pointer, lengthBytes);

        return pointer;
    }
    
    const lengthBytes = (lengthBytesUTF8(message)+1);
    if (lengthBytes > maxlen){
        const undefined = 'fatalError';
        const lengthBytes = (lengthBytesUTF8(undefined)+1);
        const pointer = _malloc(lengthBytes);
        stringToUTF8(undefined, pointer, lengthBytes);

        return pointer;
    }
    
    const pointer = _malloc(lengthBytes);
    stringToUTF8(message, pointer, lengthBytes);

    return pointer;
});

int SDLNet_BS_TCP_Recv(Socket sock, char* data, int maxlen){  
    const char* noMessage = "noMessage";
    const char* fatalError = "fatalError";
    char* message = recive(sock->id, maxlen);
    
    if (strcmp(message, fatalError) == 0){
        free(message);
        return -1;
    }

    if (strcmp(message, noMessage) == 0){
        free(message);
        return 0;
    }
    strcpy(data, message);
    return strlen(message);
}