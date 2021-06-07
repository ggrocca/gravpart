#pragma once

#ifndef GRAVPART_H_
#define GRAVPART_H_

#include <vector>
#include <memory>
#include <ranges>
#include <string>

using std::string;
using std::vector;
using std::unique_ptr;
using std::make_unique;
using std::size_t;



namespace gravpart {



    class Point {

    public:
        double x;
        double y;
        string uid;

        Point () : x (0.0), y (0.0), uid () {}
        Point (double x, double y) : x (x), y (y), uid () {}
        Point (double x, double y, string uid) : x (x), y (y), uid (uid) {}

        Point& operator+= (const Point& rhs)
        {
            this->x += rhs.x;
            this->y += rhs.y;
            return *this;
        }
        friend Point operator+ (Point lhs, const Point& rhs) {
            lhs += rhs;
            return lhs;
        }

        Point& operator-= (const Point& rhs)
        {
            this->x -= rhs.x;
            this->y -= rhs.y;
            return *this;
        }

        friend Point operator- (Point lhs, const Point& rhs) {
            lhs -= rhs;
            return lhs;
        }

        Point& operator/= (const Point& rhs)
        {
            this->x /= rhs.x;
            this->y /= rhs.y;
            return *this;
        }

        friend Point operator/ (Point lhs, const Point& rhs) {
            lhs /= rhs;
            return lhs;
        }

        Point& operator/= (const double& rhs)
        {
            this->x /= rhs;
            this->y /= rhs;
            return *this;
        }

        friend Point operator/ (Point lhs, const double& rhs) {
            lhs /= rhs;
            return lhs;
        }

        static double sqds (const Point& a, const Point& b) {
            Point d = a - b;
            return d.x * d.x + d.y * d.y;
        }

        double sqds (const Point& a) {
            return sqds (*this, a);
        }

        enum class RelativePosition { NorthWest, SouthWest, SouthEast, NorthEast };    
        RelativePosition get_rp (Point p) {
            if (p.x >= x) // easth
                if (p.y >= y) // north
                    return RelativePosition::NorthEast;
                else // south
                    return RelativePosition::SouthEast;
            else // west
                if (p.y >= y) // north
                    return RelativePosition::NorthWest;
                else // south
                    return RelativePosition::SouthWest;
        }

    };



    typedef vector<Point> PointVector;



    class GravBox {

    public:

        unique_ptr<PointVector> ps;
        Point c;

        GravBox () : ps (make_unique<PointVector> (PointVector())), c (Point ()) {}
        GravBox (unique_ptr<PointVector> aps) : ps (std::move (aps)), c(Point ()) {
            for (auto& p : *ps)
                c += p;
            c /= ps->size ();
        };

        static void compute_gravity_partitions (unique_ptr<PointVector> ps, vector<GravBox>& result, size_t maxdim);

    private:

        size_t split (vector<GravBox>& splitted);
        void snap_center ();
    };



}



#endif
