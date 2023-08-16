import { createContext, useContext } from 'react';

const WebSocketContext = createContext();

export const useWebSocket = () => {
    return useContext(WebSocketContext);
};

export const WebSocketContextProvider = ({ children, socket }) => {
    return (
        <WebSocketContext.Provider value={socket}>
            {children}
        </WebSocketContext.Provider>
    );
};

// export { WebSocketContext, WebSocketContextProvider };
