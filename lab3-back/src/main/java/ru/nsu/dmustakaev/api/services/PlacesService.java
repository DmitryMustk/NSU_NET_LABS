package ru.nsu.dmustakaev.api.services;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.web.reactive.function.client.WebClient;
import ru.nsu.dmustakaev.api.dto.place.PlaceDetailsDto;
import ru.nsu.dmustakaev.api.dto.place.PlacesDto;
import ru.nsu.dmustakaev.config.PlaceConfig;

import java.util.concurrent.CompletableFuture;

@Service
public class PlacesService {
    private final WebClient webClient;

    @Autowired
    public PlacesService(
            WebClient.Builder webClientBuilder,
            PlaceConfig placeConfig
    ) {
        this.webClient = webClientBuilder
                .baseUrl(placeConfig.getApiUrl())
                .build();
    }

    public CompletableFuture<PlacesDto> getPlaces(
            double lat,
            double lon
    ) {
        return webClient.get()
                .uri(uriBuilder -> uriBuilder.path("/places/")
                        .queryParam("fields", "id,title,description")
                        .queryParam("lat", lat)
                        .queryParam("lon", lon)
                        .queryParam("radius", 500)
                        .queryParam("page_size", 5)
                        .queryParam("text_format", "text")

                        .build())
                .retrieve()
                .bodyToMono(PlacesDto.class)
                .toFuture();
    }


    public CompletableFuture<PlaceDetailsDto> getPlaceDetails(
            long placeId
    ) {
        return webClient.get()
                .uri(uriBuilder -> uriBuilder.path("/places/{placeId}/")
                        .queryParam("fields", "id,title,description")
                        .queryParam("text_format", "text")
                        .build(placeId))
                .retrieve()
                .bodyToMono(PlaceDetailsDto.class)
                .toFuture();
    }
}
