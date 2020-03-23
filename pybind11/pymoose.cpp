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

#include "../basecode/header.h"
#include "../basecode/Cinfo.h"

#include "../shell/Shell.h"
#include "../shell/Wildcard.h"
#include "../shell/Neutral.h"

#include "helper.h"
#include "pymoose.h"

using namespace std;
namespace py = pybind11;

void initModule(py::module& m)
{
    initShell();
}

template <typename T = double>
void setProperty(Id id, const string& fname, T val)
{
    Field<T>::set(id, fname, val);
}

template <typename T = double>
T getProperty(Id id, const string& fname)
{
    return Field<T>::get(id, fname);
}

PYBIND11_MODULE(_cmoose, m)
{
    initModule(m);

    m.doc() = R"moosedoc(moose module.
    )moosedoc";

    py::class_<Id>(m, "_Id").def(py::init<>()).def_property_readonly(
        "value", &Id::value);

    py::class_<ObjId>(m, "_ObjId")
        .def(py::init<>())
        .def_property_readonly("path", &ObjId::path)
        .def_property_readonly("name", &ObjId::name);

    py::class_<FinfoWrapper>(m, "_FinfoWrapper")
        .def(py::init<const Finfo*>())
        .def_property_readonly("name", &FinfoWrapper::getName)
        .def_property_readonly("doc", &FinfoWrapper::docs)
        .def_property_readonly("type", &FinfoWrapper::type)
        .def_property_readonly("src", &FinfoWrapper::src)
        .def_property_readonly("dest", &FinfoWrapper::dest);

    py::class_<Cinfo>(m, "_Cinfo")
        .def(py::init<>())
        .def_property_readonly("finfoMap", &Cinfo::finfoMap,
                               py::return_value_policy::reference)
        .def_property_readonly("finfoNames", &Cinfo::getFinfoNames)
        .def("findFinfo", &Cinfo::findFinfoWrapper);

    m.def("create", &createIdFromPath);
    m.def("setCwe", [](Id id) { getShellPtr()->setCwe(id); });
    m.def("getCwe", []() { return getShellPtr()->getCwe(); });

    m.def("getCinfo", [](const string& name) { return Cinfo::find(name); },
          py::return_value_policy::reference);
    m.def("_wildcardFind", &wildcardFindPybind);

    m.def("getProperty", &getProperty<double>);
    m.def("getProperty", &getProperty<vector<double>>);
    m.def("getProperty", &getProperty<string>);
    m.def("getProperty", &getProperty<Id>);
    m.def("getProperty", &getProperty<unsigned int>);
    m.def("getProperty", &getProperty<bool>);

    m.def("setProperty", &setProperty<double>);
    m.def("setProperty", &setProperty<vector<double>>);
    m.def("setProperty", &setProperty<string>);
    m.def("setProperty", &setProperty<Id>);
    m.def("setProperty", &setProperty<unsigned int>);
    m.def("setProperty", &setProperty<bool>);

    m.attr("__version__") = MOOSE_VERSION;
}
