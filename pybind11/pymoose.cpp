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

#include "../basecode/header.h"
#include "../shell/Shell.h"

#include "helper.h"
#include "pymoose.h"

using namespace std;
namespace py = pybind11;

PYBIND11_MODULE(_moose, m)
{
    initShell();

    m.doc() = R"moosedoc(moose module.
    )moosedoc";

   // py::class_<ObjId>(m, "ObjId")
   //     .def(py::init<>())
   //     ;

   //py::class_<Id>(m, "Id")
   //    .def(py::init<>())
   //    ;

    m.def("create", &createIdFromPath);

    m.attr("__version__") = MOOSE_VERSION;

}
