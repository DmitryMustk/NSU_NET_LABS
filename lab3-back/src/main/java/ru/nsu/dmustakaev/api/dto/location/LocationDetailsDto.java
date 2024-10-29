package ru.nsu.dmustakaev.api.dto.location;

import lombok.Builder;
import lombok.Getter;
import ru.nsu.dmustakaev.api.dto.place.PlaceDetailsDto;
import ru.nsu.dmustakaev.api.dto.weather.WeatherDto;

import java.util.List;

@Getter
@Builder
public class LocationDetailsDto {
    private WeatherDto weather;
    private List<PlaceDetailsDto> places;
}
