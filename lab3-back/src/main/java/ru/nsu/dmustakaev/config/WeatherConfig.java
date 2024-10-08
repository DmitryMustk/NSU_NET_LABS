package ru.nsu.dmustakaev.config;

import lombok.Getter;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Configuration;

@Getter
@Configuration
public class WeatherConfig {
    @Value("${locationWeather.api.url}")
    private String apiUrl;

    @Value("${locationWeather.api.key}")
    private String apiKey;
}
