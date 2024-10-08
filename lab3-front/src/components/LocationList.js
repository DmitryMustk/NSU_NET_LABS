import React from "react";

function LocationList({ locations = [], onSelect }) {
  return (
    <div>
      <h3>Search results: </h3>
      <ul>
        {locations.length > 0 ? (
          locations.map((location) => (
            <li key={location.osm_id} onClick={() => onSelect(location)}>
              <strong>{location.name}</strong> <br />
			  <em>Type:</em> {location.osm_value || "Unknown"}, <br />
              <em>Country:</em> {location.country || "Unknown"}, <br />
              <em>State:</em> {location.state || "Unknown"}, <br />
              <em>Coordinates:</em> {location.point.lat}, {location.point.lng} <br />
              <em>Postcode:</em> {location.postcode || "N/A"} <br />
              <hr />
            </li>
          ))
        ) : (
          <li>No results found</li>
        )}
      </ul>
    </div>
  );
}

export default LocationList;
