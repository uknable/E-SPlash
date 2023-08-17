import { useState, useEffect } from 'react';
import './Dashboard.css';
import { useData } from '../../hooks/useData';

const Dashboard = ({ data }) => {
    const [currentDate, currentTime] = data.global.time.split('T');

    const { setData } = useData();

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

    return (
        <section className='dashboard'>
            <header className='page-title'>
                <h2>Dashboard</h2>
            </header>

            <div className='cards-container'>
                <div className='card'>
                    <div>{currentDate}</div>
                    <div className='card-value'>{currentTime}</div>
                </div>

                <div className='card'>
                    <div className='icon-container'>
                        <img src='/src/assets/icons/thermometer.svg' alt='thermometer' />
                    </div>
                    <div className='card-value'>{data.global.temperature ?? 'n/a'}</div>
                </div>

                <div className='card'>
                    <div className='icon-container'>
                        <img src='/src/assets/icons/thermometer.svg' alt='humidity' />
                    </div>
                    <div className='card-value'>{data.global.humidity ?? 'n/a'}</div>
                </div>

                <div className='card'>
                    <div className='icon-container'>
                        <img src='/src/assets/icons/thermometer.svg' alt='batLevel' />
                    </div>
                    <div className='card-value'>{data.global.batLevel ?? 'n/a'}</div>
                </div>

                <button onClick={fetchAndSetData}>
                    Refresh
                </button>
            </div>
        </section>
    );
};

export default Dashboard;
