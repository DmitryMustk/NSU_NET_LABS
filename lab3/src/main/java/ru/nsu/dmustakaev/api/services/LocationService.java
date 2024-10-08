package ru.nsu.dmustakaev.api.services;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.web.reactive.function.client.WebClient;
import ru.nsu.dmustakaev.api.dto.location.LocationsDto;
import ru.nsu.dmustakaev.config.LocationConfig;

import java.util.Optional;
import java.util.concurrent.CompletableFuture;

@Service
public class LocationService {
    private final WebClient webClient;
    private final LocationConfig locationConfig;

    @Autowired
    public LocationService(WebClient.Builder webClientBuilder, LocationConfig locationConfig) {
        this.webClient = webClientBuilder.baseUrl(locationConfig.getApiUrl()).build();
        this.locationConfig = locationConfig;
    }

    public CompletableFuture<LocationsDto> getLocation(String query, Optional<Integer> limit) {
        return webClient.get()
                .uri(uriBuilder -> uriBuilder.path("/geocode")
                        .queryParam("q", query)
                        .queryParam("locale", "ru")
                        .queryParam("key", locationConfig.getApiKey())
                        .queryParamIfPresent("limit", limit)
                        .build())
                .retrieve()
                .bodyToMono(LocationsDto.class)
                .toFuture();
    }
}
