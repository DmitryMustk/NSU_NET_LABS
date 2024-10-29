package ru.nsu.dmustakaev.api.dto.location;

import lombok.*;

import java.util.List;

@AllArgsConstructor
@NoArgsConstructor
@Getter
public class LocationsDto {
    private List<Hit> hits;
}
