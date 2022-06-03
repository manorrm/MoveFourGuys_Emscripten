#include <stdio.h>
#include "SDL_Net_BS.h"
#include <emscripten.h>

#define NR_OF_CLIENTS 4

void mainf(){
  
  IPaddress ip;
  SDLNet_BS_ResolveHost(&ip, NULL, 8080);
  Socket server;
  server = SDLNet_BS_TCP_Open(&ip);
  
  Socket clients[NR_OF_CLIENTS];

  char data[250];  
  char str[50];

  int running = 1;

  while(running){
    emscripten_sleep(1);
    
    for (int i = 0; i < NR_OF_CLIENTS; i++){
      if (clients[i] == NULL){
        clients[i] = SDLNet_BS_TCP_Accept();

      } else {
        //check if we have a message, 0 = no message
        int result = SDLNet_BS_TCP_Recv(clients[i], data, sizeof(data));

        if (result > 0){
          //Send data and player number to all but the original sender.
          sprintf(str, " %d", i); 
          strcat(data, str);
          for (int j = 0; j < NR_OF_CLIENTS; j++){
            if (j != i){
              if(clients[j] != NULL){
                if (SDLNet_BS_TCP_Send(clients[j], data) < 0){
                  //If send returns less than zero the client has disconnected  
                  SDLNet_BS_TCP_Close(clients[i]);
                  clients[i] = NULL;
                }
              }
            }
          }
        } else {
          //if result was less than 0 client has disconnected
          if (result < 0){
            SDLNet_BS_TCP_Close(clients[i]);
            clients[i] = NULL;
          }
        }
      }      
    } 
  }
}
