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

#include "helper.h"
#include "pymoose.h"
#include "MooseVec.h"

MooseVec::MooseVec(const string& path, unsigned int n = 1, const string& dtype = "Neutral")
: path_(path)
{
    if (!mooseExists(path)) {
        objs_.clear();
        ObjId o = mooseCreate(dtype, path, n);
        for (unsigned int i = 0; i < n; i++) objs_.push_back(ObjId(o, i));
    } else {
        objs_.clear();
        auto o = ObjId(path);
        for (unsigned int i = 0; i < o.element()->numData(); i++)
            objs_.push_back(ObjId(o, i));
    }
}

MooseVec::MooseVec(const ObjId& oid) : path_(oid.path())
{
    for (unsigned int i = 0; i < oid.element()->numData(); i++)
        objs_.push_back(ObjId(oid, i));
}

const string MooseVec::dtype() const
{
    return objs_[0].element()->cinfo()->name();
}

const size_t MooseVec::size() const
{
    return objs_.size();
}

const string MooseVec::path() const
{
    return path_;
}

unsigned int MooseVec::len()
{
    return (unsigned int)size();
}

const ObjId& MooseVec::getItem(const size_t i) const
{
    return objs_.at(i);
}

void MooseVec::setAttrOneToAll(const string& name, const py::object& val)
{
    for (const auto& o : objs_) setFieldGeneric(o, name, val);
}

void MooseVec::setAttrOneToOne(const string& name, const py::sequence& val)
{
    if ( py::len(val) != objs_.size())
        throw runtime_error(
                "Length of sequence on the right hand side "
                "does not match size of vector. "
                "Expected " +
                to_string(objs_.size()) + ", got " + to_string(py::len(val)));
    for (size_t i = 0; i < objs_.size(); i++)
        setFieldGeneric(objs_[i], name, val[i]);
}

vector<py::object> MooseVec::getAttr(const string& name)
{
    vector<py::object> res(objs_.size());
    for (unsigned int i = 0; i < objs_.size(); i++)
        res[i] = getFieldGeneric(objs_[i], name);
    return res;
}

const vector<ObjId>& MooseVec::objs() const
{
    return objs_;
}
