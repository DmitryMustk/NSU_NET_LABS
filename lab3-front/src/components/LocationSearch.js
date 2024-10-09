import React, { useState } from "react";
import axios from "axios";
import { useNavigate } from "react-router-dom";

function LocationSearch({ onSearch }) {
	const [query, setQuery] = useState('');
	const navigate = useNavigate();

	const handleSearch = async () => {
		if (query) {
			try {
				const response = await axios.get(`http://localhost:8080/api/locations?location=${query}`);
				const locations = response.data.hits || [];
				onSearch(locations);

				// После успешного поиска перенаправляем на главную страницу
				navigate('/');
			} catch (error) {
				console.error("Error fetching locations:", error);
			}
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




