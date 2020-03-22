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

// Macro to create the Shell * out of shellId
#define SHELLPTR (reinterpret_cast<Shell*>(getShell().eref().data()))


Id initShell();

Id getShell();

Id createIdFromPath(string path, string type, size_t numData=1);

Shell* getShellPtr();

#endif /* end of include guard: HELPER_H */
