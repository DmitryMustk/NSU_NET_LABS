package ru.nsu.dmustakaev.api.controllers;

import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import ru.nsu.dmustakaev.api.dto.location.LocationsDto;
import ru.nsu.dmustakaev.api.services.LocationService;

import java.util.Optional;
import java.util.concurrent.CompletableFuture;

@RestController
@RequestMapping("/api")
@RequiredArgsConstructor
public class LocationController {
    private final LocationService locationService;

    @GetMapping("/locations")
    public CompletableFuture<LocationsDto> getLocations(@RequestParam String location,
                                                        @RequestParam(required = false) int limit
    ) {
        return locationService.getLocation(location, Optional.of(limit));
    }
}
