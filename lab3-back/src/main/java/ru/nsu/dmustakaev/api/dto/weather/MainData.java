package ru.nsu.dmustakaev.api.dto.weather;

import lombok.*;

@AllArgsConstructor
@NoArgsConstructor
@Getter
public class MainData {
    private double temp;
    private double feels_like;
    private double temp_min;
    private double temp_max;
    private long pressure;
    private long humidity;
    private long sea_level;
    private long grnd_level;
}
