// =====================================================================================
//
//       Filename:  pymoose.h
//
//    Description: pymoose module.
//
//        Version:  1.0
//        Created:  03/17/2020 05:32:37 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Dilawar Singh (), dilawar.s.rajput@gmail.com
//   Organization:  NCBS Bangalore
//
// =====================================================================================

#ifndef PYMOOSE_H
#define PYMOOSE_H

Id initShell(void);

/**
   Return the Id of the Shell object.
*/
Id getShell(int argc, char ** argv);

// Macro to create the Shell * out of shellId
#define SHELLPTR (reinterpret_cast<Shell*>(getShell(0, NULL).eref().data()))

Id createIdFromPath(string path, string type, size_t numData=1);


#endif /* end of include guard: PYMOOSE_H */
