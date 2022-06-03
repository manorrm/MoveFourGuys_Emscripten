#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <emscripten.h>
#include <emscripten/websocket.h>
#include "SDL_Net_B.h"

/**
 * Inverse square root of two, for normalising velocity
 */
#define REC_SQRT2 0.7071067811865475 
EMSCRIPTEN_WEBSOCKET_T socket;
/**
 * Set of input states
 */
enum input_state{
    NOTHING_PRESSED = 0,
    UP_PRESSED = 1,
    DOWN_PRESSED = 1<<1,
    LEFT_PRESSED = 1<<2,
    RIGHT_PRESSED = 1<<3
};

/**
 * Context structure that will be passed to the loop handler
 */
struct context{
    SDL_Renderer *renderer;

    /**
     * Rectangle that the texture will be rendered into
     */
    SDL_Rect dest;
    SDL_Rect playerArray[4];
    SDL_Texture *char_texture;
    Socket socket;
    enum input_state active_state;

    /**
     * x and y components of player velocity
     */
    int vx;
    int vy;
};


/**
 * Loads the texture into the context
 */
int get_player_texture(struct context * ctx){
  SDL_Surface *image = IMG_Load("client/resources/char1.png");
  if (!image)
  {
     printf("IMG_Load: %s\n", IMG_GetError());
     return 0;
  }
  ctx->char_texture = SDL_CreateTextureFromSurface(ctx->renderer, image);
  ctx->dest.w = image->w;
  ctx->dest.h = image->h;

  SDL_FreeSurface (image);

  return 1;
}

/**
 * Processes the input events and sets the velocity
 * of the player accordingly
 */

void process_input(struct context *ctx){
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.key.keysym.sym)
        {
            case SDLK_UP:
                if (event.key.type == SDL_KEYDOWN){
                    ctx->active_state |= UP_PRESSED;
                }                     
                else if (event.key.type == SDL_KEYUP)
                    ctx->active_state ^= UP_PRESSED;
                break;
            case SDLK_DOWN:
                if (event.key.type == SDL_KEYDOWN)
                    ctx->active_state |= DOWN_PRESSED;
                else if (event.key.type == SDL_KEYUP)
                    ctx->active_state ^= DOWN_PRESSED;
                break;
            case SDLK_LEFT:
                if (event.key.type == SDL_KEYDOWN)
                    ctx->active_state |= LEFT_PRESSED;
                else if (event.key.type == SDL_KEYUP)
                    ctx->active_state ^= LEFT_PRESSED;
                break;
            case SDLK_RIGHT:
                if (event.key.type == SDL_KEYDOWN)
                    ctx->active_state |= RIGHT_PRESSED;
                else if (event.key.type == SDL_KEYUP)
                    ctx->active_state ^= RIGHT_PRESSED;
                break;
            default:
                break;
        }
    }

    ctx->vy = 0;
    ctx->vx = 0;
    if (ctx->active_state & UP_PRESSED)
        ctx->vy = -5;
    if (ctx->active_state & DOWN_PRESSED)
        ctx->vy = 5;
    if (ctx->active_state & LEFT_PRESSED)
        ctx->vx = -5;
    if (ctx->active_state & RIGHT_PRESSED)
        ctx->vx = 5;

    if (ctx->vx != 0 && ctx->vy != 0)
    {
        ctx->vx *= REC_SQRT2;
        ctx->vy *= REC_SQRT2;
    }
}

/**
 * Loop handler that gets called each animation frame,
 * process the input, update the position of the player and 
 * then render the texture
 */
void loop_handler(void *arg){
    struct context *ctx = arg;

    int vx = 0;
    int vy = 0;
    process_input(ctx);

    //change position of player
    ctx->dest.x += ctx->vx;
    ctx->dest.y += ctx->vy;

    //if position changed, send new pos to server
    
    if((ctx->vx != 0) || (ctx->vy != 0)){  
        char data1[50];

        sprintf(data1, "%d %d" , ctx->dest.x, ctx->dest.y);
        //printf("I SENT: %s\n", data1);
        SDLNet_B_TCP_Send(ctx->socket, data1);
    }

    
    //recive updates from server
    char data2[50];
    int respons = SDLNet_B_TCP_Recv(ctx->socket, data2, 250);
    if (respons > 0){
        //printf("MESSAGE: %s\n", data2);
        
        int tempX, tempY, tempPlayer;
        sscanf(data2, "%d %d %d", &tempX, &tempY, &tempPlayer);
        ctx->playerArray[tempPlayer].x = tempX;
        ctx->playerArray[tempPlayer].y = tempY;
    }

    if (respons < 0){
        printf("SERVER CLOSED CONNECTION!\n");
    }
    

    SDL_RenderClear(ctx->renderer);
    SDL_RenderCopy(ctx->renderer, ctx->char_texture, NULL, &ctx->dest);
    for(int i  = 0; i<4; i++){ 
        SDL_RenderCopy(ctx->renderer, ctx->char_texture, NULL, &ctx->playerArray[i]);
    }
    SDL_RenderPresent(ctx->renderer);


    
}

void mainf(){
    SDL_Window *window;
    struct context ctx;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(600, 400, 0, &window, &ctx.renderer);
    SDL_SetRenderDrawColor(ctx.renderer, 255, 255, 255, 255);

    get_player_texture(&ctx);
    ctx.active_state = NOTHING_PRESSED;
    ctx.dest.x = 200;
    ctx.dest.y = 100;
    ctx.vx = 0;
    ctx.vy = 0;

    for(int i = 0; i<4; i++){
        SDL_Rect playerRect;
        playerRect.x = -500;
        playerRect.y = -500;
        playerRect.w = ctx.dest.w;
        playerRect.h = ctx.dest.h;
        ctx.playerArray[i] = playerRect;
    }



    if (SDLNet_B_init() < 0){
        exit(1);
    }

    IPaddress address;
    SDLNet_B_ResolveHost(&address, "ws://localhost:", 8080);
    ctx.socket = SDLNet_B_TCP_Open(&address);

    /**
     * Schedule the main loop handler to get 
     * called on each animation frame
     */
    emscripten_set_main_loop_arg(loop_handler, &ctx, -1, 1);
}
