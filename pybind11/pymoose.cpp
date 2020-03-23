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
#include <typeinfo>
#include <utility>

#include "../external/pybind11/include/pybind11/pybind11.h"
#include "../external/pybind11/include/pybind11/stl.h"

#include "../basecode/header.h"
#include "../shell/Shell.h"
#include "../shell/Wildcard.h"

#include "helper.h"
#include "pymoose.h"

using namespace std;
namespace py = pybind11;

void initModule(py::module& m)
{
    initShell();
    auto pShell = getShellPtr();
}

PYBIND11_MODULE(_cmoose, m)
{
    initModule(m);

    m.doc() = R"moosedoc(moose module.
    )moosedoc";

    py::class_<Id>(m, "_Id").def(py::init<>());

    py::class_<ObjId>(m, "_ObjId")
        .def(py::init<>())
        .def_property_readonly("path", &ObjId::path)
        ;

    m.def("create", &createIdFromPath);
    m.def("_wildcardFind", &wildcardFindPybind);

    m.attr("__version__") = MOOSE_VERSION;

}
