package ru.nsu.dmustakaev.config;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Configuration;

@Configuration
public class LocationWeatherConfig {
    @Value("${locationWeather.api.url}")
    private String apiUrl;

    @Value("${locationWeather.api.key}")
    private String apiKey;
}
