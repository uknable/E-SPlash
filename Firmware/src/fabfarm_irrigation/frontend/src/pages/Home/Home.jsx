import { useState, useEffect, useRef } from 'react';
import { useData } from '../../hooks/useData';

import Dashboard from '../../components/Dashboard';
import Controls from '../../components/Controls';



// Websocket code from:
// https://stackoverflow.com/questions/69249467/websocket-is-undefined-immediately-after-receiving-message-in-react

const socket = new WebSocket(`ws://${window.location.hostname}/ws`);

const Home = () => {
    const { data, setData } = useData();
    const [startTime, setStartTime] = useState('');
    const [duration, setDuration] = useState('');
    
    const ws = useRef(socket);

    useEffect(() => {
        ws.current?.addEventListener("message", (event) => {
            const responseData = JSON.parse(event.data);
            console.log("WebSocket JSON response received from ESP32:", responseData);
        });
        return () => {
            ws.current?.removeAllListeners();
        }
    }, [])

    const sendMessage = (msg) => ws.current?.send(msg);

    const handleScheduleModeChange = (pin) => {

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

    const handleEnableRelay = (pin) => {
        // fetch({
        //     url: `/relays/${relayId}/enable`,
        //     method: 'POST',
        //     headers: { 'Content-Type': 'application/json' },
        //     body: { isEnabled: e.target.checked },
        // })
        //     .then((res) => res.json())
        //     .then((data) => setData(data));

        console.log(`Sending message ${pin}`);

        const dataToSend = {
            action: "enable",
            relayPin: `${pin}`,
            // relayName: `${name}`
          };

        sendMessage(JSON.stringify(dataToSend));
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
            {!!data && (
                <>
                    <Dashboard data={data} />
                    <Controls
                        data={data}
                        handleScheduleModeChange={handleScheduleModeChange}
                        handleEnableRelay={handleEnableRelay}
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
        </main>
    );
};

export default Home;
