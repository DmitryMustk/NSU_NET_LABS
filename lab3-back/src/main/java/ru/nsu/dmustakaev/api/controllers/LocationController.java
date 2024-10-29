package ru.nsu.dmustakaev.api.controllers;

import lombok.AllArgsConstructor;
import org.springframework.web.bind.annotation.*;
import ru.nsu.dmustakaev.api.dto.location.LocationDetailsDto;
import ru.nsu.dmustakaev.api.dto.location.LocationsDto;
import ru.nsu.dmustakaev.api.services.LocationService;

import java.util.concurrent.CompletableFuture;

@RestController
@RequestMapping("/api")
@AllArgsConstructor
public class LocationController {
    private final LocationService locationService;

    @CrossOrigin(origins = "http://localhost:3000")
    @GetMapping("/locations")
    public CompletableFuture<LocationsDto> getLocations(
            @RequestParam String location
    ) {
        return locationService.getLocations(location);
    }

    @CrossOrigin(origins = "http://localhost:3000")
    @GetMapping("/location-details")
    public CompletableFuture<LocationDetailsDto> getLocationDetails(
            @RequestParam double lat,
            @RequestParam double lon
    ) {
        return locationService.getLocationDetails(lat, lon);
    }

}
