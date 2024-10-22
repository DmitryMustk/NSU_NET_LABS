package ru.nsu.dmustakaev.api.controllers;

import lombok.AllArgsConstructor;
import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;
import ru.nsu.dmustakaev.api.dto.weather.WeatherDto;
import ru.nsu.dmustakaev.api.services.WeatherService;

import java.util.concurrent.CompletableFuture;

@RestController
@RequestMapping("/api")
@AllArgsConstructor
public class WeatherController {
    private final WeatherService weatherService;

    @CrossOrigin(origins = "http://localhost:3000")
    @GetMapping("/weather")
    public CompletableFuture<WeatherDto> getWeather(@RequestParam double lat, @RequestParam double lon) {
        return weatherService.getWeather(lat, lon);
    }
}
