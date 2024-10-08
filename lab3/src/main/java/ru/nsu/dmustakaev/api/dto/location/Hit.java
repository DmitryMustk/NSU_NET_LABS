package ru.nsu.dmustakaev.api.dto.location;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class Hit {
    private Point point;
    private List<Double> extent;
    private String name;
    private String country;
    private String countrycode;
    private String city;
    private String state;
    private String postcode;
    private long osm_id;
    private String osm_type;
    private String osm_key;
    private String osm_value;
}
