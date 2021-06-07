#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <proj.h>

#include "geoproject.h"

#define GEOPROJECT_STRLEN 64

unsigned lonlat2znum (double lon, double lat) {
    if (lat >= 72.0 && lat <= 84.0 && lon >= 0) {
        if (lon < 9.0)
            return 31;
        else if (lon < 21.0)
            return 33;
        else if (lon < 33.0)
            return 35;
        else if (lon < 42.0)
            return 37;
    }
    return ((int)((lon + 180) / 6)) + 1;
}

char* zone_letters = "CDEFGHJKLMNPQRSTUVWXX";
char lat2zletter (double lat)  {
    return (lat >= -80.0 &&  lat <= 84.0)? zone_letters[((int)lat + 80) >> 3] : 0;
}

char* lonlat2epsgutmz (double lon, double lat) {
    return znumzletter2epsgutmz (lonlat2znum (lon, lat), lat2zletter (lat));
}

char* znumzletter2epsgutmz (unsigned num, char letter) {
    // Warning: this should really check for out of bounds num and letters.

    char* epsgutmz = calloc (GEOPROJECT_STRLEN, sizeof (*epsgutmz));
    
    // EPSG code for a northern hemisphere UTM: add 32600. For southern hemisphere UTMs: add 32700.
    unsigned base = (letter >= 'N')? 32600 : 32700;

    snprintf (epsgutmz,  GEOPROJECT_STRLEN, "EPSG:%d", base + num);

    return epsgutmz;
}

void release_epsgutmz (char* epsgutmz) {
    free (epsgutmz);
}

// if !inv, (lons,lats) -> (xs, ys)    [ EPSG:4326 -> epsgutmz  ]
// if inv,  (xs, ys)    -> (lons,lats) [ epsgutmz  -> EPSG:4326 ]
int transform (double* lons, double* lats, double* xs, double* ys, unsigned len, char* epsgutmz, bool inv) {
    PJ_CONTEXT *C;
    PJ *P;
    PJ* P_for_GIS;
    PJ_COORD a, b;

    C = proj_context_create();
    P = proj_create_crs_to_crs (C, "EPSG:4326", epsgutmz, NULL);
    if (0==P) {
        fprintf(stderr, "geoproject.c: Could not create context or projection.\n");
        return 1;
    }

    P_for_GIS = proj_normalize_for_visualization(C, P);
    if( 0 == P_for_GIS )  {
        fprintf(stderr, "geoproject.c: projection normalization failed.\n");
        return 1;
    }
    proj_destroy(P);
    P = P_for_GIS;
    
    for (unsigned int i = 0; i < len; i++) {
        if (!inv) { /* transform to UTM zone */
            /* Given that we have used proj_normalize_for_visualization(), the order of */
            /* coordinates is longitude, latitude, and values are expressed in degrees. */
            a = proj_coord (lons[i], lats[i], 0, 0);
            b = proj_trans (P, PJ_FWD, a);
            xs[i] = b.enu.e;
            ys[i] = b.enu.n;
        }
        else { /* transform to geo coords */
            a = proj_coord (xs[i], ys[i], 0, 0);
            b = proj_trans (P, PJ_INV, a);
            lons[i] = b.lp.lam;
            lats[i] = b.lp.phi;
        }
    }

    /* Clean up */
    proj_destroy (P);
    proj_context_destroy (C);
    return 0;
}

// epsgutmz must be a valid string, as returned by znumzletter2epsgutmz or lonlat2epsgutmz.
int utm2geo (double* input_xs, double* input_ys, double* output_lons, double* output_lats, unsigned len, char* epsgutmz) {
    return transform (output_lons, output_lats, input_xs, input_ys, len, epsgutmz, true);
}

// If *epsgutmz is NULL, guess its value from the first coordinate and return it, otherwise use its value.
// Warning: it is up to the user to make sure that all points are in the same zone as the first, or close enough.
// Don't forget to call release afterwards on the epsgutmz string, otherwise it's a memory leak!
int geo2utm (double* input_lons, double* input_lats, double* output_xs, double* output_ys, unsigned len, char** epsgutmz) {
    char* guessed;
    guessed = *epsgutmz? *epsgutmz : lonlat2epsgutmz (input_lons[0], input_lats[0]);
    *epsgutmz = guessed;
    return transform (input_lons, input_lats, output_xs, output_ys, len, guessed, false);
}
