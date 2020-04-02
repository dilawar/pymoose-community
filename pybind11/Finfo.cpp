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
#include "../external/pybind11/include/pybind11/functional.h"

namespace py = pybind11;

#include "../basecode/header.h"
#include "../utility/print_function.hpp"
#include "../utility/strutil.h"
#include "../builtins/Variable.h"
#include "pymoose.h"
#include "helper.h"
#include "Finfo.h"

__Finfo__::__Finfo__(const ObjId& oid, const Finfo* f, const string& finfoType)
    : oid_(oid), f_(f), finfoType_(finfoType)
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
                oid, fieldName, py::cast<unsigned int>(key),
                py::cast<double>(val));
    }

    py::print("NotImplemented::setLookupValueFinfoItem:", key, "to value", val,
              "for object", oid.path(), "and fieldName=", fieldName,
              "rttiType=", rttType, srcDestType);
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

// py::object __Finfo__::getLookupValueFinfo(const ObjId& oid, const Finfo* f)
//{
//    return py::cast(__Finfo__(oid, f));
//}

py::object __Finfo__::getItem(const py::object& key)
{
    return func_(key);
}

py::object __Finfo__::operator()(const py::object& key)
{
    return func_(key);
}

py::cpp_function __Finfo__::getDestFinfoSetterFunc(const ObjId& oid,
                                                   const Finfo* finfo)
{
    const auto rttType = finfo->rttiType();
    vector<string> types;
    moose::tokenize(rttType, ",", types);

    if (types.size() == 1)
        return getDestFinfoSetterFunc1(oid, finfo, types[0]);

    assert(types.size() == 2);
    return getDestFinfoSetterFunc2(oid, finfo, types[0], types[1]);
}

// Get DestFinfo2
py::cpp_function __Finfo__::getDestFinfoSetterFunc2(const ObjId& oid,
                                                    const Finfo* finfo,
                                                    const string& ftype1,
                                                    const string& ftype2)
{
    const auto fname = finfo->name();
    if (ftype1 == "double") {
        if (ftype2 == "unsigned int") {
            std::function<bool(double, unsigned int)> func = [oid, fname](
                const double a, const long b) {
                return SetGet2<double, unsigned int>::set(oid, fname, a, b);
            };
            return func;
        }
    }
    throw runtime_error("getFieldPropertyDestFinfo2::NotImplemented " + fname +
                        " for rttType " + finfo->rttiType() + " for oid " +
                        oid.path());
}

// Get DestFinfo1.
py::cpp_function __Finfo__::getDestFinfoSetterFunc1(const ObjId& oid,
                                                    const Finfo* finfo,
                                                    const string& ftype)
{
    const auto fname = finfo->name();
    if (ftype == "void") {
        std::function<bool()> func = [oid, fname]() {
            return SetGet0::set(oid, fname);
        };
        return func;
    }
    if (ftype == "vector<Id>") {
        std::function<bool(const vector<Id>&)> func = [oid, fname](
            const vector<Id>& ids) {
            return SetGet1<vector<Id>>::set(oid, fname, ids);
        };
        return func;
    }
    if (ftype == "vector<ObjId>") {
        std::function<bool(const vector<ObjId>&)> func = [oid, fname](
            const vector<ObjId>& ids) {
            return SetGet1<vector<ObjId>>::set(oid, fname, ids);
        };
        return func;
    }
    if (ftype == "vector<double>") {
        std::function<bool(const vector<double>&)> func = [oid, fname](
            const vector<double>& data) {
            return SetGet1<vector<double>>::set(oid, fname, data);
        };
        return func;
    }

    throw runtime_error("getFieldPropertyDestFinfo1::NotImplemented " + fname +
                        " for rttType " + ftype + " for oid " + oid.path());
}

py::object __Finfo__::getFieldValue(const ObjId& oid, const Finfo* f)
{
    auto rttType = f->rttiType();
    auto fname = f->name();
    py::object r = py::none();

    if (rttType == "double" or rttType == "float")
        r = pybind11::float_(getField<double>(oid, fname));
    else if (rttType == "vector<double>") {
        // r = py::cast(getField<vector<double>>(oid, fname));
        r = getFieldNumpy<double>(oid, fname);
    } else if (rttType == "vector<unsigned int>") {
        // r = pybind11::cast(getField<vector<unsigned int>>(oid, fname));
        r = getFieldNumpy<unsigned int>(oid, fname);
    } else if (rttType == "string")
        r = pybind11::str(getField<string>(oid, fname));
    else if (rttType == "char")
        r = pybind11::int_(getField<char>(oid, fname));
    else if (rttType == "int")
        r = pybind11::int_(getField<int>(oid, fname));
    else if (rttType == "unsigned int")
        r = pybind11::int_(getField<unsigned int>(oid, fname));
    else if (rttType == "unsigned long")
        r = pybind11::int_(getField<unsigned long>(oid, fname));
    else if (rttType == "bool")
        r = pybind11::bool_(getField<bool>(oid, fname));
    else if (rttType == "Id")
        r = py::cast(getField<Id>(oid, fname));
    else if (rttType == "ObjId")
        r = py::cast(getField<ObjId>(oid, fname));
    else if (rttType == "Variable")
        r = py::cast(getField<Variable>(oid, fname));
    else if (rttType == "vector<Id>")
        r = py::cast(getField<vector<Id>>(oid, fname));
    else if (rttType == "vector<ObjId>")
        r = py::cast(getField<vector<ObjId>>(oid, fname));
    else {
        MOOSE_WARN("Warning: getValueFinfo:: Unsupported type '" + rttType +
                   "'");
        r = py::none();
    }
    return r;
}

py::list __Finfo__::getElementFinfo(const ObjId& objid, const Finfo* f)
{
    auto rttType = f->rttiType();
    auto fname = f->name();
    auto oid = ObjId(objid.path() + '/' + fname);
    auto len = Field<unsigned int>::get(oid, "numField");
    vector<ObjId> res(len);
    for (unsigned int i = 0; i < len; i++)
        res[i] = ObjId(oid.path(), oid.dataIndex, i);
    return py::cast(res);
}

