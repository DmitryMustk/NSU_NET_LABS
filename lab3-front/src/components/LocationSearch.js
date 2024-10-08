import React, { useState } from "react";
import axios from "axios";

function LocationSearch({ onSearch }) {
	const [query, setQuery] = useState('');

	const handleSearch = async () => {
		if (query) {
			const response = await axios.get(`http://localhost:8080/api/locations?location=${query}`);
			const locations = response.data.hits || [];
			onSearch(locations);
		}
	};

	return (
		<div className="input-group">
			<input
				type="text"
				className="form-control bg-secondary text-light"
				placeholder="Enter place name"
				aria-label="Place name"
				aria-describedby="button-addon2"
				value={query}
				onChange={(ev) => setQuery(ev.target.value)}
			/>
			<div className="input-group-append">
				<button
					className="btn btn-outline-light"
					type="button"
					id="button-addon2"
					onClick={handleSearch}
				>
					Search
				</button>
			</div>
		</div>
	);
}

export default LocationSearch;




// return (
// 	<div>
// 		<input
// 			type="text"
// 			value={query}
// 			onChange={(ev) => setQuery(ev.target.value)}
// 			placeholder="Enter the place name"
// 		/>
// 		<button onClick={handleSearch}>Search</button>
// 	</div>
// );