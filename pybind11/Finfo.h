/***
 *    Description:  Finfo Wrapper
 *
 *        Created:  2020-03-30

 *         Author:  Dilawar Singh <dilawar.s.rajput@gmail.com>
 *        License:  GPLv3
 */

#ifndef FINFO_H
#define FINFO_H

class __Finfo__ {
public:
    __Finfo__(const ObjId& oid, const Finfo* f);

    template <typename T>
    py::object getLookupValueFinfoItemInner(const ObjId& oid,
                                            const string& fname, const T& key,
                                            const string& tgtType)
    {
        if (tgtType == "bool")
            return py::cast(LookupField<T, bool>::get(oid, fname, key));
        if (tgtType == "double")
            return py::cast(LookupField<T, double>::get(oid, fname, key));
        else if (tgtType == "vector<Id>")
            return py::cast(LookupField<T, vector<Id>>::get(oid, fname, key));
        else if (tgtType == "vector<ObjId>")
            return py::cast(
                LookupField<T, vector<ObjId>>::get(oid, fname, key));
        return py::none();
    }

    // Exposed to python as __setitem__
    bool setItem(const py::object& key, const py::object& val);

    // Exposed to python as __getitem__
    py::object getItem(const py::object& key);

    py::object getLookupValueFinfoItem(const ObjId& oid, const py::object& key, const Finfo* f);

    static py::object getLookupValueFinfo(const ObjId& oid, const Finfo* f);

    static bool setLookupValueFinfoItem(const ObjId& oid, const py::object& key,
                                        const py::object& val,
                                        const Finfo* finfo);

    py::object operator()(const py::object& key);

public:
    ObjId oid_;
    const Finfo* f_;
    std::function<py::object(const py::object& key)> func_;
};

#endif /* end of include guard: FINFO_H */
