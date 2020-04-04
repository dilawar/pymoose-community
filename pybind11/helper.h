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

class Shell;

Id initShell();

ObjId createIdFromPath(string path, string type, unsigned int numData = 1);

Shell* getShellPtr();

bool mooseExists(const string& path);

void mooseMoveId(const Id& a, const ObjId& b);
void mooseMoveObjId(const ObjId& a, const ObjId& b);

ObjId mooseElement(const string& path);

ObjId loadModelInternal(const string& fname, const string& modelpath,
                        const string& solverclass);

ObjId getElementField(const ObjId objid, const string& fname);

ObjId getElementFieldItem(const ObjId& objid, const string& fname,
                          unsigned int index);

// Connect using doConnect
ObjId shellConnect(const ObjId& src, const string& srcField, const ObjId& tgt,
                   const string& tgtField, const string& msgType);

bool mooseDelete(const ObjId& oid);
bool mooseDelete(const string& path);

ObjId mooseCreate(const string type, const string& path,
                  unsigned int numdata = 1);

ObjId mooseCopy(const py::object& orig, ObjId newParent, string newName,
                unsigned int n, bool toGlobal, bool copyExtMsgs);

py::object mooseGetCwe();

void mooseSetClock(const unsigned int clockId, double dt);

void mooseUseClock(size_t tick, const string& path, const string& field);

vector<string> mooseGetFieldNames(const string& className,
                                  const string& finfoType);

map<string, string> mooseGetFieldDict(const string& className,
                                      const string& finfoType);

void mooseReinit();
void mooseStart(double runtime, bool notify);

py::cpp_function getPropertyDestFinfo(const ObjId& oid, const Finfo* finfo);

#endif /* end of include guard: HELPER_H */
