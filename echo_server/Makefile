CC = emcc
all: server.c
	$(CC) SDL_Net_BS.c server.c -sEXPORTED_FUNCTIONS='["_mainf"]' -sEXPORTED_RUNTIME_METHODS='["ccall","lengthBytesUTF8","stringToUTF8","UTF32ToString"]' -sASYNCIFY  -s MODULARIZE -O3 -sALLOW_MEMORY_GROWTH -o node/server.js

