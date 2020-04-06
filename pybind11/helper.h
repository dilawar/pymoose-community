// =====================================================================================
//
//       Filename:  helper.h
//
//    Description:
//
//        Version:  1.0
//        Created:  03/22/2020 09:06:16 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Dilawar Singh (), dilawar.s.rajput@gmail.com
//   Organization:  NCBS Bangalore
//
// =====================================================================================

#ifndef HELPER_H
#define HELPER_H

#include "../shell/Shell.h"

namespace py = pybind11;
using namespace std;

inline Shell* getShellPtr(void)
{
    return reinterpret_cast<Shell*>(Id().eref().data());
}

Id initShell();

bool mooseExists(const string& path);

void mooseMoveId(const Id& a, const ObjId& b);
void mooseMoveObjId(const ObjId& a, const ObjId& b);

inline ObjId mooseObjIdPath(const string& path)
{
    ObjId oid(path);
    if (oid.bad()) {
        cerr << "moose_element: " << path << " does not exist!" << endl;
        return ObjId(Id());
    }
    return oid;
}

inline ObjId mooseObjIdObj(const ObjId& obj)
{
    return ObjId(obj.id, obj.dataIndex, obj.fieldIndex);
}

inline ObjId mooseObjIdId(const Id& id)
{
    return ObjId(id);
}

inline ObjId mooseCreateFromPath(const string type, const string& path, unsigned int numdata)
{

#if 0
    // NOTE: This function is costly because of regex use. But it can be
    // enabled later.
    auto newpath = moose::normalizePath(path);
#endif

    // If path exists and user is asking for the same type then return the
    // object else raise exception.
    if(mooseExists(path)) {
        auto oid = ObjId(path);
        if(oid.element()->cinfo()->name() == type)
            return oid;
    }

    // Split into dirname and basename component.
    auto p = moose::splitPath(path);

    // Name must not end with [\d*] etc.  normalizePath takes care of it if
    // enabled. 
    string name(p.second);
    if(name.back() == ']')
        name = name.substr(0, name.find_last_of('['));
    return getShellPtr()->doCreate2(type, ObjId(p.first), name, numdata);
}

inline ObjId mooseCreateFromObjId(const string& type, const ObjId& oid, unsigned int numData)
{
    return oid;
}

inline ObjId mooseCreateFromId(const string& type, const Id& id, unsigned int numData)
{
    return mooseCreateFromObjId(type, ObjId(id), numData);
}


ObjId loadModelInternal(const string& fname, const string& modelpath,
                        const string& solverclass);

ObjId getElementField(const ObjId objid, const string& fname);

ObjId getElementFieldItem(const ObjId& objid, const string& fname,
                          unsigned int index);

// Connect using doConnect
ObjId shellConnect(const ObjId& src, const string& srcField, const ObjId& tgt,
                   const string& tgtField, const string& msgType);

inline bool mooseDeleteObj(const ObjId& oid)
{
    return getShellPtr()->doDelete(oid);
}

inline bool mooseDeleteStr(const string& path)
{
    return getShellPtr()->doDelete(ObjId(path));
}


ObjId mooseCreate(const string type, const string& path,
                  unsigned int numdata = 1);

ObjId mooseCopy(const py::object& orig, ObjId newParent, string newName,
                unsigned int n, bool toGlobal, bool copyExtMsgs);

py::object mooseGetCwe();
void mooseSetCwe(const ObjId& oid);

void mooseSetClock(const unsigned int clockId, double dt);

void mooseUseClock(size_t tick, const string& path, const string& field);

map<string, string> mooseGetFieldDict(const string& className,
                                      const string& finfoType);

void mooseReinit();

void mooseStart(double runtime, bool notify);

void mooseStop();

py::cpp_function getPropertyDestFinfo(const ObjId& oid, const Finfo* finfo);

vector<string> mooseGetFieldNames(const string& className, const string& finfoType);

#endif /* end of include guard: HELPER_H */
