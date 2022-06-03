#ifndef SDL_Net_B_h
#define SDL_Net_B_h

#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <string.h>

/*
B_Socket struct is not to be confused with the socket handles used by 
Emscripten WebSocket library. Socket is a pointer to a struct containing 
a socket handle and a pointer to an IPaddress struct. It Emulates the 
TCP_Socket provided by the SDL_Net library.
*/
typedef struct B_Socket* Socket;
/*Emulates the IPaddress struct from SDL_Net. It contains host as a 
string and port as an integer.
*/
typedef struct B_IPaddress {
    const char* host;    /* IPv4 host address */
    int port;            /* port */
} IPaddress;

/*
Abstracts ‘emscripten_websocket_is_supported’ that checks 
if websockets are supported in the browser. Returns 1 if supported 
and 0 if not. Init also allocates memory for the message storage, 
more details under TCP_Recv.
*/
int SDLNet_B_init();

/*
Abstracts the ‘emscripten_websocket_deinitialize’ function and shuts 
down all existing websockets. 
*/
void SDLNet_B_Quit();

/*
Adds a given hostname and port to an IPaddress struct. 
*/
int SDLNet_B_ResolveHost(IPaddress *address, const char* host, int port);

/*
Simply returns hostname from provided IPaddress struct.
*/
const char *SDLNet_B_ResolveIP(IPaddress *address);

/*
Takes an IPaddress struct and uses its content to create a new 
Websocket with the Emscripten Websocket library. Callback methods 
used by the Websocket are set. A B_Socket struct is created, 
containing the newly created sockets handle and a pointer to 
the IPaddress struct. A new socket_message_index struct for the 
server connection is created. Returns a B_Socket struct of new socket.
*/
Socket SDLNet_B_TCP_Open(IPaddress *address);

/*
Uses ‘emscripten_websocket_close’ and ‘emscripten_websocket_delete’ 
to close a socket.
*/
void SDLNet_B_TCP_Close(Socket sock);

/*
Returns a pointer to the given Sockets IPaddress struct. 
*/
IPaddress *SDLNet_B_TCP_GetPeerAddress(Socket sock);

/*
Sends data in UTF-8 text format and returns length of 
sent data or -1 at failure. 
*/
int SDLNet_B_TCP_Send(Socket sock, const char* data);

/*
Accesses MessageStorage and gets the message from the first 
node in the linked list belonging to the socket provided. 
Filles given char array with retrieved data from node if length 
of data is less than maxlen. Returns -1 if connection is closed.
*/
int SDLNet_B_TCP_Recv(Socket sock, char* data, int maxlen);

#endif