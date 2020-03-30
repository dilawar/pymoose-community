/***
 *    Description:  moose.vec class.
 *
 *        Created:  2020-03-30

 *         Author:  Dilawar Singh <dilawar.s.rajput@gmail.com>
 *        License:  GPLv3 
 */

#ifndef VEC_H

#define VEC_H

#include <vector>
using namespace std;

class MooseVec {

public:

    MooseVec(const string& path, size_t n=1, const string& dtype="Neutral"): path_(path), n_(n), dtype_(dtype)
    {
        if(! mooseExists(path)) {
            objs_.clear();
            ObjId o = mooseCreate(dtype, path, n);
            for (size_t i = 0; i < n; i++) 
                objs_.push_back(ObjId(o, i));
        }
        else {
            objs_.clear();
            auto o = Id(path);
            for (size_t i = 0; i < o.element()->numData(); i++) 
                objs_.push_back(ObjId(o, i));
        }

    }

    size_t len()
    {
        return objs_.size();
    }

private:
    std::string path_;
    size_t n_;
    const std::string dtype_;
    std::vector<ObjId> objs_;
};


#endif /* end of include guard: VEC_H */
