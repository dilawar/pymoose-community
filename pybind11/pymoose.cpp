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

#include <map>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

#include "../external/pybind11/include/pybind11/functional.h"
#include "../external/pybind11/include/pybind11/numpy.h"
#include "../external/pybind11/include/pybind11/pybind11.h"
#include "../external/pybind11/include/pybind11/stl.h"

// See
// https://pybind11.readthedocs.io/en/stable/advanced/cast/stl.html#binding-stl-containers
// #include "../external/pybind11/include/pybind11/stl_bind.h"

#include "../basecode/global.h"
#include "../basecode/header.h"
#include "../basecode/Cinfo.h"
#include "../builtins/Variable.h"
#include "../shell/Neutral.h"
#include "../shell/Shell.h"
#include "../shell/Wildcard.h"
#include "../utility/strutil.h"
#include "helper.h"
#include "pymoose.h"

using namespace std;
namespace py = pybind11;

Id initModule(py::module& m)
{
    return initShell();
}

template <typename T = double>
void setProperty(const ObjId& id, const string& fname, T val)
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

py::object getValueFinfo(const ObjId& oid, const string& fname,
                         const string& rttType)
{
    py::object r = py::none();
    if (rttType == "double")
        r = pybind11::float_(getProp<double>(oid, fname));
    else if (rttType == "float")
        r = pybind11::float_(getProp<double>(oid, fname));
    else if (rttType == "vector<double>")
        r = py::cast(getProp<vector<double>>(oid, fname));
    else if (rttType == "string")
        r = pybind11::str(getProp<string>(oid, fname));
    else if (rttType == "char")
        r = pybind11::str(getProp<string>(oid, fname));
    else if (rttType == "int")
        r = pybind11::int_(getProp<int>(oid, fname));
    else if (rttType == "unsigned long")
        r = pybind11::int_(getProp<unsigned long>(oid, fname));
    else if (rttType == "unsigned int")
        r = pybind11::int_(getProp<unsigned int>(oid, fname));
    else if (rttType == "bool")
        r = pybind11::bool_(getProp<bool>(oid, fname));
    else if (rttType == "Id")
        r = py::cast(getProp<Id>(oid, fname));
    else if (rttType == "ObjId")
        r = py::cast(getProp<ObjId>(oid, fname));
    else if (rttType == "Variable")
        r = py::cast(getProp<Variable>(oid, fname));
    else if (rttType == "vector<Id>")
        r = py::cast(getProp<vector<Id>>(oid, fname));
    else if (rttType == "vector<ObjId>")
        r = py::cast(getProp<vector<ObjId>>(oid, fname));
    else {
        py::print("Warning: pymoose::getProperty::Warning: Unsupported type " +
                  rttType);
        r = py::none();
    }
    return r;
}

inline ObjId getElementFinfoItem(const ObjId& oid, const size_t& i)
{
    return ObjId(oid.path(), oid.dataIndex, i);
}

py::list getElementFinfo(const ObjId& objid, const string& fname,
                         const string& rttType)
{
    auto oid = ObjId(objid.path() + '/' + fname);
    auto len = Field<unsigned int>::get(oid, "numField");
    vector<ObjId> res(len);
    for (size_t i = 0; i < len; i++)
        res[i] = ObjId(oid.path(), oid.dataIndex, i);
    return py::cast(res);
}

py::object getLookupValueFinfoItem(const ObjId& oid, const string& fname,
                                   const string& k, const string& rttType)
{
    vector<string> srcDestType;
    moose::tokenize(rttType, ",", srcDestType);
    string srcType = srcDestType[0];
    string tgtType = srcDestType[1];

    py::object r;
    if (tgtType == "bool")
        r = py::cast(LookupField<string, bool>::get(oid, fname, k));
    else if (tgtType == "vector<Id>")
        r = py::cast(LookupField<string, vector<Id>>::get(oid, fname, k));
    else
        cerr << "Unsupported types: " << rttType << endl;
    return r;
}

py::function getLookupValueFinfo(const ObjId& oid, const string& fname,
                                 const string& rttType)
{
    std::function<py::object(const string&)> f = [oid, fname, rttType](
        const string& key) {
        return getLookupValueFinfoItem(oid, fname, key, rttType);
    };
    return py::cast(f);
}

py::object getProperty(const ObjId& oid, const string& fname)
{
    auto cinfo = oid.element()->cinfo();
    auto finfo = cinfo->findFinfo(fname);

    if (!finfo) {
        py::print("Field " + fname + " is not found on " + oid.path());
        return pybind11::none();
    }

    string rttType = finfo->rttiType();
    string finfoType = cinfo->getFinfoType(finfo);

    if (finfoType == "ValueFinfo")
        // return value.
        return getValueFinfo(oid, fname, rttType);
    else if (finfoType == "FieldElementFinfo") {
        // Return list.
        return getElementFinfo(oid, fname, rttType);
    } else if (finfoType == "LookupValueFinfo") {
        // Return function.
        return getLookupValueFinfo(oid, fname, rttType);
    }

    cout << "Searching for " << fname << " with rttType " << rttType
         << " and type: " << finfoType << endl;

    py::print("Warning: pymoose::getProperty::Warning: Unsupported type " +
              rttType);
    return pybind11::none();
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
        .def_property_readonly("name",
                               [](const Id& id) { return ObjId(id).name(); })
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
        .def_property_readonly(
             "parent", [](const ObjId& oid) { return Neutral::parent(oid); })
        .def_property_readonly("name", &ObjId::name)
        .def_property_readonly("className", [](const ObjId& oid) {
             return oid.element()->cinfo()->name();
         })
        .def_property_readonly("id", [](ObjId& oid) { return oid.id; })
        .def_property_readonly(
             "type", [](ObjId& oid) { return oid.element()->cinfo()->name(); })
        //--------------------------------------------------------------------
        // Set/Get
        //--------------------------------------------------------------------
        // Overload of Field::set
        .def("setField", &setProperty<double>)
        .def("setField", &setProperty<double>)
        .def("setField", &setProperty<vector<double>>)
        .def("setField", &setProperty<std::string>)
        .def("setField", &setProperty<bool>)

        // Overload for Field::get
        .def("getField", &getProperty)
        .def("getElementField", &getElementField)
        .def("getElementFieldItem", &getElementFieldItem)
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

    py::class_<Variable>(m, "_Variable").def(py::init<>());

    py::class_<Cinfo>(m, "_Cinfo")
        .def(py::init<>())
        .def_property_readonly("name", &Cinfo::name)
        .def_property_readonly("finfoMap", &Cinfo::finfoMap,
                               py::return_value_policy::reference)
        .def("findFinfo", &Cinfo::findFinfoWrapper)
        .def("baseCinfo", &Cinfo::baseCinfo, py::return_value_policy::reference)
        .def("isA", &Cinfo::isA);

    py::class_<Shell>(m, "_Shell")
        .def(py::init<>())
        .def("create", &Shell::doCreate2)
        .def("addMsg", &Shell::doAddMsg)
        .def("getCwe", &Shell::getCwe)
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
    m.def("delete", &mooseDelete);
    m.def("create", &mooseCreate);
    m.def("reinit", &mooseReinit);
    m.def("start", &mooseStart, py::arg("runtime"), py::arg("notify") = false);
    m.def("element", &mooseElement);
    m.def("exists", &doesExist);
    m.def("getCwe", &mooseGetCwe);
    m.def("setClock", &mooseSetClock);
    m.def("loadModelInternal", &loadModelInternal);
    m.def("getFieldDict", &mooseGetFieldDict, py::arg("className"),
          py::arg("finfoType") = "");
    m.def("copy", &mooseCopy, py::arg("orig"), py::arg("newParent"),
          py::arg("newName"), py::arg("num") = 1, py::arg("toGlobal") = false,
          py::arg("copyExtMsgs") = false);
    // Attributes.
    m.attr("NA") = NA;
    m.attr("PI") = PI;
    m.attr("FaradayConst") = FaradayConst;
    m.attr("GasConst") = GasConst;
    m.attr("__version__") = MOOSE_VERSION;
}
