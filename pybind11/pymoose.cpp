// =====================================================================================
//
//       Filename:  pymoose.cpp
//
//    Description:  Python bindings generated using PyBind11.
//
//        Version:  1.0
//        Created:  03/15/2020 04:03:58 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Dilawar Singh (), dilawar.s.rajput@gmail.com
//   Organization:  NCBS Bangalore
//
// =====================================================================================

#include <vector>
#include <utility>
#include <typeinfo>
#include <typeindex>
#include <map>

#include "../external/pybind11/include/pybind11/pybind11.h"
#include "../external/pybind11/include/pybind11/stl.h"
#include "../external/pybind11/include/pybind11/numpy.h"

// See
// https://pybind11.readthedocs.io/en/stable/advanced/cast/stl.html#binding-stl-containers
// #include "../external/pybind11/include/pybind11/stl_bind.h"

#include "../basecode/header.h"
#include "../basecode/global.h"
#include "../basecode/Cinfo.h"

#include "../shell/Shell.h"
#include "../shell/Wildcard.h"
#include "../shell/Neutral.h"

#include "helper.h"
#include "pymoose.h"

using namespace std;
namespace py = pybind11;

Id initModule(py::module& m)
{
    return initShell();
}

template <typename T = double>
void setProp(const ObjId& id, const string& fname, T val)
{
    Field<T>::set(id, fname, val);
}

template <typename T = double>
T getProp(const ObjId& id, const string& fname)
{
    return Field<T>::get(id, fname);
}

// FIXME: Is it most efficient?
// See discussion here: https://github.com/pybind/pybind11/issues/1042
template <typename T = double>
py::array_t<T> getFieldNumpy(const ObjId& id, const string& fname)
{
    auto v = Field<vector<T>>::get(id, fname);
    return py::array_t<T>(v.size(), v.data());
}

py::object getFieldElement(const ObjId& oid, const string& fname)
{
    auto cinfo = oid.element()->cinfo();
    auto finfo = cinfo->findFinfo(fname);
    if (! finfo) {
        py::object avl = py::cast(cinfo->getFinfoNameAndType());
        py::print("Field " + fname + " is not found on '" + oid.path()
                + "'. Available: ",  avl);
        return pybind11::none();
    }
}

py::object getFieldGeneric(const ObjId& oid, const string& fname)
{
    auto cinfo = oid.element()->cinfo();
    auto finfo = cinfo->findFinfo(fname);
    if (!finfo) {
        py::print("Field " + fname + " is not found on " + oid.path());
        return pybind11::none();
    }

    string ftype = finfo->rttiType();
    if (ftype == "double")
        return pybind11::float_(getProp<double>(oid, fname));
    else if (ftype == "float")
        return pybind11::float_(getProp<double>(oid, fname));
    else if (ftype == "vector<double>")
        return py::cast(getProp<vector<double>>(oid, fname));
    else if (ftype == "string")
        return pybind11::str(getProp<string>(oid, fname));
    else if (ftype == "char")
        return pybind11::str(getProp<string>(oid, fname));
    else if (ftype == "int")
        return pybind11::int_(getProp<int>(oid, fname));
    else if (ftype == "unsigned long")
        return pybind11::int_(getProp<unsigned long>(oid, fname));
    else if (ftype == "unsigned int")
        return pybind11::int_(getProp<unsigned int>(oid, fname));
    else if (ftype == "bool")
        return pybind11::bool_(getProp<bool>(oid, fname));
    else if (ftype == "Id")
        return py::cast(getProp<Id>(oid, fname));
    else if (ftype == "ObjId")
        return py::cast(getProp<ObjId>(oid, fname));
    else if (ftype == typeid(Variable).name())
        return py::cast(getProp<Variable>(oid, fname));
    py::print("pymoose::getFieldGeneric::Warning: Unsupported type " + ftype);
    return pybind11::none();
}

ObjId connect(const ObjId& src, const string& srcField, const ObjId& tgt,
              const string& tgtField)
{
    auto pShell = getShellPtr();
    return pShell->doAddMsg("Single", src, srcField, tgt, tgtField);
}

PYBIND11_MODULE(_cmoose, m)
{
    m.doc() = R"moosedoc(moose module.)moosedoc";

    initModule(m);

    py::class_<Id>(m, "_Id")
        .def(py::init<>())
        .def(py::init<unsigned int>())
        .def(py::init<const string&, const string&>())
        .def(py::init<const ObjId&>())
        // properties
        .def_property_readonly("numIds", &Id::numIds)
        .def_property_readonly("path", &Id::path)
        .def_property_readonly("id", &Id::value)
        .def_property_readonly("cinfo",
                               [](Id& id) { return id.element()->cinfo(); },
                               py::return_value_policy::reference)
        .def_property_readonly(
             "type", [](Id& id) { return id.element()->cinfo()->name(); })
        .def("__repr__", [](const Id& id) {
             return "<Id id=" + std::to_string(id.value()) + " path=" +
                    id.path() + " class=" + id.element()->cinfo()->name() + ">";
         });

    py::class_<ObjId>(m, "_ObjId")
        .def(py::init<>())
        .def(py::init<Id>())
        .def(py::init<Id, unsigned int>())
        .def(py::init<Id, unsigned int, unsigned int>())
        .def(py::init<const string&>())
        //---------------------------------------------------------------------
        //  Readonly properties.
        //---------------------------------------------------------------------
        .def_property_readonly("value",
                               [](const ObjId oid) { return oid.id.value(); })
        .def_property_readonly("path", &ObjId::path)
        .def_property_readonly("name", &ObjId::name)
        .def_property_readonly("name", &ObjId::name)
        .def_property_readonly("id", [](ObjId& oid) { return oid.id; })
        .def_property_readonly(
             "type", [](ObjId& oid) { return oid.element()->cinfo()->name(); })
        //--------------------------------------------------------------------
        // Set/Get
        //--------------------------------------------------------------------
        // Overload of Field::set
        .def("setField", &setProp<double>)
        .def("setField", &setProp<double>)
        .def("setField", &setProp<vector<double>>)
        .def("setField", &setProp<std::string>)
        .def("setField", &setProp<bool>)

        // Overload for Field::get
        // NOTE: Get it tricky to get right.
        // See discussion here: https://github.com/pybind/pybind11/issues/1667
        // Required c++14 compiler.
        .def("getField", &getFieldGeneric)
        .def("getFieldElement", &getFieldElement)
        .def("getNumpy", &getFieldNumpy<double>)

        //---------------------------------------------------------------------
        //  Connect
        //---------------------------------------------------------------------
        .def("connect", &connect)

        //---------------------------------------------------------------------
        //  Extra
        //---------------------------------------------------------------------
        .def("__repr__", [](const ObjId& oid) {
             return "<" + oid.element()->cinfo()->name() + " id=" +
                    std::to_string(oid.id.value()) + " path=" + oid.path() +
                    ">";
         });

    py::class_<FinfoWrapper>(m, "_FinfoWrapper")
        .def(py::init<const Finfo*>())
        .def_property_readonly("name", &FinfoWrapper::getName)
        .def_property_readonly("doc", &FinfoWrapper::docs)
        .def_property_readonly("type", &FinfoWrapper::type)
        .def_property_readonly("src", &FinfoWrapper::src,
                               py::return_value_policy::reference)
        .def_property_readonly("dest", &FinfoWrapper::dest,
                               py::return_value_policy::reference);

    py::class_<Cinfo>(m, "_Cinfo")
        .def(py::init<>())
        .def_property_readonly("name", &Cinfo::name)
        .def_property_readonly("finfoMap", &Cinfo::finfoMap,
                               py::return_value_policy::reference)
        .def("findFinfo", &Cinfo::findFinfoWrapper)
        .def("baseCinfo", &Cinfo::baseCinfo,
             py::return_value_policy::reference);

    py::class_<Shell>(m, "_Shell")
        .def(py::init<>())
        .def("create", &Shell::create)
        .def("addMsg", &Shell::doAddMsg)
        .def("setClock", &Shell::doSetClock)
        .def("reinit", &Shell::doReinit)
        .def("delete", &Shell::doDelete)
        .def("start", &Shell::doStart, py::arg("runtime"),
             py::arg("notify") = false)
        .def("quit", &Shell::doQuit);

    // Module functions.
    m.def("getShell",
          []() { return reinterpret_cast<Shell*>(Id().eref().data()); },
          py::return_value_policy::reference);

    m.def("wildcardFind", &wildcardFind2);

    // Attributes.
    m.attr("NA") = NA;
    m.attr("PI") = PI;
    m.attr("FaradayConst") = FaradayConst;
    m.attr("GasConst") = GasConst;

    m.attr("__version__") = MOOSE_VERSION;
}
