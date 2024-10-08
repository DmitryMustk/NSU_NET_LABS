package ru.nsu.dmustakaev.api.services;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.web.reactive.function.client.WebClient;
import ru.nsu.dmustakaev.api.dto.weather.WeatherDto;
import ru.nsu.dmustakaev.config.WeatherConfig;

import java.util.concurrent.CompletableFuture;

@Service
public class WeatherService {
    private final WebClient webClient;
    private final WeatherConfig weatherConfig;

    @Autowired
    public WeatherService(WebClient.Builder webClient, WeatherConfig weatherConfig) {
        this.webClient = webClient.baseUrl(weatherConfig.getApiUrl()).build();
        this.weatherConfig = weatherConfig;
    }

    public CompletableFuture<WeatherDto> getWeather(double lat, double lon) {
        return webClient.get()
                .uri(uriBuilder -> uriBuilder.path("/weather")
                        .queryParam("lat", lat)
                        .queryParam("lon", lon)
                        .queryParam("appid", weatherConfig.getApiKey())
                        .build())
                .retrieve()
                .bodyToMono(WeatherDto.class)
                .toFuture();

    }
}
