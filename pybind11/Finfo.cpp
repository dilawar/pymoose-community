// =====================================================================================
//
//       Filename:  Finfo.cpp
//
//    Description:  
//
//         Author:  Dilawar Singh (), dilawar.s.rajput@gmail.com
//   Organization:  NCBS Bangalore
//
// =====================================================================================

#include "../basecode/header.h"
#include "helper.h"
#include "Finfo.h"

__Finfo__::__Finfo__(const ObjId& oid, const string& fname, const Finfo* f,
        const string& ftype)
    : oid_(oid), fname_(fname), f_(f), finfoType_(ftype)
{
    func_ = [oid, fname, f](const py::object& key) {
        return getLookupValueFinfoItem(oid, fname, key, f);
    };
}

void __Finfo__::setItem(const py::object& key, const py::object& val)
{
    setLookupValueFinfoItem(key, val, f_, fname_);
}


py::object __Finfo__::getLookupValueFinfoItem(const ObjId& oid, const string& fname,
        const py::object& key, const Finfo* f)
{
    auto rttType = f->rttiType();
    vector<string> srcDestType;
    moose::tokenize(rttType, ",", srcDestType);
    string srcType = srcDestType[0];
    string tgtType = srcDestType[1];

    py::object r;

    // cout << " LookupValue at fname " << fname << " with src: " << srcType
    // << " and tgtType: " << tgtType << endl;

    if (srcType == "string") {
        auto k = py::cast<string>(key);
        r = getLookupValueFinfoItemInner<string>(oid, fname, k, tgtType);
    }
    else if (srcType == "unsigned int") {
        auto k = py::cast<unsigned int>(key);
        r = getLookupValueFinfoItemInner<unsigned int>(oid, fname, k,
                tgtType);
    }

    if (r.is(py::none())) {
        py::print("getLookupValueFinfoItem::NotImplemented for key:", key,
                "srcType:", srcType, "and tgtType:", tgtType,
                "path: ", oid.path());
        throw runtime_error(
                "getLookupValueFinfoItem::NotImplemented error");
    }
    return r;
}

py::object __Finfo__::getLookupValueFinfo(const ObjId& oid, const string& fname,
        const Finfo* f)
{
    // std::function<py::object(const string&)> f = [oid, fname, rttType](
    //    const string& key) {
    //    return getLookupValueFinfoItem(oid, fname, key, rttType);
    //};
    return py::cast(__Finfo__(oid, fname, f, "LookupValueFinfo"));
}

py::object __Finfo__::operator()(const py::object& key) { return func_(key); }

