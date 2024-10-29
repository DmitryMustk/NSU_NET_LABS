package ru.nsu.dmustakaev.api.dto.location;

import lombok.*;

import java.util.List;

@AllArgsConstructor
@NoArgsConstructor
@Getter
public class Hit {
    private Point point;
    private String name;
    private String country;
    private String city;
    private String state;
    private String osm_key;
    private String osm_value;
}
