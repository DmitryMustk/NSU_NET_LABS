import React from 'react';
import ReactDOM from 'react-dom/client';
import App from './App';

import 'bootstrap/dist/css/bootstrap.min.css';


const root = ReactDOM.createRoot(document.getElementById('root'));
root.render(
    <div className={"mb-3 bg-dark p-3 text-light"}>
  <React.StrictMode>
    <App />
  </React.StrictMode>
    </div>
);

