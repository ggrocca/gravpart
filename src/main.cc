#include <iostream>
#include <fstream>
#include <tuple>
#include <vector>
#include <memory>
#include <string>

using std::string;
using std::vector;
using std::unique_ptr;
using std::make_unique;
using std::size_t;
using std::to_string;
using std::endl;
using std::cerr;
using std::make_tuple;

#include "lib/argparse.hpp" // Downloaded on 2021-06-06 from https://raw.githubusercontent.com/p-ranav/argparse/master/include/argparse/argparse.hpp
#include "lib/csv.hpp"      // Downloaded on 2021-06-06 from https://raw.githubusercontent.com/vincentlaucsb/csv-parser/master/single_include/csv.hpp
#define UUID_SYSTEM_GENERATOR
#include "lib/uuid.h"       // downloaded on 2021-06-06 from https://raw.githubusercontent.com/mariusbancila/stduuid/master/include/uuid.h

// partioning method and data structures
#include "gravitypartition.hh"
using namespace gravpart;

// This would work better wrapped inside a more C++ friendly interface, but it's good enough for now.
#include "geoproject.h"


// usage: [-h] -c CSV_INPUT [-o OUTPUT_NAME] [-d DIM_MAX]
int main (int argc, char* argv[])
{
    argparse::ArgumentParser program (argv[0]);

    program.add_argument ("-c", "--csv-input")
        .default_value (std::string("input.csv"))
        .help ("Input path to a csv file, with format (user_id,latitude,longitude). A header line with correct column names must be present.");
    program.add_argument ("-o", "--output-name")
        .default_value (std::string("output"))
        .help ("Output path and name, to be applied as prefix of all generated files (default = 'output')");
    program.add_argument ("-d", "--dim-max")
        .help("Maximum numbers of locations in each set. (default = 80)")
        .default_value (80)
        .action([](const std::string& value) { return std::stoi(value); });

    try {
        program.parse_args (argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cout << err.what () << endl;
        std::cout << program;
        exit(0);
    }

    auto input_csv = program.get<std::string> ("-c");
    auto output_name = program.get<std::string> ("-o");
    auto maxdim = program.get<int> ("-d");

    // load csv (user_id,latitude,longitude)
    csv::CSVReader reader (input_csv);

    auto lons = vector<double> ();
    auto lats = vector<double> ();
    auto xs = vector<double> ();
    auto ys = vector<double> ();
    auto uids = vector<string> ();

    for (auto& row: reader) {
        if (!(row["latitude"].is_float () && row["longitude"].is_float ())) {
            cerr << "Error in CSV parsing: latitude and/or longitude seem not to be floating point values." << endl;
            exit (-1);
        }
        lons.push_back (row["longitude"].get<double>());
        lats.push_back (row["latitude"].get<double>());
        uids.push_back (row["user_id"].get<>());
        xs.push_back (0.0);
        ys.push_back (0.0);
    }

    // convert from geo to utm
    char* epsgutmz = nullptr;
    geo2utm (&lons[0], &lats[0], &xs[0], &ys[0], uids.size (), &epsgutmz);

    // convert from columns to points
    auto ps = make_unique<PointVector> (PointVector());
    for (size_t i = 0; i < uids.size (); i++)
        ps->emplace_back (xs[i], ys[i], uids[i]);

    // partition
    auto partitions = vector<GravBox> ();
    GravBox::compute_gravity_partitions (std::move (ps), partitions, maxdim);
    ps.reset ();

    // convert positions and centers from points to columns, and assign group_ids to the resulting partitions
    auto gids = vector<string> ();
    uids.clear ();
    xs.clear ();
    ys.clear ();
    auto c_lons = vector<double> ();
    auto c_lats = vector<double> ();
    auto c_xs = vector<double> ();
    auto c_ys = vector<double> ();
    auto c_gids = vector<string> ();

    for (auto& g : partitions) {
        string gid = uuids::to_string (uuids::uuid_system_generator{}());
        c_xs.push_back (g.c.x);
        c_ys.push_back (g.c.y);
        c_lons.push_back (0.0);
        c_lats.push_back (0.0);
        c_gids.push_back (gid);

        for (auto& p: *g.ps) {
            xs.push_back (p.x);
            ys.push_back (p.y);
            uids.push_back (p.uid);
            gids.push_back (gid);
        }
    }

    // convert positions and centers from utm to geo
    utm2geo (&xs[0], &ys[0], &lons[0], &lats[0], uids.size (), epsgutmz);
    utm2geo (&c_xs[0], &c_ys[0], &c_lons[0], &c_lats[0], c_gids.size (), epsgutmz);
    release_epsgutmz (epsgutmz);

    // output positions
    std::ofstream p_ofs;
    string output_positions_csv = output_name+"_"+to_string(maxdim)+"_positions.csv";
    p_ofs.open (output_positions_csv, std::ofstream::out);
    if (!p_ofs.is_open ()) {
        cerr << "Error opening output file " << output_positions_csv << endl;
        exit (-2);
    }

    auto p_writer = csv::make_csv_writer(p_ofs);
    p_writer << make_tuple("uid", "gid", "latitude", "longitude");
    for (size_t i = 0; i < uids.size (); i++)
        p_writer << make_tuple(uids[i], gids[i], lats[i], lons[i]);
    p_ofs.close();

    // output centers
    std::ofstream c_ofs;
    string output_centers_csv = output_name+"_"+to_string(maxdim)+"_centers.csv";
    c_ofs.open (output_centers_csv, std::ofstream::out);
    if (!c_ofs.is_open ()) {
        cerr << "Error opening output file " << output_centers_csv << endl;
        exit (-2);
    }

    auto c_writer = csv::make_csv_writer(c_ofs);
    c_writer << make_tuple("gid", "latitude", "longitude");    
    for (size_t i = 0; i < c_gids.size (); i++)
        c_writer << make_tuple(c_gids[i], c_lats[i], c_lons[i]);
    c_ofs.close();
}
