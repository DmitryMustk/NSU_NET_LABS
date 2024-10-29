import React, { useEffect, useState } from "react";
import axios from "axios";
import { useParams } from "react-router-dom";

function LocationDetails() {
    const { lat, lon } = useParams();
    const [locationData, setLocationData] = useState(null);

    useEffect(() => {
        const fetchLocationDetails = async () => {
            try {
                const response = await axios.get(`http://localhost:8080/api/location-details?lat=${lat}&lon=${lon}`);
                setLocationData(response.data);
            } catch (error) {
                console.error("Error fetching location details:", error);
            }
        };
        fetchLocationDetails();
    }, [lat, lon]);

    const kelvinToCelsius = (tempK) => (tempK - 273.15).toFixed(2);

    if (!locationData) return <div>Loading...</div>;

    const { weather, places } = locationData;
    const weatherDescription = weather.weather[0].description;
    const temperature = kelvinToCelsius(weather.main.temp);
    const feelsLike = kelvinToCelsius(weather.main.feels_like);

    return (
        <div className="container my-4">
            <h2 className="mb-4 text-center">Weather in {weather.name}</h2>
            <div className="card text-light bg-secondary mb-4">
                <div className="card-body">
                    <h5 className="card-title">
                        🌡️ Temperature: {temperature}°C (Feels like: {feelsLike}°C)
                    </h5>
                    <p className="card-text">🌥️ {weatherDescription.charAt(0).toUpperCase() + weatherDescription.slice(1)}</p>
                    <p className="card-text">Wind Speed: {weather.wind.speed} m/s</p>
                </div>
            </div>

            <h3 className="text-center mb-3">Interesting Places to Visit</h3>
            <div className="list-group">
                {places.map((place) => (
                    <div key={place.id} className="list-group-item bg-dark text-light">
                        <h5>{place.title}</h5>
                        <p>{place.description}</p>
                    </div>
                ))}
            </div>
        </div>
    );
}

export default LocationDetails;
