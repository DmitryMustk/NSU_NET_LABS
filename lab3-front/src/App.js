import React, { useState } from "react";
import { BrowserRouter as Router, Route, Routes } from "react-router-dom";
import LocationSearch from './components/LocationSearch';
import LocationList from './components/LocationList';
import LocationDetails from './components/LocationDetails';

function App() {
	const [locations, setLocations] = useState([]);

	const handleSearch = (results) => {
		setLocations(results);
	};

	return (
		<Router>
			<div className="d-flex flex-column min-vh-100 bg-dark text-light">
				<div className="container my-auto">
					<h1 className="display-4 text-center mb-4">Locations Search</h1>
					<LocationSearch onSearch={handleSearch} />
					<Routes>
						<Route path="/" element={<LocationList locations={locations} />} />
						<Route path="/location/:lat/:lon" element={<LocationDetails />} />
					</Routes>
				</div>
			</div>
		</Router>
	);
}

export default App;
