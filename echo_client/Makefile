CC = emcc
all: echo_client.c
	$(CC) SDL_Net_B.c echo_client.c -O3  -s ASYNCIFY -lwebsocket.js -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' -s USE_SDL_TTF=2 -s EXPORTED_FUNCTIONS='["_mainf"]' -s EXPORTED_RUNTIME_METHODS=ccall --preload-file client/resources -sALLOW_MEMORY_GROWTH  -o client/client.js

