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
    __Finfo__(const ObjId& oid, const string& fname, const Finfo* f, const string& ftype);

    void setItem(const py::object& key, const py::object& val);

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

    py::object getLookupValueFinfoItem(const ObjId& oid, const string& fname,
                                       const py::object& key, const Finfo* f);

    py::object getLookupValueFinfo(const ObjId& oid, const string& fname,
                                   const Finfo* f);

    py::object operator()(const py::object& key);

public:
    ObjId oid_;
    string fname_;
    const Finfo* f_;
    const string finfoType_;
    std::function<py::object(const py::object& key)> func_;
};

#endif /* end of include guard: FINFO_H */
