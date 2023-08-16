import { createContext, useState, useEffect, useRef } from 'react';

const WebSocketContext = createContext(null);


const WebSocketContextProvider = ({ children }) => {
    let socket = useRef(null)

    function initWebSocket() {
        console.log('Trying to open a WebSocket connection...');
        socket = new WebSocket(`ws://${window.location.hostname}/ws`);
        socket.onopen = onOpen;
        socket.onclose = onClose;
        socket.onmessage = onMessage;
    }

    const onOpen = (event) => {
        console.log('WebSocket connection opened');
    }
    
    const onClose = (event) => {
        console.log('WebSocket connection closed');
        setTimeout(initWebSocket, 2000);
    }

    const onMessage = (event) => {
        console.log('WebSocket response received');
        console.log(event);
    
        sendMessage("hello");
    }
    
    // Make the function wait until the connection is made...
    const waitForSocketConnection = (ws, callback) => {
        setTimeout(() => {
            if (ws.readyState === 1) {
                console.log("WebSocket connection is open to send message.")
                if (callback != null){
                    callback();
                }
            } else {
                console.log("Waiting for connection...")
                waitForSocketConnection(ws, callback);
            }

        }, 1000); // milliseconds
    }
    
    const sendMessage = (msg) => {
        // Wait until the state of the socket is not ready and send the message when it is...
        waitForSocketConnection(socket, function(){
            console.log("WebSocket message sent from app: ", msg);
            socket.send(msg);
        });
    }

    useEffect(() => {
        initWebSocket();
    }, []);

    return <WebSocketContext.Provider value={socket}>{children}</WebSocketContext.Provider>;
};

export { WebSocketContext, WebSocketContextProvider };
