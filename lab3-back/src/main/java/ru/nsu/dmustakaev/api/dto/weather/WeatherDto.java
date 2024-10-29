package ru.nsu.dmustakaev.api.dto.weather;

import lombok.*;

import java.util.List;

@AllArgsConstructor
@NoArgsConstructor
@Getter
public class WeatherDto {
    private Coordinates coord;
    private List<Weather> weather;
    private String base;
    private MainData main;
    private long visibility;
    private Wind wind;
    private Clouds clouds;
    private long dt;
    private Sys sys;
    private long timezone;
    private long id;
    private String name;
    private long cod;
}
