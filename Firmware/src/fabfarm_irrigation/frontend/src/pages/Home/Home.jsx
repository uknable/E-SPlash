import { useState, useEffect, useRef, useContext } from 'react';
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

    // Could probably refactor the following code since it's from DataContext.jsx
    const url = '/data.json';

    const fetchAndSetData = () => {
        fetch(url)
            .then((res) => res.json())
            .then((data) => {
                console.log('fetched data from server:', data);
                setData(data);
                // setTimeout(fetchAndSetData, DATA_FETCH_INTERVAL); // fetches and sets data at a regular interval
                // ^ is commented out because I wanted to fetch and set data when a websocket message is received.
            })
            .catch((err) => {
                throw new Error('Critical error fetching data from server:', err);
            });
    };

    useEffect(() => {
        ws.current?.addEventListener("message", (event) => {
            // const responseData = JSON.parse(event.data);
            // console.log("WebSocket JSON response received from ESP32:", responseData);
            fetchAndSetData();
        });
        return () => {
            ws.current?.removeAllListeners();
        }
    }, []);

    const sendMessage = (msg) => ws.current?.send(msg);

    const handleScheduleModeChange = (pin) => {
        console.log(`Toggling Schedule Mode for pin ${pin}`);

        const dataToSend = {
            action: "scheduleMode",
            relayPin: `${pin}`,
          };

        sendMessage(JSON.stringify(dataToSend));
    }

    const handleEnableRelay = (pin) => {
        console.log(`Toggling on/off status for pin ${pin}`);

        const dataToSend = {
            action: "enable",
            relayPin: `${pin}`,
          };

        sendMessage(JSON.stringify(dataToSend));
    };

    const addSchedule = (pin) => {
        console.log(`Adding a schedule for pin ${pin}`);

        const dataToSend = {
            action: "addSchedule",
            relayPin: `${pin}`,
            startTime: `${startTime}`,
            duration: `${duration}`,
        };

        console.log(dataToSend);
        sendMessage(JSON.stringify(dataToSend));
    };

    const deleteSchedule = (pin, scheduleId) => {
        console.log(`Deleting a schedule for pin ${pin} at scheduleId ${scheduleId}`);

        const dataToSend = {
            action: "deleteSchedule",
            relayPin: `${pin}`,
            scheduleId: `${scheduleId}`
        }

        sendMessage(JSON.stringify(dataToSend));

        // fetch({
        //     url: `/relays/${relayId}/schedule/${scheduleid}`,
        //     method: 'DELETE',
        //     headers: { 'Content-Type': 'application/json' },
        // })
        //     .then((res) => res.json())
        //     // or should the server just return { "id": string, "startTime": string , "duration": number }
        //     // and should I just setData with that prop?
        //     .then((data) => setData(data));
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
                        deleteSchedule={deleteSchedule}
                        modifySchedule={modifySchedule}
                    />
                </>
            )}
        </main>
    );
};

export default Home;
