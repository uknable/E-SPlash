import React from 'react';
import ReactDOM from 'react-dom/client';
import { BrowserRouter as Router } from 'react-router-dom';

import App from './App.jsx';
import { DataContextProvider } from './context/DataContext';
import { WebSocketContextProvider } from './context/WebSocketContext';

// import './styles/normalize.css';
import './index.css';


ReactDOM.createRoot(document.getElementById('root')).render(
    <React.StrictMode>
        <Router>
            <WebSocketContextProvider>
                <DataContextProvider>
                    <App />
                </DataContextProvider>
            </WebSocketContextProvider>
        </Router>
    </React.StrictMode>
);
