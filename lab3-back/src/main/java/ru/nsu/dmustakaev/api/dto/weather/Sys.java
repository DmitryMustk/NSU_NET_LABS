package ru.nsu.dmustakaev.api.dto.weather;

import lombok.*;

@AllArgsConstructor
@NoArgsConstructor
@Getter
public class Sys {
    private long type;
    private long id;
    private String country;
    private long sunrise;
    private long sunset;
}
