package ru.nsu.dmustakaev.api.dto.weather;

import lombok.*;

@AllArgsConstructor
@Getter
public class Wind {
    private double speed;
    private long deg;
    private double gust;
}
