/***
 *    Description:  Finfo Wrapper
 *
 *        Created:  2020-03-30

 *         Author:  Dilawar Singh <dilawar.s.rajput@gmail.com>
 *        License:  GPLv3
 */

#ifndef FINFO_H
#define FINFO_H

#include "../basecode/ObjId.h"
#include "helper.h"

class __Finfo__ {
public:
    __Finfo__(const ObjId& oid, const string& fname, const Finfo* f)
        : oid_(oid), fname_(fname), f_(f)
    {
        func_ = [oid, fname, f](const py::object& key) {
            return getLookupValueFinfoItem(oid, fname, key, f);
        };
    }

    py::object operator()(const py::object& key)
    {
        return func_(key);
    }

public:
    ObjId oid_;
    string fname_;
    const Finfo* f_;
    std::function<py::object(const py::object& key)> func_;
};

#endif /* end of include guard: FINFO_H */
