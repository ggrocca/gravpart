#pragma once

#ifndef GEOPROJECT_H_
#define GEOPROJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

    // returns a poinnter to a valid 'EPSG:23***' string. Allocated memory must be released.
    char* latlon2epsgutmz (double lon, double lat);
    
    // returns a poinnter to a valid 'EPSG:23***' string. Allocated memory must be released.
    char* znumzletter2epsgutmz (unsigned num, char letter);

    // releases a string returned by one of the previous functions, or passed by reference when using geo2utm in guessing mode.
    void release_epsgutmz (char* epsgutmz);

    // epsgutmz must be a valid string, as returned by znumzletter2epsgutmz or lonlat2epsgutmz.
    // Warning: it is up to the user to make sure that all points are in the same zone as the first, or close enough.
    int utm2geo (double* input_xs, double* input_ys, double* output_lons, double* output_lats, unsigned len, char* epsgutmz);

    // If *epsgutmz is NULL, guess its value from the first coordinate and return it, otherwise use its value.
    // Warning: it is up to the user to make sure that all points are in the given zone, or in the same zone as the first point, if guessing the zone, or close enough.
    // Don't forget to call release afterwards on the epsgutmz string, otherwise it's a memory leak!
    int geo2utm (double* input_lons, double* input_lats, double* output_xs, double* output_ys, unsigned len, char** epsgutmz);

#ifdef __cplusplus
}
#endif

#endif
