package ru.nsu.dmustakaev.config;

import lombok.Getter;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Configuration;

@Configuration
@Getter
public class LocationConfig {
    @Value("${location.api.url}")
    private String apiUrl;

    @Value("${location.api.key}")
    private String apiKey;
}
