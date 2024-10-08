import React, {useState} from "react";
import LocationSearch from './components/LocationSearch';
import LocationList from './components/LocationList';

function App() {
	const [locations, setLocations] = useState([]);
	const [selectedLocation, setSelectedLocation] = useState(null);

	const handleSearch = (results) => {
		setLocations(results);
		setSelectedLocation(null);
	};

	const handleSelect = (location) => {
		setSelectedLocation(location);
	};

	return (
		<div className="container">
			<h1 className="display-4 text-center my-4">Locations Search</h1>
			<LocationSearch onSearch={handleSearch} />
			<LocationList locations={locations} onSelect={handleSelect} />
		</div>
	);
}

export default App;
