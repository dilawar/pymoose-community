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

namespace py = pybind11;
using namespace std;

Id initShell();

ObjId createIdFromPath(string path, string type, size_t numData = 1);

Shell* getShellPtr();

bool mooseExists(const string& path);

ObjId mooseElement(const ObjId& oid);
ObjId mooseElement(const string& path);

ObjId loadModelInternal(const string& fname, const string& modelpath,
                        const string& solverclass);

ObjId getElementField(const ObjId objid, const string& fname);

ObjId getElementFieldItem(const ObjId& objid, const string& fname,
                          unsigned int index);

py::object getFieldGeneric(const ObjId& oid, const string& fname);

ObjId mooseConnect(const ObjId& src, const string& srcField, const ObjId& tgt,
                   const string& tgtField);

bool mooseDelete(const ObjId& oid);
bool mooseDelete(const string& path);

Id mooseCreate(const string type, const string& path, size_t numdata = 1);

Id mooseCopy(const Id& orig, ObjId newParent, string newName,
                unsigned int n, bool toGlobal, bool copyExtMsgs);


py::object mooseGetCwe();

void mooseSetClock(const size_t clockId, double dt);

map<string, string> mooseGetFieldDict(const string& className,
                                      const string& finfoType);

void mooseReinit();
void mooseStart(double runtime, bool notify);

py::list getElementFinfo(const ObjId& objid, const string& fname,
                         const Finfo* f);

py::object handleDestFinfo(const ObjId& obj, const string& fname);

py::object getValueFinfo(const ObjId& oid, const string& fname, const Finfo* f);

py::cpp_function getPropertyDestFinfo(const ObjId& oid, const string& fname,
                                      const Finfo* finfo);

py::object getProperty(const ObjId& oid, const string& fname);

py::object getLookupValueFinfo(const ObjId& oid, const string& fname,
                               const Finfo* f);

py::object getLookupValueFinfoItem(const ObjId& oid, const string& fname,
                                   const string& k, const Finfo* f);

#endif /* end of include guard: HELPER_H */
