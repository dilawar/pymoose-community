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

using namespace std;

Id initShell();

ObjId createIdFromPath(string path, string type, size_t numData=1);

Shell* const getShellPtr();

bool doesExist(const string& path);

ObjId element(const string& path);

ObjId loadModelInternal(const string& fname, const string& modelpath, const string& solverclass);

#endif /* end of include guard: HELPER_H */
