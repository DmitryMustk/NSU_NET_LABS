package ru.nsu.dmustakaev.config;

import lombok.Getter;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Configuration;

@Getter
@Configuration
public class PlaceConfig {
    @Value("${place.api.url}")
    private String apiUrl;
}
