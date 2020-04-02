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

#include "../external/pybind11/include/pybind11/pybind11.h"
#include "../external/pybind11/include/pybind11/stl.h"
#include "../external/pybind11/include/pybind11/numpy.h"
// #include "../external/pybind11/include/pybind11/functional.h"
namespace py = pybind11;

#include "../basecode/header.h"
#include "../utility/strutil.h"
#include "helper.h"
#include "Finfo.h"

__Finfo__::__Finfo__(const ObjId& oid, const Finfo* f) : oid_(oid), f_(f)
{
    func_ = [oid, f, this](const py::object& key) {
        return __Finfo__::getLookupValueFinfoItem(oid, key, f);
    };
}

// Exposed to python as __setitem__ on Finfo
bool __Finfo__::setItem(const py::object& key, const py::object& val)
{
    return __Finfo__::setLookupValueFinfoItem(oid_, key, val, f_);
}

bool __Finfo__::setLookupValueFinfoItem(const ObjId& oid, const py::object& key,
                                        const py::object& val,
                                        const Finfo* finfo)
{
    auto rttType = finfo->rttiType();
    auto fieldName = finfo->name();

    vector<string> srcDestType;
    moose::tokenize(rttType, ",", srcDestType);
    assert(srcDestType.size() == 2);

    auto srcType = srcDestType[0];
    auto destType = srcDestType[1];

    if (srcType == "unsigned int") {
        if (destType == "double")
            return LookupField<unsigned int, double>::set(
                oid, fieldName, py::cast<unsigned int>(key), py::cast<double>(val));
    }

    py::print("NotImplemented::setLookupValueFinfoItem:", key, "to value", val,
              "for object", oid.path(), "and fieldName=", fieldName, "rttiType=",
              rttType, srcDestType);
    throw runtime_error("NotImplemented");
    return true;
}

py::object __Finfo__::getLookupValueFinfoItem(const ObjId& oid,
                                              const py::object& key,
                                              const Finfo* f)
{
    auto rttType = f->rttiType();
    auto fname = f->name();
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
    } else if (srcType == "unsigned int") {
        auto k = py::cast<unsigned int>(key);
        r = getLookupValueFinfoItemInner<unsigned int>(oid, fname, k, tgtType);
    }

    if (r.is(py::none())) {
        py::print("getLookupValueFinfoItem::NotImplemented for key:", key,
                  "srcType:", srcType, "and tgtType:", tgtType, "path: ",
                  oid.path());
        throw runtime_error("getLookupValueFinfoItem::NotImplemented error");
    }
    return r;
}

py::object __Finfo__::getLookupValueFinfo(const ObjId& oid, const Finfo* f)
{
    // std::function<py::object(const string&)> f = [oid, fname, rttType](
    //    const string& key) {
    //    return getLookupValueFinfoItem(oid, fname, key, rttType);
    //};
    return py::cast(__Finfo__(oid, f));
}

py::object __Finfo__::getItem(const py::object& key)
{
    return func_(key);
}

py::object __Finfo__::operator()(const py::object& key)
{
    return func_(key);
}

