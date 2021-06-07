// Quick'n'dirty test for geoproject.c
// compile:
// gcc-11 -Wall -std=c17 geoproject.c geoproject_test.c -o geoproject_test -I/usr/local/opt/proj/include/ -L/usr/local/opt/proj/lib/ -lproj
// command line: geoproject file.dat
// file.dat: a single integer containing column length, then two columns of coordinates (lon lat), space separated.
// EG: $ cat > prova.dat
// ----
// 3
// 7.1 45.1
// 7.2 45.3
// 7.3 45.3
// ----
// $ geoproject_test prova.dat
// innput coordinates must be geographic coordinates, WGS84.
// output: the input, then UTM coordinates, the string of the UTM zone, coordinates converted back to lonlat

#include <stdio.h>
#include <stdlib.h>

#include "geoproject.h"

int main (int argc, char *argv[]) {

    if (argc != 2)
        return -1;

    FILE* fp = fopen (argv[1], "r");

    if (!fp)
        return -2;
    unsigned len = 0;
    fscanf (fp, "%d", &len);
    
    double* input_x = malloc (sizeof(input_x) * len);
    double* input_y = malloc (sizeof(input_y) * len);
    double* output_x = malloc (sizeof(output_x) * len);
    double* output_y = malloc (sizeof(output_y) * len);
    double* back_x = malloc (sizeof(back_x) * len);
    double* back_y = malloc (sizeof(back_y) * len);

    for (unsigned i = 0; i < len; i++)
        fscanf (fp, "%lf %lf\n", &input_x[i], &input_y[i]);

    printf ("%d\n", len);
    for (unsigned i = 0; i < len; i++)
        printf ("%lf %lf\n", input_x[i], input_y[i]);

    char* epsgutmz = NULL;
    geo2utm (input_x, input_y, output_x, output_y, len, &epsgutmz);

    for (unsigned i = 0; i < len; i++)
        printf ("%lf %lf\n", output_x[i], output_y[i]);
    printf ("%s\n", epsgutmz);

    utm2geo (output_x, output_y, back_x, back_y, len, epsgutmz);

    for (unsigned i = 0; i < len; i++)
        printf ("%lf %lf\n", back_x[i], back_y[i]);

    release_epsgutmz (epsgutmz);
    free (input_x);
    free (input_y);
    free (output_x);
    free (output_y);
    free (back_x);
    free (back_y);
}
