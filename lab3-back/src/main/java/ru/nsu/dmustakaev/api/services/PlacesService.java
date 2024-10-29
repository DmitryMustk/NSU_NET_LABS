package ru.nsu.dmustakaev.api.services;

import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.http.codec.json.Jackson2JsonDecoder;
import org.springframework.stereotype.Service;
import org.springframework.util.MimeTypeUtils;
import org.springframework.web.reactive.function.client.ExchangeStrategies;
import org.springframework.web.reactive.function.client.WebClient;
import ru.nsu.dmustakaev.api.dto.place.PlaceDetailsDto;
import ru.nsu.dmustakaev.api.dto.place.PlacesDto;

import java.util.concurrent.CompletableFuture;

@Service
public class PlacesService {
    private final WebClient webClient;

    @Autowired
    public PlacesService(WebClient.Builder webClientBuilder) {
        this.webClient = webClientBuilder.baseUrl("https://kudago.com/public-api/v1.4/")
                .defaultHeader(HttpHeaders.ACCEPT, MediaType.APPLICATION_JSON_VALUE)
                .defaultHeader(HttpHeaders.CONTENT_TYPE, MediaType.APPLICATION_JSON_VALUE)
                .exchangeStrategies(ExchangeStrategies.builder().codecs(configurer ->{
                    ObjectMapper mapper = new ObjectMapper();
                    mapper.configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false);
                    configurer.customCodecs().register(new Jackson2JsonDecoder(mapper, MimeTypeUtils.parseMimeType(MediaType.TEXT_PLAIN_VALUE)));
                }).build())
                .build();
    }

    public CompletableFuture<PlacesDto> getPlaces(double lat, double lon) {
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


    public CompletableFuture<PlaceDetailsDto> getPlaceDetails(long placeId) {
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
