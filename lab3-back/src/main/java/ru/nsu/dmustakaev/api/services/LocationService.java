package ru.nsu.dmustakaev.api.services;

import lombok.AllArgsConstructor;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.web.reactive.function.client.WebClient;
import ru.nsu.dmustakaev.api.dto.location.LocationDetailsDto;
import ru.nsu.dmustakaev.api.dto.location.LocationsDto;
import ru.nsu.dmustakaev.api.dto.place.PlaceDetailsDto;
import ru.nsu.dmustakaev.api.dto.place.PlacesDto;
import ru.nsu.dmustakaev.api.dto.weather.WeatherDto;
import ru.nsu.dmustakaev.config.LocationConfig;

import java.util.List;
import java.util.concurrent.CompletableFuture;

@Service
@AllArgsConstructor
public class LocationService {
    private final WeatherService weatherService;
    private final PlacesService placesService;
    private final WebClient webClient;
    private final LocationConfig locationConfig;

    @Autowired
    public LocationService(
            WeatherService weatherService,
            PlacesService placesService,
            WebClient.Builder webClientBuilder,
            LocationConfig locationConfig
    ) {
        this.weatherService = weatherService;
        this.placesService = placesService;
        this.webClient = webClientBuilder.baseUrl(locationConfig.getApiUrl()).build();
        this.locationConfig = locationConfig;
    }

    public CompletableFuture<LocationsDto> getLocations(
            String query
    ) {
        return webClient.get()
                .uri(uriBuilder -> uriBuilder.path("/geocode")
                        .queryParam("q", query)
                        .queryParam("locale", "ru")
                        .queryParam("key", locationConfig.getApiKey())
                        .build())
                .retrieve()
                .bodyToMono(LocationsDto.class)
                .toFuture();
    }

    public CompletableFuture<LocationDetailsDto> getLocationDetails(
            double lat,
            double lon
    ) {
        CompletableFuture<PlacesDto> placesFuture = placesService.getPlaces(lat, lon);
        CompletableFuture<WeatherDto> weatherFuture = weatherService.getWeather(lat, lon);

        CompletableFuture<List<PlaceDetailsDto>> placeDetailsFuture = placesFuture
                .thenApply(placesDto -> placesDto
                        .getResults()
                        .stream()
                        .map(placeDto -> placesService.getPlaceDetails(Long.parseLong(placeDto.getId())))
                        .toList()
                )
                .thenCompose(futures -> CompletableFuture.allOf(futures.toArray(new CompletableFuture[0]))
                        .thenApply(v -> futures.stream()
                                .map(CompletableFuture::join)
                                .toList())
                );

        return placeDetailsFuture.thenCombine(weatherFuture, (places, weather) -> LocationDetailsDto.builder()
                .places(places)
                .weather(weather)
                .build());

    }
}

