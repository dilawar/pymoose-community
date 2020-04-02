// =====================================================================================
//
//       Filename:  pymoose.cpp
//
//    Description:  Python bindings generated using PyBind11.
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

#include "../external/pybind11/include/pybind11/pybind11.h"
#include "../external/pybind11/include/pybind11/stl.h"
#include "../external/pybind11/include/pybind11/numpy.h"
// #include "../external/pybind11/include/pybind11/functional.h"
namespace py = pybind11;

// See https://pybind11.readthedocs.io/en/master/classes.html#overloaded-methods
template <typename... Args>
using overload_cast_ = pybind11::detail::overload_cast_impl<Args...>;

#include "../basecode/header.h"
#include "../basecode/global.h"
#include "../basecode/Cinfo.h"
#include "../builtins/Variable.h"
#include "../shell/Neutral.h"
#include "../shell/Shell.h"
#include "../shell/Wildcard.h"
#include "../utility/strutil.h"

#include "../basecode/global.h"

#include "helper.h"
#include "Finfo.h"
#include "MooseVec.h"

#include "pymoose.h"

using namespace std;
using namespace pybind11::literals;

Id initModule(py::module& m)
{
    return initShell();
}

bool setFieldGeneric(const ObjId& oid, const string& fieldName,
                     const py::object& val)
{
    auto cinfo = oid.element()->cinfo();
    auto finfo = cinfo->findFinfo(fieldName);
    if (!finfo) {
        throw runtime_error(fieldName + " is not found on " + oid.path());
        return false;
    }
    auto fieldType = finfo->rttiType();

    if (fieldType == "double")
        return Field<double>::set(oid, fieldName, val.cast<double>());
    if (fieldType == "vector<double>")
        return Field<vector<double>>::set(oid, fieldName, val.cast<vector<double>>());
    if (fieldType == "float")
        return Field<float>::set(oid, fieldName, val.cast<float>());
    if (fieldType == "unsigned int")
        return Field<unsigned int>::set(oid, fieldName,
                                        val.cast<unsigned int>());
    if (fieldType == "unsigned long")
        return Field<unsigned long>::set(oid, fieldName,
                                         val.cast<unsigned long>());
    if (fieldType == "int")
        return Field<int>::set(oid, fieldName, val.cast<int>());
    if (fieldType == "bool")
        return Field<bool>::set(oid, fieldName, val.cast<bool>());
    if (fieldType == "string")
        return Field<string>::set(oid, fieldName, val.cast<string>());
    if (fieldType == "vector<string>")
        return Field<vector<string>>::set(oid, fieldName, val.cast<vector<string>>());
    if (fieldType == "char")
        return Field<char>::set(oid, fieldName, val.cast<char>());
    if (fieldType == "ObjId")
        return Field<ObjId>::set(oid, fieldName, val.cast<ObjId>());
    if (fieldType == "Id") {
        // NB: Note that we cast to ObjId here and not to Id. 
        return Field<Id>::set(oid.id, fieldName, val.cast<ObjId>());
    }
    if (fieldType == "Variable")
        return Field<Variable>::set(oid, fieldName, val.cast<Variable>());

    throw runtime_error("NotImplemented::setField : " + fieldName +
                        " with value type " + fieldType);
    return false;
}

py::object getFieldGeneric(const ObjId& oid, const string& fieldName)
{
    auto cinfo = oid.element()->cinfo();
    auto finfo = cinfo->findFinfo(fieldName);

    if (!finfo) {
        cout << "Error: " << fieldName << " is not found on " << oid.path()
             << endl;
        return pybind11::none();
    }

    string finfoType = cinfo->getFinfoType(finfo);

    if (finfoType == "ValueFinfo")
        // return value.
        return getValueFinfo(oid, finfo);
    else if (finfoType == "FieldElementFinfo") {
        // Return list.
        return getElementFinfo(oid, finfo);
    } else if (finfoType == "LookupValueFinfo") {
        // Return function.
        return __Finfo__::getLookupValueFinfo(oid, finfo);
    } else if (finfoType == "DestFinfo") {
        return getFieldPropertyDestFinfo(oid, finfo);
    }

    throw runtime_error("getFieldGeneric::NotImplemented : " + fieldName + " with rttType "
         + finfo->rttiType() + " and type: '" + finfoType + "'");
    return pybind11::none();
}

// ObjId connectGeneric(const py::object& src, const string srcfield,
//                      const py::object& tgt, const string& tgtfield,
//                      const string& type)
// {
//     return src.connect(srcfield, tgt, tgtfield, type);
// }

PYBIND11_MODULE(_cmoose, m)
{
    m.doc() = R"moosedoc(moose module.)moosedoc";

    initModule(m);

    // This is a wrapper around Field::get  and LookupField::get which may
    // return simple values or vector. Python scripts expect LookupField to
    // return either list of dict which can be queried by key and index. This
    // class bind both __getitem__ to the getter function call.
    // Note that both a.isA["Compartment"] and a.isA("Compartment") are valid
    // now.
    py::class_<__Finfo__>(m, "_Finfo")
        .def(py::init<const ObjId&, const Finfo*>())
        .def("__call__", &__Finfo__::operator())
        .def("__getitem__", &__Finfo__::getItem)
        .def("__setitem__", &__Finfo__::setItem)
        ;

    py::class_<Id>(m, "_Id")
        .def(py::init<>())
        .def(py::init<unsigned int>())
        .def(py::init<const string&>())
        .def(py::init<const ObjId&>())
        // properties
        .def_property_readonly("numIds", &Id::numIds)
        .def_property_readonly("path", &Id::path)
        .def_property_readonly(
             "name", [](const Id& id) { return id.element()->getName(); })
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
        .def_property_readonly(
             "name", [](const ObjId& oid) { return oid.element()->getName(); })
        .def_property_readonly("className", [](const ObjId& oid) {
             return oid.element()->cinfo()->name();
         })
        .def_property_readonly("id", [](ObjId& oid) { return oid.id; })
        .def_property_readonly(
             "type", [](ObjId& oid) { return oid.element()->cinfo()->name(); })

        //--------------------------------------------------------------------
        // Set/Get
        //--------------------------------------------------------------------
        // Overload of Field::set . FIXME: remove after testing.
        // .def("setField", &setField<double>)
        // .def("setField", &setField<double>)
        // .def("setField", &setField<vector<double>>)
        // .def("setField", &setField<std::string>)
        // .def("setField", &setField<bool>)

        // Overload for Field::get
        .def("getField", &getFieldGeneric)
        // .def("getElementField", &getElementField)
        // .def("getElementFieldItem", &getElementFieldItem)
        // .def("getNumpy", &getFieldNumpy<double>)

        /**
        *  Override __eq__ etc.
        */
        .def("__eq__", [](const ObjId& a, const ObjId& b) { return a == b; })
        .def("__ne__", [](const ObjId& a, const ObjId& b) { return a != b; })

        /**
        * Attributes.
        */
        .def("__getattr__", &getFieldGeneric)
        .def("__setattr__", &setFieldGeneric)
        // .def("__setattr__", &setField<bool>)
        // .def("__setattr__", &setField<double>)
        // .def("__setattr__", &setField<size_t>)
        // .def("__setattr__", &setField<unsigned long>)
        // .def("__setattr__", &setField<unsigned int>)
        // .def("__setattr__", &setField<int>)
        // .def("__setattr__", &setField<vector<double>>)
        // .def("__setattr__", &setField<std::string>)
        // .def("__setattr__", &setField<ObjId>)
        // .def("__setattr__", &setField<vector<ObjId>>)
        // .def("__setattr__", &setField<Id>)
        // .def("__setattr__", &setField<vector<Id>>)

        //---------------------------------------------------------------------
        //  Connect
        //---------------------------------------------------------------------
        // This would be so much easier with c++17.
        .def("connect",
             [](const ObjId& src, const string& srcfield, const ObjId& tgt,
                const string& tgtfield, const string& type) {
             return mooseConnect(src, srcfield, tgt, tgtfield, type);
         })
        // // , "src"_a, "srcfield"_a, "dest"_a, "destfield"_a, "msgtype"_a)
        // //, "src"_a, "srcfield"_a, "dest"_a, "destfield"_a, "msgtype"_a =
        // //"OneToAll")
        // .def("connect", &connectVec<MooseVec, MooseVec>)
        // // , "src"_a, //"srcfield"_a, "dest"_a, "destfield"_a, "msgtype"_a =
        // // "OneToOne")
        // .def("connect", &connect<ObjId, ObjId>)
        // // , "src"_a, "srcfield"_a, "dest"_a, "destfield"_a, "msgtype"_a =
        // // "Single")
        // .def("connect", &connect<ObjId, Id>)
        // // , "src"_a, "srcfield"_a, "dest"_a, "destfield"_a, "msgtype"_a =
        // // "Single")
        // .def("connect", &connect<Id, ObjId>)
        // // , "src"_a, "srcfield"_a, "dest"_a, "destfield"_a, "msgtype"_a =
        // // "Single")
        // .def("connect", &connect<Id, Id>)
        // // , "src"_a, "srcfield"_a, "dest"_a, "destfield"_a, "msgtype"_a =
        // // "Single")

        //---------------------------------------------------------------------
        //  Extra
        //---------------------------------------------------------------------
        .def("__repr__", [](const ObjId& oid) {
             return "<moose." + oid.element()->cinfo()->name() + " id=" +
                    std::to_string(oid.id.value()) + " dataIndex=" +
                    to_string(oid.eref().dataIndex()) + " path=" + oid.path() +
                    ">";
         });

    // Variable.
    py::class_<Variable>(m, "_Variable").def(py::init<>());

    // Cinfo.
    py::class_<Cinfo>(m, "_Cinfo")
        .def(py::init<>())
        .def_property_readonly("name", &Cinfo::name)
        .def_property_readonly("finfoMap", &Cinfo::finfoMap,
                               py::return_value_policy::reference)
        // .def("findFinfo", &Cinfo::findFinfoWrapper)
        .def("baseCinfo", &Cinfo::baseCinfo, py::return_value_policy::reference)
        .def("isA", &Cinfo::isA);

    // Shell
    py::class_<Shell>(m, "_Shell")
        .def(py::init<>())
        .def("create", &Shell::doCreate2)
        .def("addMsg", &Shell::doAddMsg)
        .def("getCwe", &Shell::getCwe)
        .def("setClock", &Shell::doSetClock)
        .def("reinit", &Shell::doReinit)
        .def("delete", &Shell::doDelete)
        .def("start", &Shell::doStart, "runtime"_a, "notify"_a = false)
        .def("quit", &Shell::doQuit);

    // Vec
    py::class_<MooseVec>(m, "vec", py::dynamic_attr())
        .def(py::init<const string&, unsigned int, const string&>(), "path"_a,
             "n"_a = 1, "dtype"_a = "Neutral")  // Default
        .def(py::init<const ObjId&>())
        .def("connect", &MooseVec::connectToSingle)
        .def("connect", &MooseVec::connectToVec)
        .def("__len__", &MooseVec::len)
        .def("__iter__",
             [](const MooseVec& v) {
                 return py::make_iterator(v.objs().begin(), v.objs().end());
             },
             py::keep_alive<0, 1>())
        .def("__getitem__", &MooseVec::getItem)
        .def("__setattr__", &MooseVec::setAttrOneToOne)
        .def("__setattr__", &MooseVec::setAttrOneToAll)
        .def("__getattr__", &MooseVec::getAttr)
        .def("__repr__", [](const MooseVec & v)->string {
             return "<moose.vec path=" + v.path() + " class=" + v.dtype() +
                    " size=" + std::to_string(v.size()) + ">";
         })
        // This is to provide old API support. Some scripts use .vec even on a
        // vec to get a vec. So silly or so Zen?!
        .def_property_readonly("vec", [](const MooseVec& vec) { return &vec; },
                               py::return_value_policy::reference_internal)
        .def_property_readonly("type",
                               [](const MooseVec& v) { return "moose.vec"; });

    // Module functions.
    m.def("getShell",
          []() { return reinterpret_cast<Shell*>(Id().eref().data()); },
          py::return_value_policy::reference);

    m.def("seed", [](unsigned int a) { moose::mtseed(a); });
    m.def("rand", [](double a, double b) { return moose::mtrand(a, b); },
          "a"_a = 0, "b"_a = 1);
    m.def("wildcardFind", &wildcardFind2);
    m.def("delete", overload_cast_<const ObjId&>()(&mooseDelete));
    m.def("delete", overload_cast_<const string&>()(&mooseDelete));
    m.def("create", &mooseCreate);
    m.def("reinit", &mooseReinit);
    m.def("start", &mooseStart, "runtime"_a, "notify"_a = false);
    m.def("element", overload_cast_<const ObjId&>()(&mooseElement));
    m.def("element", overload_cast_<const string&>()(&mooseElement));
    m.def("exists", &mooseExists);
    m.def("getCwe", &mooseGetCwe);
    m.def("setClock", &mooseSetClock);
    m.def("useClock", &mooseUseClock);
    m.def("loadModelInternal", &loadModelInternal);
    m.def("getFieldDict", &mooseGetFieldDict, "className"_a,
          "finfoType"_a = "");
    m.def("copy", &mooseCopy, "orig"_a, "newParent"_a, "newName"_a, "num"_a = 1,
          "toGlobal"_a = false, "copyExtMsgs"_a = false);

    // Attributes.
    m.attr("NA") = NA;
    m.attr("PI") = PI;
    m.attr("FaradayConst") = FaradayConst;
    m.attr("GasConst") = GasConst;
    m.attr("__version__") = MOOSE_VERSION;
}
