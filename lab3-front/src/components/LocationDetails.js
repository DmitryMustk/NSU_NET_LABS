import React, { useEffect, useState } from "react";
import axios from "axios";
import { useParams } from "react-router-dom";

function LocationDetails() {
    const { lat, lon } = useParams();
    const [weather, setWeather] = useState(null);

    useEffect(() => {
        const fetchWeather = async () => {
            const response = await axios.get(`http://localhost:8080/api/weather?lat=${lat}&lon=${lon}`);
            setWeather(response.data);
        };
        fetchWeather();
    }, [lat, lon]);

    const getWeatherEmoji = (description) => {
        if (description.includes("clear")) return "☀️";
        if (description.includes("clouds")) return "☁️";
        if (description.includes("rain")) return "🌧️";
        if (description.includes("thunderstorm")) return "⛈️";
        if (description.includes("snow")) return "❄️";
        if (description.includes("mist")) return "🌫️";
        return "🌈";
    };

    const kelvinToCelsius = (tempK) => {
        return (tempK - 273.15).toFixed(2);
    };

    if (!weather) {
        return <div>Loading...</div>;
    }

    const weatherEmoji = getWeatherEmoji(weather.weather[0].description);

    return (
        <div className="container">
            <h2>Weather in {weather.name}</h2>
            <div className="card bg-dark text-light">
                <div className="card-body">
                    <h4 className="card-title">
                        {weatherEmoji} Temperature: {kelvinToCelsius(weather.main.temp)}°C
                    </h4>
                    <p className="card-text">Feels like: {kelvinToCelsius(weather.main.feels_like)}°C</p>
                    <p className="card-text">
                        {weatherEmoji} Weather: {weather.weather[0].description}
                    </p>
                    <p className="card-text">Humidity: {weather.main.humidity}%</p>
                    <p className="card-text">Wind Speed: {weather.wind.speed} m/s</p>
                </div>
            </div>
        </div>
    );
}

export default LocationDetails;
