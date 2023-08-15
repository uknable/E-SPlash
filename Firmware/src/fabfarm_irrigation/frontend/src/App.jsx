import { Routes, Route } from 'react-router-dom';

import './App.css';

import Topbar from './components/Topbar';
import Home from './pages/Home';
import Settings from './pages/Settings';
import Update from './pages/Update';

let socket;

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    socket = new WebSocket(`ws://${window.location.hostname}/ws`);
    socket.onopen = onOpen;
    socket.onclose = onClose;
    socket.onmessage = onMessage;
}
function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
    console.log('Message received');
    console.log(event);

    sendMessage("hello");
}

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
}

// Make the function wait until the connection is made...
function waitForSocketConnection(ws, callback){
    setTimeout(
        function () {
            if (ws.readyState === 1) {
                console.log("Connection is made")
                if (callback != null){
                    callback();
                }
            } else {
                console.log("wait for connection...")
                waitForSocketConnection(ws, callback);
            }

        }, 1000); // milliseconds
}

function sendMessage(msg){
    // Wait until the state of the socket is not ready and send the message when it is...
    waitForSocketConnection(socket, function(){
        console.log("message sent from sendMessage");
        socket.send(msg);
    });
}


function App() {
    return (
        <div className='app-container'>
            <Topbar />
            <Routes>
                <Route path='/' element={<Home />} />
                <Route path='/settings' element={<Settings />} />
                <Route path='/update' element={<Update />} />
            </Routes>
        </div>
    );
}

export default App;
