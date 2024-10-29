package ru.nsu.dmustakaev.api.dto.weather;

import lombok.*;

@AllArgsConstructor
@NoArgsConstructor
@Getter
public class Coordinates {
    private double lon;
    private double lat;
}
