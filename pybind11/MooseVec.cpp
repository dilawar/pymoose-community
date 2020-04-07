/***
 *    Description:  vec api.
 *
 *        Created:  2020-04-01

 *         Author:  Dilawar Singh <dilawar.s.rajput@gmail.com>
 *        License:  MIT License
 */

#include "../basecode/header.h"

using namespace std;

#include "../external/pybind11/include/pybind11/pybind11.h"
#include "../external/pybind11/include/pybind11/numpy.h"
namespace py = pybind11;

#include "../utility/strutil.h"
#include "helper.h"
#include "pymoose.h"
#include "MooseVec.h"

MooseVec::MooseVec(const string& path, unsigned int n = 0,
                   const string& dtype = "Neutral")
    : path_(path)
{
    oid_ = ObjId(path);
}

MooseVec::MooseVec(const ObjId& oid) : oid_(oid), path_(oid.path())
{
}

MooseVec::MooseVec(const Id& id) : oid_(ObjId(id)), path_(id.path())
{
}

const string MooseVec::dtype() const
{
    return oid_.element()->cinfo()->name();
}

const size_t MooseVec::size() const
{
    if (oid_.element()->hasFields())
        return Field<unsigned int>::get(oid_, "numField");
    return oid_.element()->numData();
}

const string MooseVec::name() const
{
    return oid_.element()->getName();
}

const string MooseVec::path() const
{
    return path_;
}

unsigned int MooseVec::len()
{
    return (unsigned int)size();
}

ObjId MooseVec::getItem(const size_t i) const
{
    if (oid_.element()->hasFields())
        return getFieldItem(i);
    return getDataItem(i);
}

ObjId MooseVec::getDataItem(const size_t i) const
{
    return ObjId(oid_.path(), i, oid_.fieldIndex);
}

ObjId MooseVec::getFieldItem(const size_t i) const
{
    return ObjId(oid_.path(), oid_.dataIndex, i);
}

void MooseVec::setAttrOneToAll(const string& name, const py::object& val)
{
    for (size_t i = 0; i < size(); i++) setFieldGeneric(getItem(i), name, val);
}

void MooseVec::setAttrOneToOne(const string& name, const py::sequence& val)
{
    if (py::len(val) != size())
        throw runtime_error(
            "Length of sequence on the right hand side "
            "does not match size of vector. "
            "Expected " +
            to_string(size()) + ", got " + to_string(py::len(val)));

    for (size_t i = 0; i < size(); i++)
        setFieldGeneric(getItem(i), name, val[i]);
}

vector<py::object> MooseVec::getAttribute(const string& name)
{
    vector<py::object> res(size());
    for (unsigned int i = 0; i < size(); i++)
        res[i] = getFieldGeneric(getItem(i), name);
    return res;
}

// FIXME: Only double is supported here. Not sure if this is enough. This
// should be the API function.
py::array_t<double> MooseVec::getAttributeNumpy(const string &name)
{
    auto cinfo = oid_.element()->cinfo();
    auto finfo = cinfo->findFinfo(name);

    if (!finfo) {
        throw py::key_error(name + " is not found on '" + oid_.path() + "'.");
    }

    string finfoType = cinfo->getFinfoType(finfo);

    // Either return a simple value (ValueFinfo), list, dict or DestFinfo
    // setter.
    // The DestFinfo setter is a function.

    vector<double> res(size());
    if (finfoType == "ValueFinfo") {
        for (unsigned int i = 0; i < size(); i++)
            res[i] = getField<double>(getItem(i), name);
        return py::array_t<double>(res.size(), res.data());
    }

    throw runtime_error("MooseVec::getAttributeNumpy::NotImplemented : " + name +
                        " with rttType " + finfo->rttiType() + " and type: '" +
                        finfoType + "'");
    return py::array_t<double>();
}


ObjId MooseVec::connectToSingle(const string& srcfield, const ObjId& tgt,
                                const string& tgtfield, const string& msgtype)
{
    return shellConnect(oid_, srcfield, tgt, tgtfield, msgtype);
}

ObjId MooseVec::connectToVec(const string& srcfield, const MooseVec& tgt,
                             const string& tgtfield, const string& msgtype)
{
    if (size() != tgt.size())
        throw runtime_error(
            "Length mismatch. Source vector size is " + to_string(size()) +
            " but the target vector size is " + to_string(tgt.size()));
    return shellConnect(oid_, srcfield, tgt.obj(), tgtfield, msgtype);
}

const ObjId& MooseVec::obj() const
{
    return oid_;
}

vector<ObjId> MooseVec::objs() const
{
    vector<ObjId> items;
    for (size_t i = 0; i < size(); i++)
        items.push_back(ObjId(oid_.path(), i, 0));
    return items;
}

size_t MooseVec::id() const
{
    return oid_.id.value();
}

void MooseVec::generateIterator()
{
    objs_.resize(size());
    for (size_t i = 0; i < size(); i++) objs_[i] = getItem(i);
}

const vector<ObjId>& MooseVec::objref() const
{
    return objs_;
}
