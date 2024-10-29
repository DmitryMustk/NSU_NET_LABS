package ru.nsu.dmustakaev.api.dto.place;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;

import java.util.List;

@AllArgsConstructor
@NoArgsConstructor
@Getter
public class PlacesDto {
    private int count;
    private List<PlaceDto> results;
}
