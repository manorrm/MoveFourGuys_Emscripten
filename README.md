
# move4guys_Emscripten
A simple example of Emscripten and our SDL_Net_B and SDL_Net_BS library intended to emulate SDL_Net syntax to run C code in a browser with multiplayer support over WebSockets.
Consists of a client and a node echo server that can handle up to four clients.


## Dependencies
   - Python          https://www.python.org/downloads/
   - npm & Node.js   https://docs.npmjs.com/downloading-and-installing-node-js-and-npm#overview
   - Git             https://git-scm.com/downloads

## Installation:
1.	Download/clone this repository 
2.	Open a CLI such as CMD in windows and navigate to “echo_server/node” and install the Websocket library. 
```
npm i
```
3.	While in “echo_server/node”, start node server on port 8080 by running
```
node index.js
```
5.	You will need a fileserver to host the client side part of this, you can use the one included in python or install one using npm by running the command down below in a CLI.
```
npm i -g http-server
```
6.	Navigate to “echo_client/client” and start a file server, just make sure it doesn't use the 8080 port.
```
http-server -p <port>
OR
python -m http.server <port>
```
7.	Repeat step 7 three more times and go to the servers in browser (http://localhost:<port>). Press the “echo_client.html” to run the client in the browser. 
8.	Run around as up to 4 Santas in your browser using the arrow keys :).  

## Edit code:

1.	To install Emscripten we found that the simplest way is via git. If you don’t have git installed follow link above and install with default settings. 
    NB! If you use windows and haven’t used git before, select default console window when given the option. 
2.	Now install Emscripten. https://emscripten.org/docs/getting_started/downloads.html
    NB! When at:
```
./emsdk activate latest
```
   do
```
./emsdk activate --permanent
```
   to enable constant usage of emsdk. Will be very fiddly otherwise as you only activate it per terminal window. 
3.	Now let’s test the compiler. Open server folder in IDE and go to “src” and run the Makefile with emscripten make.
```
emmake make
```
4.	If that worked you should be able to edit and compile code. Make sure to have a function called “mainf” that takes no arguments and returns void in your server and client       code. If you add more files they need to be added to the Makefile as well.

    Good luck :]   


