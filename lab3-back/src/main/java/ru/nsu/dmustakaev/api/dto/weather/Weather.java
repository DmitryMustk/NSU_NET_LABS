package ru.nsu.dmustakaev.api.dto.weather;

import lombok.*;


@AllArgsConstructor
@NoArgsConstructor
@Getter
public class Weather {
    private long id;
    private String main;
    private String description;
    private String icon;
}
