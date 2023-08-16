import { useState, useEffect, useRef } from 'react';
import { useData } from '../../hooks/useData';

import Dashboard from '../../components/Dashboard';
import Controls from '../../components/Controls';

import { useWebSocket, WebSocketContextProvider } from '../../context/WebSocketContext';

const Home = () => {
    const { data, setData } = useData();

    const [startTime, setStartTime] = useState('');
    const [duration, setDuration] = useState('');

    let socket;

    function initWebSocket() {
        console.log('Trying to open a WebSocket connection...');
        socket = new WebSocket(`ws://${window.location.hostname}/ws`);
        socket.onopen = onOpen;
        socket.onclose = onClose;
        socket.onmessage = onMessage;
    }

    function onOpen(event) {
        console.log('WebSocket connection opened');
    }

    function onClose(event) {
        console.log('WebSocket connection closed');
        setTimeout(initWebSocket, 2000);
    }

    function onMessage(event) {
        console.log('WebSocket response received');
        console.log(event);

        sendMessage("hello");
    }

    // Make the function wait until the connection is made...
    function waitForSocketConnection(ws, callback) {
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

    function sendMessage(msg) {
        // Wait until the state of the socket is not ready and send the message when it is...
        waitForSocketConnection(socket, () => {
            console.log("WebSocket message sent from app: ", msg);
            socket.send(msg);
        });
    }

    // function sendMessage(msg) {
    //     socket.send(msg);
    // }

    useEffect(() => {
        initWebSocket();
    }, []);

    const handleScheduleModeChange = () => {
        console.log("Sending message");
        sendMessage("hello from handleScheduleModeChange");
    }

    // const handleScheduleModeChange = (e, relayId) => {
    //     console.log("sending request", relayId);
    //     fetch({
    //         url: `/relays/${relayId}/schedule-mode`,
    //         method: 'POST',
    //         headers: { 'Content-Type': 'application/json' },
    //         body: { isScheduleMode: e.target.checked },
    //     })
    //         .then((res) => {
    //             console.log("response received", res);
    //             res.json()
    //         })
    //         .then((data) => {
    //             console.log("setting data", data);
    //             setData(data)
    //         });
    // };

    const handleToggleRelay = (e, relayId) => {
        fetch({
            url: `/relays/${relayId}/enable`,
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: { isEnabled: e.target.checked },
        })
            .then((res) => res.json())
            .then((data) => setData(data));
    };

    const addSchedule = (relayId) => {
        fetch({
            url: `/relays/${relayId}/schedule`,
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: { startTime, duration },
        })
            .then((res) => res.json())
            // or should the server just return { "id": string, "startTime": string , "duration": number }
            // and should I just setData with that prop?
            .then((data) => setData(data));
    };

    const removeSchedule = (relayId, scheduleid) => {
        fetch({
            url: `/relays/${relayId}/schedule/${scheduleid}`,
            method: 'DELETE',
            headers: { 'Content-Type': 'application/json' },
        })
            .then((res) => res.json())
            // or should the server just return { "id": string, "startTime": string , "duration": number }
            // and should I just setData with that prop?
            .then((data) => setData(data));
    };

    const modifySchedule = () => {
        fetch({
            url: `/relays/${relayId}/schedule/${scheduleid}`,
            method: 'PATCH',
            headers: { 'Content-Type': 'application/json' },
            // is it gonna be the same startTime and duration of addSchedule? probably not. one in a modal, the other inline
            body: { startTime, duration },
        })
            .then((res) => res.json())
            // or should the server just return { "id": string, "startTime": string , "duration": number }
            // and should I just setData with that prop?
            .then((data) => setData(data));
    };

    return (
        <main>
            <WebSocketContextProvider socket={socket} >
                {!!data && (
                    <>
                        <Dashboard data={data} />
                        <Controls
                            data={data}
                            handleScheduleModeChange={handleScheduleModeChange}
                            handleToggleRelay={handleToggleRelay}
                            scheduleInputs={{
                                startTime,
                                setStartTime,
                                duration,
                                setDuration,
                            }}
                            addSchedule={addSchedule}
                            removeSchedule={removeSchedule}
                            modifySchedule={modifySchedule}
                        />
                    </>
                )}
            </WebSocketContextProvider>
        </main>
    );
};

export default Home;
