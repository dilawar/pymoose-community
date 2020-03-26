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

// See https://pybind11.readthedocs.io/en/stable/advanced/cast/stl.html#binding-stl-containers
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

template <typename T>
void setProp(const ObjId& id, const string& fname, T val)
{
    Field<T>::set(id, fname, val);
}

template <typename T>
T getProp(ObjId id, const string& fname)
{
    // cerr << "Getting " << fname << " for " << id.path() << endl;
    return Field<T>::get(id, fname);
}

template <typename T>
void setPropVec(const ObjId& id, const string& fname, const vector<T>& val)
{
    Field<T>::setVec(id, fname, val);
}

template <typename T>
vector<T> getPropVec(const ObjId& id, const string& fname)
{
    vector<T> val;
    Field<T>::getVec(id, fname, val);
    return val;
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
        .def_property_readonly("value",
                               [](const ObjId oid) { return oid.id.value(); })
        .def_property_readonly("path", &ObjId::path)
        .def_property_readonly("name", &ObjId::name)
        .def_property_readonly("name", &ObjId::name)
        .def_property_readonly("id", [](ObjId& oid) { return oid.id; })
        .def_property_readonly(
             "type", [](ObjId& oid) { return oid.element()->cinfo()->name(); })
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
        .def_property_readonly("finfoNames", &Cinfo::getFinfoNames)
        .def("findFinfo", &Cinfo::findFinfoWrapper)
        .def("baseCinfo", &Cinfo::baseCinfo,
             py::return_value_policy::reference);

    py::class_<Shell>(m, "_Shell")
        .def(py::init<>())
        .def("create", &Shell::create)
        .def("addMsg", &Shell::doAddMsg)
        .def("setClock", &Shell::doSetClock)
        .def("reinit", &Shell::doReinit)
        .def("start", &Shell::doStart, py::arg("runtime"),
             py::arg("notify") = false)
        .def("quit", &Shell::doQuit);

    m.def("nextId", Id::nextId);
    m.def("create", &createIdFromPath);
    m.def("exists", &doesExist);
    m.def("element", &element);

    m.def("move", [](ObjId o, ObjId oid) { getShellPtr()->doMove(o, oid); });
    m.def("copy", [](ObjId o, ObjId newP, string newName = "", size_t n = 1,
                     bool toGlobal = false, bool copyExtMsg = false) {
        if (newName.empty())
            newName = o.element()->getName();
        getShellPtr()->doCopy(o, newP, newName, n, toGlobal, copyExtMsg);
    });

    m.def("setCwe", [](const ObjId& id) { getShellPtr()->setCwe(id); });
    m.def("getCwe", []() { return getShellPtr()->getCwe(); });
    m.def("delete", [](ObjId oid) { return getShellPtr()->doDelete(oid); });
    m.def("reinit", []() { return getShellPtr()->doReinit(); });
    m.def("stop", []() { return getShellPtr()->doStop(); });

    m.def("seed", [](int seed) { return moose::setGlobalSeed(seed); });

    m.def("start", [](double runtime, bool notify = false) {
        getShellPtr()->doStart(runtime, notify);
    });

    m.def("getCinfo", [](const string& name) { return Cinfo::find(name); },
          py::return_value_policy::reference);

    m.def("_wildcardFind", &wildcardFindPybind);

    m.def("loadModelInternal", &loadModelInternal);

    // Overload for Field::get
    m.def("get", &getProp<double>);
    m.def("get", &getProp<vector<double>>);
    m.def("get", &getProp<string>);
    m.def("get", &getProp<unsigned int>);
    m.def("get", &getProp<bool>);

    // Overload of Field::set
    m.def("set", &setProp<double>);
    m.def("set", &setProp<vector<double>>);
    m.def("set", &setProp<string>);
    m.def("set", &setProp<unsigned int>);
    m.def("set", &setProp<bool>);

    // m.def("setVec", &setPropVec<double>);
    // m.def("getVec", &getPropVec<double>);

    m.def("getShell",
          []() { return reinterpret_cast<Shell*>(Id().eref().data()); },
          py::return_value_policy::reference);

    // Attributes.
    m.attr("NA") = NA;
    m.attr("PI") = PI;
    m.attr("FaradayConst") = FaradayConst;
    m.attr("GasConst") = GasConst;

    m.attr("__version__") = MOOSE_VERSION;
}
