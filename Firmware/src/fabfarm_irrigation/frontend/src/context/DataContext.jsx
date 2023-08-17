import { createContext, useState, useEffect } from 'react';

const DataContext = createContext(null);
const DATA_FETCH_INTERVAL = 5000; // in milliseconds

const DataContextProvider = ({ children }) => {
    const [data, setData] = useState(null);

    const url = '/data.json';
    // const url = '/src/mockData/testdata.json';
    const value = { data, setData };

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
        fetchAndSetData();
    }, []);

    return <DataContext.Provider value={value}>{children}</DataContext.Provider>;
};

export { DataContext, DataContextProvider };
