#include <stdio.h>
#include <stdlib.h>
#include "SDL_Net_B.h"
#include <emscripten.h>
#include <emscripten/websocket.h>
#include <string.h>
	
struct B_Socket {
    int socketHandle; //Emscripten socket handle
    IPaddress* address; 
};

/*
Contains an incoming message and a pointer to the next node in the linked list.
*/
typedef struct node *Node;
struct node{
    char* message;
    Node next;
};

/*
A struct containing a pointer to the B_Socket of the server and two Node
pointers. One for the first and last Node making up a linked list for 
storing incoming messages.   
*/
typedef struct socket_message_index{
    Socket socket;
    Node first;
    Node last;
} *Index;

/*
Consists of an array of Index pointers and an integer keeping track of the 
number of elements in the array. If a client is connected to more than one 
server, each server's messages will be reachable via its Index pointer in 
the array.
*/
typedef struct message_storage{
    Index indexies [10];
    int nrOfElements;
} MessageStorage;

MessageStorage* storage;

EM_BOOL WebSocketOpen(int eventType, const EmscriptenWebSocketOpenEvent *e, void *userData){
	printf("open(eventType=%d, userData=%ld)\n", eventType, (long)userData);	
	return 0;
}

EM_BOOL WebSocketClose(int eventType, const EmscriptenWebSocketCloseEvent *e, void *userData){
	printf("close(eventType=%d, wasClean=%d, code=%d, reason=%s, userData=%ld)\n", eventType, e->wasClean, e->code, e->reason, (long)userData);
	
    for (int i = 0; i < storage->nrOfElements; i++){
        int socketHandle = storage->indexies[i]->socket->socketHandle;
        if (e->socket == socketHandle){
            storage->indexies[i]->socket->socketHandle = -1;
        }
    }
    
    return 0;
}

EM_BOOL WebSocketError(int eventType, const EmscriptenWebSocketErrorEvent *e, void *userData){
	printf("error(eventType=%d, userData=%ld)\n", eventType, (long)userData);
	return 0;
}


int SDLNet_B_init(){
    if (!emscripten_websocket_is_supported())
	{
		return 0;
	}
    storage = malloc(sizeof(struct message_storage));  
    storage->nrOfElements = 0;
    if (storage == NULL){
        return 0;
    }
    
    return 1;    
}

void SDLNet_B_Quit(){
    emscripten_websocket_deinitialize();
}

int SDLNet_B_ResolveHost(IPaddress *address, const char* host, int port){
    address->host = host;
    address->port = port;
    return 1;
}

const char *SDLNet_B_ResolveIP(IPaddress *address){
    return address->host;
}

int countDigits(int n)  {  
    int counter=0; 
    while(n!=0)  
    {  
        n=n/10;  
        counter++;  
    }  
    return counter;  
}  

void SDLNet_B_TCP_Close(Socket sock){
    emscripten_websocket_close(sock->socketHandle, 1000, "SDLNet_B_TCP_Close");
    emscripten_websocket_delete(sock->socketHandle);
}

IPaddress *SDLNet_B_TCP_GetPeerAddress(Socket sock){
    return sock->address;
}


int SDLNet_B_TCP_Send(Socket sock, const char* data){
    int result = emscripten_websocket_send_utf8_text(sock->socketHandle, data);
    
    if (result < 0){
        return -1;
    } 
    
    return strlen(data);
}


EM_BOOL WebSocketMessage(int eventType, const EmscriptenWebSocketMessageEvent *e, void *userData){
    if (e->isText){
        for (int i = 0; i < storage->nrOfElements; i++){
            Index index = storage->indexies[i];
           
            if (index->socket->socketHandle == e->socket){                  
                Node node = malloc(sizeof(struct node));
                node->message = malloc(e->numBytes);
                
                strcpy(node->message, (char*) e->data);
                
                node->next = NULL;

                if (index->first == NULL){
                    index->first = node;
                    index->last = node;        
                } else {
                    index->last->next = node;
                    index->last = node;  
                }
            }     
        }
    }
    
    return 0;
    
}

int SDLNet_B_TCP_Recv(Socket sock, char* data, int maxlen){  
    if (sock->socketHandle < 0){
        return -1;
    }
    
    for (int i = 0; i < storage->nrOfElements; i++){
        Index index = storage->indexies[i];
        if (sock->socketHandle == index->socket->socketHandle){
            
            if (index->first == NULL){
                return 0;
            }
            
            if (strlen(index->first->message) > maxlen){
                return -1;
            }      

            Node tmp = index->first;  
            index->first = index->first->next;
            strcpy(data, tmp->message);
            free(tmp);
            return strlen(data);
        }
    }
    return -1;
}


Socket SDLNet_B_TCP_Open(IPaddress *address){
    EmscriptenWebSocketCreateAttributes attr;
    emscripten_websocket_init_create_attributes(&attr);

    char port[countDigits(address->port)+1];
    sprintf(port, "%d", address->port);
    
    int urlLength = (strlen(address->host) + strlen(port));
    char url[urlLength]; 
    strcat(url, address->host);
    strcat(url, port);

    printf("%s\n", url);

	attr.url = url;

	int socket = emscripten_websocket_new(&attr);
    
    if(socket > 0){

        emscripten_websocket_set_onopen_callback(socket, (void*)42, WebSocketOpen);
	    emscripten_websocket_set_onclose_callback(socket, (void*)43, WebSocketClose);
	    emscripten_websocket_set_onerror_callback(socket, (void*)44, WebSocketError);
	    emscripten_websocket_set_onmessage_callback(socket, (void*)45, WebSocketMessage);

        Socket s = malloc(sizeof(struct B_Socket));  
        if (s == NULL){
            return NULL;
        }
        s->address = address;
        s->socketHandle = socket;

        Index index = malloc(sizeof(struct socket_message_index));
        if (index == NULL){
            return NULL;
        }

        index->first = malloc(sizeof(struct node));
        index->last = malloc(sizeof(struct node));

        index->socket = s;
        index->first = NULL;
        index->last = NULL;
        
        storage->indexies[storage->nrOfElements] = index;
        storage->nrOfElements++;

        return s;  
    }
    
    return NULL;
}