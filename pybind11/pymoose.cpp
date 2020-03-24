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
#include "../basecode/global.h"
#include "../basecode/Cinfo.h"

#include "../shell/Shell.h"
#include "../shell/Wildcard.h"
#include "../shell/Neutral.h"

#include "helper.h"
#include "pymoose.h"

using namespace std;
namespace py = pybind11;

void defineFinfos(py::object& cls, const string& cname)
{
    auto pCinfo = Cinfo::find(cname);
    assert(pCinfo);

    cerr << "\n====================================================== " << endl;
    cerr << "Adding Finfos to class " << cls << " with name " << cname << endl;

    cerr << "ValueFinfo: ";
    for(size_t i = 0; i < pCinfo->getNumValueFinfo(); i++)
    {
        auto pFinfo = pCinfo->getValueFinfo(i);
        cerr << pFinfo->name() << " ";
    }
    cerr << endl;

    cerr << "FieldElementFinfo: ";
    for(size_t i = 0; i < pCinfo->getNumFieldElementFinfo(); i++)
    {
        auto pFinfo = pCinfo->getFieldElementFinfo(i);
        cout << pFinfo->name() << " ";
    }
    cerr << endl;
    cerr << "DestFinfos: ";
    for(size_t i = 0; i < pCinfo->getNumDestFinfo(); i++)
    {
        auto pFinfo = pCinfo->getDestFinfo(i);
        cout << pFinfo->name() << " ";
    }
    cerr << endl;
}

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

    py::class_<Id>(m, "_Id").def(py::init<>())
        .def_property_readonly("value", &Id::value)
        .def_property_readonly("path", [](Id id){return id.path("/"); })
        // Return self.
        .def_property_readonly("id", [](Id& id){return id;})
        .def_property_readonly("type"
                , [](Id& id){ return id.element()->cinfo()->name(); }
                )
        ;

    py::class_<ObjId>(m, "_ObjId")
        .def(py::init<>())
        .def_property_readonly("path", &ObjId::path)
        .def_property_readonly("name", &ObjId::name)
        .def_property_readonly("id", [](ObjId& oid) { return oid.id; })
        .def_property_readonly("type"
                , [](ObjId& oid){ return oid.element()->cinfo()->name(); }
                )
        ;

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

    m.def("exists", &createIdFromPath);

    m.def("element", &element);

    m.def("move", [](Id o, ObjId oid){ getShellPtr()->doMove(o, oid); });
    m.def("copy", [](Id o, ObjId newP, string newName="", size_t n=1, bool toGlobal=false, bool copyExtMsg=false){ 
            if(newName.empty())
                newName = o.element()->getName();
            getShellPtr()->doCopy(o, newP, newName, n, toGlobal, copyExtMsg); 
        });

    m.def("setCwe", [](Id id) { getShellPtr()->setCwe(id); });
    m.def("getCwe", []() { return getShellPtr()->getCwe(); });
    m.def("delete", [](ObjId oid) { return getShellPtr()->doDelete(oid);});
    m.def("reinit", []() { return getShellPtr()->doReinit();});
    m.def("stop", []() { return getShellPtr()->doStop();});

    m.def("seed", [](int seed) { return moose::setGlobalSeed(seed);});

    m.def("start", [](double runtime, bool notify=false) {
            getShellPtr()->doStart(runtime, notify);
            });

    m.def("getCinfo", [](const string& name) { return Cinfo::find(name); },
          py::return_value_policy::reference);

    m.def("_wildcardFind", &wildcardFindPybind);

    m.def("loadModelInternal", &loadModelInternal);


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

    // TODO:
    m.def("__defineFinfos", &defineFinfos);

    m.attr("__version__") = MOOSE_VERSION;
}
