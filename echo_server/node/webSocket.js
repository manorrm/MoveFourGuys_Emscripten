
const ws = require('ws');
const storage = require("./storageArrays.js")
const UUID = require('crypto')
var wss = null;

function startSocket(port) {
  try {
    wss = new ws.Server({ port: port });
    console.log('WebSocket server listening on ws://localhost:' + port + '/');

    wss.on('connection', (ws, req) => {
      console.log('Client connected!');
      var address = req.socket.remoteAddress
      var port = req.socket.remotePort
      
      storage.newConnection(ws, address, port, UUID.randomUUID().toString('utf8')) 

      ws.on('message', (message) => {
        try {
          if (storage.newMessage(ws, message.toString('utf8')) < 0) {
            storage.removeSocket(storage.getUUID(ws));   
            ws.close();
          }
        } catch (error) {
          console.log("ERROR at newMessage in webSocket.js, INFO: "+error);
          ws.close();
        }
        
      });

      ws.on('close', ()=>{
        try {
          storage.removeSocket(storage.getUUID(ws));
        } catch (error) {
          console.log("ERROR at onClose in webSocket.js, INFO: "+error);
        }
        
      })
    });
    return 1;

  } catch (error) {
    console.log("ERROR at startSocket in webSocket.js, INFO: "+error.message);
    return -1;
  }
}

function send(uuid, message) {
  try {
    var ws = storage.getSocket(uuid);
    ws.send(message.toString('utf8'));
    return 1;
  } catch (error) {
    console.log("ERROR at send in webSocket.js, INFO: "+error.message);
    return -1;
  }
  
}

function close(uuid) {
  try {
    var ws = storage.removeSocket(uuid);
    ws.close();
  } catch (error) {
    console.log("ERROR at close in webSocket.js, INFO: "+error);
  }
  
}

function quit() {
  var cons = storage.getConnections();
  cons.forEach(con => {
    storage.removeSocket(con.uuid);
    con.socket.close();
  });
  wss.close();
}

module.exports = {startSocket, send, close, quit}

