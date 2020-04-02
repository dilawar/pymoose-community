// =====================================================================================
//
//       Filename:  helper.cpp
//
//    Description: Helper functions.
//
//        Version:  1.0
//        Created:  03/22/2020 09:05:13 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Dilawar Singh (), dilawar.s.rajput@gmail.com
//   Organization:  NCBS Bangalore
//
// =====================================================================================

#include <stdexcept>
#include <memory>

#include "../external/pybind11/include/pybind11/pybind11.h"
#include "../external/pybind11/include/pybind11/stl.h"
#include "../external/pybind11/include/pybind11/numpy.h"
#include "../external/pybind11/include/pybind11/functional.h"

// See
// https://pybind11.readthedocs.io/en/stable/advanced/cast/stl.html#binding-stl-containers
// #include "../external/pybind11/include/pybind11/stl_bind.h"

#include "../basecode/header.h"
#include "../basecode/global.h"
#include "../basecode/Cinfo.h"

#include "../shell/Shell.h"
#include "../shell/Wildcard.h"
#include "../shell/Neutral.h"

#include "../scheduling/Clock.h"
#include "../mpi/PostMaster.h"

#include "../builtins/Variable.h"

#include "../utility/strutil.h"

#include "helper.h"
#include "Finfo.h"
#include "pymoose.h"

using namespace std;

namespace py = pybind11;

using namespace std;

Id initShell(void)
{
    Cinfo::rebuildOpIndex();

    Id shellId;

    Element* shelle =
        new GlobalDataElement(shellId, Shell::initCinfo(), "/", 1);

    Id clockId = Id::nextId();
    assert(clockId.value() == 1);
    Id classMasterId = Id::nextId();
    Id postMasterId = Id::nextId();

    Shell* s = reinterpret_cast<Shell*>(shellId.eref().data());
    s->setHardware(1, 1, 0);
    s->setShellElement(shelle);

    /// Sets up the Elements that represent each class of Msg.
    auto numMsg = Msg::initMsgManagers();

    new GlobalDataElement(clockId, Clock::initCinfo(), "clock", 1);
    new GlobalDataElement(classMasterId, Neutral::initCinfo(), "classes", 1);
    new GlobalDataElement(postMasterId, PostMaster::initCinfo(), "postmaster",
                          1);

    assert(shellId == Id());
    assert(clockId == Id(1));
    assert(classMasterId == Id(2));
    assert(postMasterId == Id(3));

    Shell::adopt(shellId, clockId, numMsg++);
    Shell::adopt(shellId, classMasterId, numMsg++);
    Shell::adopt(shellId, postMasterId, numMsg++);
    assert(numMsg == 10);  // Must be the same on all nodes.

    Cinfo::makeCinfoElements(classMasterId);
    return shellId;
}

/**
   Utility function to create objects from full path, dimensions
   and classname.
*/
ObjId createIdFromPath(string path, string type, unsigned int numData)
{
    Shell* pShell = reinterpret_cast<Shell*>(Id().eref().data());
    string parent_path;
    string name;

    string trimmed_path = moose::trim(path);
    unsigned int pos = trimmed_path.rfind("/");
    if (pos != string::npos) {
        name = trimmed_path.substr(pos + 1);
        parent_path = trimmed_path.substr(0, pos);
    } else {
        name = trimmed_path;
    }
    // handle relative path
    if (trimmed_path[0] != '/') {
        string current_path = pShell->getCwe().path();
        if (current_path != "/")
            parent_path = current_path + "/" + parent_path;
        else
            parent_path = current_path + parent_path;
    } else if (parent_path.empty())
        parent_path = "/";

    ObjId parent_id(parent_path);
    if (parent_id.bad()) {
        string message = "Parent element does not exist: ";
        message += parent_path;
        throw std::runtime_error(message);
        return Id();
    }

    auto nId =
        pShell->doCreate(type, parent_id, string(name), numData, MooseGlobal);

    if (nId == Id() && trimmed_path != "/" && trimmed_path != "/root") {
        string message = "no such moose class : " + type;
        throw std::runtime_error(message);
    }

    return nId;
}

Shell* getShellPtr(void)
{
    return reinterpret_cast<Shell*>(Id().eref().data());
}

bool mooseExists(const string& path)
{
    return Id(path) != Id() || path == "/" || path == "/root";
}

ObjId mooseElement(const string& path)
{
    ObjId oid(path);
    if (oid.bad()) {
        cerr << "moose_element: " << path << " does not exist!" << endl;
        return ObjId(Id());
    }
    return oid;
}

ObjId loadModelInternal(const string& fname, const string& modelpath,
                        const string& solverclass = "")
{
    Id model;
    if (solverclass.empty()) {
        model = getShellPtr()->doLoadModel(fname, modelpath);
    } else {
        model = getShellPtr()->doLoadModel(fname, modelpath, solverclass);
    }

    if (model == Id()) {
        throw runtime_error("could not load model");
        return Id();
    }
    return ObjId(model);
}

ObjId getElementField(const ObjId objid, const string& fname)
{
    return ObjId(objid.path() + '/' + fname);
}

ObjId getElementFieldItem(const ObjId& objid, const string& fname,
                          unsigned int index)
{
    ObjId oid = getElementField(objid, fname);

    auto len = Field<unsigned int>::get(oid, "numField");
    assert(len >= 0);

    if (index >= len) {
        throw runtime_error(
            "ElementField.getItem: index out of bounds. "
            "Total elements=" +
            to_string(len) + ".");
        return ObjId();
    }

    // Negative indexing. Thanks Subha for hint.
    if (index < 0) {
        index += len;
    }
    if (index < 0) {
        throw runtime_error("ElementField.getItem: invalid index: " +
                            to_string(index) + ".");
        return ObjId();
    }
    return ObjId(oid.id, oid.dataIndex, index);
}

ObjId mooseConnect(const ObjId& src, const string& srcField, const ObjId& tgt,
                   const string& tgtField, const string& msgType)
{
    auto pShell = getShellPtr();
    return pShell->doAddMsg(msgType, src, srcField, tgt, tgtField);
}

bool mooseDelete(const ObjId& oid)
{
    return getShellPtr()->doDelete(oid);
}

bool mooseDelete(const string& path)
{
    return getShellPtr()->doDelete(ObjId(path));
}

ObjId mooseCreate(const string type, const string& path, unsigned int numdata)
{
    auto p = moose::splitPath(path);
    return getShellPtr()->doCreate2(type, ObjId(p.first), p.second, numdata);
}

void mooseSetClock(const unsigned int clockId, double dt)
{
    getShellPtr()->doSetClock(clockId, dt);
}

void mooseUseClock(size_t tick, const string& path, const string& field)
{
    getShellPtr()->doUseClock(path, field, tick);
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Current Working Element.
 *
 * @Returns  cwe.
 */
/* ----------------------------------------------------------------------------*/
py::object mooseGetCwe()
{
    return py::cast(getShellPtr()->getCwe());
}

map<string, string> mooseGetFieldDict(const string& className,
                                      const string& finfoType = "")
{
    const Cinfo* cinfo = Cinfo::find(className);
    if (!cinfo) {
        cout << "Warning: Invalid class " << className << endl;
        return {};
    }

    map<string, string> fieldDict;
    if (finfoType == "") {
        auto finfos = cinfo->finfoMap();
        for (auto& v : finfos) fieldDict[v.first] = v.second->rttiType();
        return fieldDict;
    }

    // Now the specific one.
    // FIXME: Fix the typeids  or remove the 'get' and 'set'
    if (finfoType == "valueFinfo" || finfoType == "value") {
        for (unsigned int ii = 0; ii < cinfo->getNumValueFinfo(); ++ii) {
            auto* finfo = cinfo->getValueFinfo(ii);
            fieldDict[finfo->name()] = finfo->rttiType();
        }
    } else if (finfoType == "srcFinfo" || finfoType == "src") {
        for (unsigned int ii = 0; ii < cinfo->getNumSrcFinfo(); ++ii) {
            auto* finfo = cinfo->getSrcFinfo(ii);
            fieldDict[finfo->name()] = finfo->rttiType();
        }
    } else if (finfoType == "destFinfo" || finfoType == "dest") {
        for (unsigned int ii = 0; ii < cinfo->getNumDestFinfo(); ++ii) {
            auto* finfo = cinfo->getDestFinfo(ii);
            fieldDict[finfo->name()] = finfo->rttiType();
        }
    } else if (finfoType == "lookupFinfo" || finfoType == "lookup") {
        for (unsigned int ii = 0; ii < cinfo->getNumLookupFinfo(); ++ii) {
            auto* finfo = cinfo->getLookupFinfo(ii);
            fieldDict[finfo->name()] = finfo->rttiType();
        }
    } else if (finfoType == "sharedFinfo" || finfoType == "shared") {
        for (unsigned int ii = 0; ii < cinfo->getNumSrcFinfo(); ++ii) {
            auto* finfo = cinfo->getSrcFinfo(ii);
            fieldDict[finfo->name()] = finfo->rttiType();
        }
    } else if (finfoType == "fieldElementFinfo" || finfoType == "field" ||
               finfoType == "fieldElement") {
        for (unsigned int ii = 0; ii < cinfo->getNumFieldElementFinfo(); ++ii) {
            auto* finfo = cinfo->getFieldElementFinfo(ii);
            fieldDict[finfo->name()] = finfo->rttiType();
        }
    }
    return fieldDict;
}

void mooseReinit()
{
    getShellPtr()->doReinit();
}

void mooseStart(double runtime, bool notify = false)
{
    getShellPtr()->doStart(runtime, notify);
}

// ObjId mooseCopy(const ObjId& orig, ObjId newParent, string newName, unsigned
// int n=1
//        , bool toGlobal=false, bool copyExtMsgs=false)
//{
//    auto id =  getShellPtr()->doCopy(orig.id, newParent, newName, n, toGlobal,
// copyExtMsgs);
//    return ObjId(id);
//}
//
//ObjId mooseCopy(const ObjId& orig, ObjId newParent, string newName,
//                unsigned int n = 1, bool toGlobal = false,
//                bool copyExtMsgs = false)
//{
//    return ObjId(getShellPtr()->doCopy(orig, newParent, newName, n, toGlobal,
//                                       copyExtMsgs));
//}

ObjId mooseCopy(const py::object& elem, ObjId newParent, string newName,
                unsigned int n = 1, bool toGlobal = false,
                bool copyExtMsgs = false)
{
    // py::print("Copyging..", elem);
    Id orig = py::cast<Id>(elem);
    return ObjId(getShellPtr()->doCopy(orig, newParent, newName, n, toGlobal,
                                       copyExtMsgs));
}


py::object getValueFinfo(const ObjId& oid, const Finfo* f)
{
    auto rttType = f->rttiType();
    auto fname = f->name();
    py::object r = py::none();

    if (rttType == "double" or rttType == "float")
        r = pybind11::float_(getField<double>(oid, fname));
    else if (rttType == "vector<double>") {
        // r = py::cast(getField<vector<double>>(oid, fname));
        r = getFieldNumpy<double>(oid, fname);
    } else if (rttType == "string")
        r = pybind11::str(getField<string>(oid, fname));
    else if (rttType == "char")
        r = pybind11::int_(getField<char>(oid, fname));
    else if (rttType == "int")
        r = pybind11::int_(getField<int>(oid, fname));
    else if (rttType == "unsigned int")
        r = pybind11::int_(getField<unsigned int>(oid, fname));
    else if (rttType == "unsigned long")
        r = pybind11::int_(getField<unsigned long>(oid, fname));
    else if (rttType == "bool")
        r = pybind11::bool_(getField<bool>(oid, fname));
    else if (rttType == "Id")
        r = py::cast(getField<Id>(oid, fname));
    else if (rttType == "ObjId")
        r = py::cast(getField<ObjId>(oid, fname));
    else if (rttType == "Variable")
        r = py::cast(getField<Variable>(oid, fname));
    else if (rttType == "vector<Id>")
        r = py::cast(getField<vector<Id>>(oid, fname));
    else if (rttType == "vector<ObjId>")
        r = py::cast(getField<vector<ObjId>>(oid, fname));
    else {
        cout << "Warning: getFielderty:: Unsupported type '" << rttType << "'"
             << endl;
        r = py::none();
    }
    return r;
}

py::list getElementFinfo(const ObjId& objid, const Finfo* f)
{
    auto rttType = f->rttiType();
    auto fname = f->name();
    auto oid = ObjId(objid.path() + '/' + fname);
    auto len = Field<unsigned int>::get(oid, "numField");
    vector<ObjId> res(len);
    for (unsigned int i = 0; i < len; i++)
        res[i] = ObjId(oid.path(), oid.dataIndex, i);
    return py::cast(res);
}

py::cpp_function getFieldPropertyDestFinfo(const ObjId& oid, const Finfo* finfo)
{
    const auto rttType = finfo->rttiType();
    const auto fname = finfo->name();

    if (rttType == "void") {
        std::function<bool()> func = [oid, fname]() {
            return SetGet0::set(oid, fname);
        };
        return func;
    }
    if (rttType == "vector<Id>") {
        std::function<bool(const vector<Id>&)> func = [oid, fname](
            const vector<Id>& ids) {
            return SetGet1<vector<Id>>::set(oid, fname, ids);
        };
        return func;
    }
    if (rttType == "vector<ObjId>") {
        std::function<bool(const vector<ObjId>&)> func = [oid, fname](
            const vector<ObjId>& ids) {
            return SetGet1<vector<ObjId>>::set(oid, fname, ids);
        };
        return func;
    }
    if (rttType == "vector<double>") {
        std::function<bool(const vector<double>&)> func = [oid, fname](
            const vector<double>& data) {
            return SetGet1<vector<double>>::set(oid, fname, data);
        };
        return func;
    }

    throw runtime_error("getFieldPropertyDestFinfo::NotImplemented " + fname +
                        " for rttType " + rttType + " for oid " + oid.path());
}

