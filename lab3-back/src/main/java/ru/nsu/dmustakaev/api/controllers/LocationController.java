package ru.nsu.dmustakaev.api.controllers;

import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;
import ru.nsu.dmustakaev.api.dto.location.LocationsDto;
import ru.nsu.dmustakaev.api.services.LocationService;

import java.util.Optional;
import java.util.concurrent.CompletableFuture;

@RestController
@RequestMapping("/api")
@RequiredArgsConstructor
public class LocationController {
    private final LocationService locationService;

    @CrossOrigin(origins = "http://localhost:3000")
    @GetMapping("/locations")
    public CompletableFuture<LocationsDto> getLocations(@RequestParam String location) {
        return locationService.getLocation(location);
    }
}
