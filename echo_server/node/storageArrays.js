var checkingIndex = 0;
var connections = new Array();
var messages = new Array();

function newConnection(socket, address, port, uuid) {
    var connection = {
        socket: socket,
        address: address,
        port: port,
        uuid: uuid,
        handled: false,
        TTL: Date.now()
    }
    console.log(address, port, uuid)
    connections.push(connection)    
}

function newMessage(socket, newMessage) {    
    for (let index = 0; index < checkingIndex; index++) {
        if (socket === connections[index].socket) {
            var message = {
                uuid: connections[index].uuid,
                data: newMessage
            }   
            
            messages.push(message);
            return 1;
        }        
    }

    var result = connections.find(obj => {
        return obj.socket === socket;
    })
    
    if (typeof result === 'undefined') {
        throw "NO CONNECTION FOUND";
    }

    if (Date.now() - result.TTL > 3000) {
        return -1;
    }
    
}

function getClientsOldestMessage(uuid) {
    var client = connections.find(obj => {
        return obj.uuid === uuid;
    })

    if (typeof client === 'undefined') {
        return "fatalError";
    }
    
    var result = messages.find(obj => {
        return obj.uuid === uuid;
    })

    if (typeof result === 'undefined') {
        return "noMessage";
    }

    const index = messages.indexOf(result);
    var message = messages[index]; // retrive the item
    
    if (index > -1) {
        messages.splice(index, 1); // remove the item
    }    

    
    return message.data;
}

function getSocket(uuid){
    var result = connections.find(obj => {
        return obj.uuid === uuid;
    })

    if (typeof result === 'undefined') {
        return result;
    }

    return result.socket;
}

function getConnections() {
    return connections;
}

function getConnection() {
    return connections[checkingIndex];
}

function incrementCheckingIndex() {
    connections[checkingIndex].handled = true;
    checkingIndex++;
    console.log("INCREMENT to "+checkingIndex)
}

function getCheckingIndex() {
    return checkingIndex;
}

function getConnectionArrayLength() {
    return connections.length;
}

function removeSocket(uuid) {
    var client = connections.find(obj => {
        return obj.uuid === uuid;
    })

    if (typeof client === 'undefined') {
        throw "NO CLIENT FOUND";
    }
    console.log("REMOVING: "+client.uuid);
    const index = connections.indexOf(client);
    var connection = connections[index]; // retrive the item
    if (index > -1) {
        connections.splice(index, 1); // remove the item
        var handled = 0;
        connections.forEach(obj => {
            if (obj.handled) {
                handled++;
            }
        });
        checkingIndex = handled;
    }   
    
    var filtered = messages.filter(obj => {
        return obj.uuid !== uuid;
    })

    messages = filtered;
    return connection.socket;
}

function getUUID(socket) {
    var result = connections.find(obj => {
        return obj.socket == socket
    })

    if (typeof result === 'undefined') {
        return result;
    }
    
    return result.uuid;
}

module.exports = {
    getConnection, 
    newConnection, 
    getSocket,
    incrementCheckingIndex,
    getCheckingIndex,
    getConnectionArrayLength,
    newMessage,
    getClientsOldestMessage,
    removeSocket,
    getUUID,
    getConnections
    }