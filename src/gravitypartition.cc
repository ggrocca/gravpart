#include "gravitypartition.hh"


namespace gravpart {


#include <type_traits>
    template <typename E>
    constexpr auto to_underlying (E e) noexcept {
        return static_cast<std::underlying_type_t<E>> (e);
    }



    size_t GravBox::split (vector<GravBox>& splits) {
        splits.clear ();

        if (ps->size () > 1) {
            for (size_t i : std::ranges::views::iota(0, 4))
                splits.emplace_back (), (void)i;
        
            for (auto p : *ps) {
                size_t index = to_underlying (c.get_rp (p));
                splits[index].ps->push_back (p);
                splits[index].c += p;
            }

            for (auto gb_it = splits.begin (); gb_it != splits.end ();)
                if (gb_it->ps->size () > 0) {
                    gb_it->c /= gb_it->ps->size ();
                    ++gb_it;
                }
                else
                    gb_it = splits.erase(gb_it);
        }

        return splits.size ();
    }

    void GravBox::snap_center () {
        Point cc = c;
        c = *std::min_element (ps->begin (), ps->end (), 
                                        [&cc] (auto& a, auto& b) { return cc.sqds (a) <  cc.sqds (a); }
                                        );
    }


    void GravBox::compute_gravity_partitions (unique_ptr<vector<Point>> ps, vector<GravBox>& result, size_t maxdim) {
        result.clear ();
        result.emplace_back (std::move (ps));
        ps.reset ();

        for (size_t i = 0; i < result.size (); i++) {
            if (result[i].ps->size () <= maxdim)
                continue;

            auto splits = vector<GravBox>();
            if (result[i].split (splits) == 0)
                continue;
            result.insert (result.end (), std::make_move_iterator (splits.begin ()), std::make_move_iterator (splits.end ()));
            result[i].ps.reset ();
        }

        result.erase (std::remove_if (result.begin (), result.end (), 
                                      [] (auto& gb) { return gb.ps == nullptr; } ),
                      result.end ());
        
        for (auto& gb : result)
            gb.snap_center ();
    }


}
