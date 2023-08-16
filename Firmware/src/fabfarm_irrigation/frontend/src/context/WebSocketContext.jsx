import { createContext, useState, useEffect } from 'react';
// import { fetchData } from '../services';

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
                setTimeout(fetchAndSetData, DATA_FETCH_INTERVAL);
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
