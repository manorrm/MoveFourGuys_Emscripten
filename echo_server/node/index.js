
const wasm = require("./server.js");

wasm().then((instance) => {
    console.log("Starting server")  
    instance.ccall('mainf', null);
  })
  