package ru.nsu.dmustakaev.api.dto.place;

import lombok.*;

@AllArgsConstructor
@NoArgsConstructor
@Getter
public class PlaceDetailsDto {
    private String id;
    private String title;
    private String description;
}
