# GravPart

A small C++ utility for the spatial partitioning of geographic positions, using a very simple strategy based on center of mass computation.

**Warning**: this is just a proof-of-concept exercise that I wrote to brush up and update my C/C++ skills during an otherwise boring Sunday. I have not finished debugging this yet and it misses organized tests and error checking, so don't expect it to be correct. It is not!

Overall, it's just an exercise, don't take it seriously! There are better ways and countless libraries to perform spatial partitioning, indexing and clustering.


## Input 
A csv file containing geographic positions, in the following format `user_id,latitude,longitude`.
It's important that the csv file contains this exact header and columns.
- Latitudes and longitudes values are expected to be in the WGS84 reference, expressed in decimal degrees. Moreover, positions will be projected to [UTM][1]. It is up to the user to make sure that this transformation makes sense (that is, all locations must be inside or near the same UTM zone).
- The `user_id` column is meant to provide an identifier for the locations, given that the output won't be in the same order. An increasing integer will do, or write a constant value if you don't care. The parser expects this column.

[1]: https://en.wikipedia.org/wiki/Universal_Transverse_Mercator_coordinate_system


## Output

Two files, one with the original positions, now labeled with the identifier (`gid`) of their partition, and a second one with all the partitions, each with its identifier and center. The center always coincides with one of the locations in the partition (the nearest to the center of mass of the group).
- Positions file: A csv of labeled locations, with 4 columns: `(id, gid, lat, lon)`.
- Centers file: A csv with the computed partitions and their centers, with 3 columns `(gid, lat, lon)`.

The output files are in the same format as [PyGeoKMedoids](https://github.com/ggrocca/pygeokmedoids), a small python spatial clustering project. PyGeoKMedoids may provide additional utilities to analyze and process the output.



## Algorithm

- The strategy is to recursively split the Cartesian plane (where the projected locations are) into four rectangular quadrants.
- The splitting occurs on the center of mass of the points that are in the quadrant to be split, which should make the recursive splits (and the resulting rectangular shapes) guided by location density.
- Locations are moved into the new split quadrants (which are a subdivision of their parent) and the parent is left empty.
- The splitting stops when a quadrant contains less locations than a given threshold, and there is no quadrant left to split.
- The output partitions correspond to the non-empty quadrants that are left, which form a perfect covering of the original space.

Note that this procedure roughly resembles building a [Quadtree][2], but the computational complexity is different (and worst), because each step requires computing a new set of center of mass computations, which is a global problem over the input. I think that the complexity involved in building this structure for locations arranged in a non-degenerate fashion should be O(N*(N-D)), where N is the number of input locations and D is a factor related to how big the stop threshold is, but I haven't done a careful analysis.

This structure is not suitable to big data analysis, but it should work OK with small and medium sized data sets (tens or hundreds of thousands of points: probably OK; millions or billions of points: maybe not so much).

[2]: https://en.wikipedia.org/wiki/Quadtree


## Dependencies

- A compiler supporting modern C/C++ features (C17, C++20). I've used GCC11.
- Gnu Make. Don't write portable Makefiles; use a portable Make! Unless you want to compile it yourself, obviously.
- [PROJ](https://proj.org/), a generic coordinate transformation library.
- [GSL - Guidelines Support Library](https://github.com/microsoft/GSL) (I don't use this directly but argparse, one of the header libraries I'm using, does).

On MacOS, using brew:
```
brew install make gcc proj cpp-gsl
```
If you're on MacOS, make sure (pun intended) that you're running Make as installed by brew, and not the system one!

## Compilation

Clone the project, and run `make` in the root directory:
```
make
```
Edit the `Makefile` if installed dependencies are not found on your system `include` and `lib` paths.


## Running

Command line example:
```
./gravpart -c input.csv -o output_name -d 100
```
This will divide the input locations into clusters containing less than 100 locations each. Output files will be `output_80_centers.csv` and `output_80_positions.csv`.

## Note: UTM projection module

This project is C++, with the exception of `geoproject.c`, a simple C interface to the [PROJ](https://proj.org/) library. A simple, standalone test and example for this module is provided as `geoproject_test.c`.

- Test compilation (MacOS, `libproj` installed with brew):
```
gcc-11 -Wall -std=c17 geoproject.c geoproject_test.c -o geoproject_test -I/usr/local/opt/proj/include/ -L/usr/local/opt/proj/lib/ -lproj
```
- Alternatively, if `libproj` is already in your paths:
```
gcc-11 -Wall -std=c17 geoproject.c geoproject_test.c -o geoproject_test -lproj
```
- Test run:
```
geoproject_test input.dat
```

- The input file is text based, with the following format: it must begin with a single integer containing column length, then two columns of coordinates (lon lat), space separated. For example:
```
3
7.1 45.1
7.2 45.3
7.3 45.3
```


## Credits

I've directly incorporated into the project the following header only libraries (all credits to the original authors):
- [Argparse](https://github.com/p-ranav/argparse).
- [Vince's CSV Parser](https://github.com/vincentlaucsb/csv-parser).
- [stduuid](https://github.com/mariusbancila/stduuid).

The build system is [eggmake](https://github.com/ggrocca/eggmake), a lightweight Makefile engine, written by me.
