#ifndef SDL_Net_BS_h
#define SDL_Net_BS_h

#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <string.h>
/*
BS_Socket struct contains an id variable in the form of a UUID as a
string and a pointer to an IPaddress struct. It emulates the TCP_Socket 
provided by the SDL_Net library.
*/
typedef struct BS_Socket* Socket;
/*
Emulates the IPaddress struct from SDL_Net. It contains host as a 
string and port as an integer.
*/
typedef struct BS_IPaddress {
    const char* host;    /* IPv4 host address */
    int port;            /* port */
} IPaddress;

/*
Closes all connected sockets and finally closes itself to new connections.  
*/
void SDLNet_BS_Quit();

/*
Adds a given hostname and port to an IPaddress struct. 
*/
int SDLNet_BS_ResolveHost(IPaddress *address, const char* host, int port);

/*
Simply returns hostname from provided IPaddress struct.
*/
const char *SDLNet_BS_ResolveIP(IPaddress *address);

/*
Takes an IPaddress struct and uses its content to start the WebSocket server. 
Returns a pointer to a BS_Socket with the provided IPaddress and 
the id “SERVER”. 
*/
Socket SDLNet_BS_TCP_Open(IPaddress *address);

/*
Closes the socket and removes the connected client and all unread 
incoming messages.
*/
void SDLNet_BS_TCP_Close(Socket sock);

/*
Checks the list of connected clients for unhandled connections. If a new 
client has connected to the server its information is retrieved. A new 
BS_Socket with the clients address and UUID is created and returned to 
the user in the server code.
*/
Socket SDLNet_BS_TCP_Accept();

/*
Returns a pointer to the given Sockets IPaddress struct. 
*/
IPaddress *SDLNet_BS_TCP_GetPeerAddress(Socket sock);

/*
Passes UUID of provided socket and message to be sent to Websocket file. 
WebSocket is retrieved from the connection list by id and used to send 
the message in UTF-8 format.  Returns length of message. If function 
returns -1 message was not sent and socket should be closed.    
*/
int SDLNet_BS_TCP_Send(Socket sock, const char* data);

/*
Checks the message array for the first available message with the same 
id as the provided socket. Message is removed from the array and returned 
to the user by copying content of message to provided data char pointer. 
If function returns 0 no message was found and if it returns -1 no client 
was found. The latter warrants the need to close the socket.
*/
int SDLNet_BS_TCP_Recv(Socket sock, char* data, int maxlen);

#endif