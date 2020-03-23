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

#include "../basecode/header.h"
// #include "../basecode/global.h"
#include "../shell/Shell.h"
#include "../scheduling/Clock.h"
#include "../mpi/PostMaster.h"
#include "../utility/strutil.h"

#include "helper.h"

using namespace std;

Id initShell(void)
{
    Cinfo::rebuildOpIndex();

    Id shellId;

    Element* shelle = new GlobalDataElement(shellId, Shell::initCinfo(), "/", 1 );

    Id clockId = Id::nextId();
    assert( clockId.value() == 1 );
    Id classMasterId = Id::nextId();
    Id postMasterId = Id::nextId();

    Shell* s = reinterpret_cast< Shell* >( shellId.eref().data() );
    s->setHardware(1, 1, 0);
    s->setShellElement( shelle );

    /// Sets up the Elements that represent each class of Msg.
    auto numMsg = Msg::initMsgManagers();

    new GlobalDataElement(clockId, Clock::initCinfo(), "clock", 1);
    new GlobalDataElement(classMasterId, Neutral::initCinfo(), "classes", 1);
    new GlobalDataElement(postMasterId, PostMaster::initCinfo(), "postmaster", 1);

    assert ( shellId == Id() );
    assert( clockId == Id( 1 ) );
    assert( classMasterId == Id( 2 ) );
    assert( postMasterId == Id( 3 ) );

    Shell::adopt( shellId, clockId, numMsg++ );
    Shell::adopt( shellId, classMasterId, numMsg++ );
    Shell::adopt( shellId, postMasterId, numMsg++ );
    assert( numMsg == 10 ); // Must be the same on all nodes.

    Cinfo::makeCinfoElements( classMasterId );
    return shellId;
}


/**
   Utility function to create objects from full path, dimensions
   and classname.
*/
Id createIdFromPath(string path, string type, size_t numData)
{
    Shell* pShell = reinterpret_cast<Shell*>(Id().eref().data());
    string parent_path;
    string name;

    string trimmed_path = moose::trim( path );
    size_t pos = trimmed_path.rfind("/");
    if (pos != string::npos)
    {
        name = trimmed_path.substr(pos+1);
        parent_path = trimmed_path.substr(0, pos);
    }
    else
    {
        name = trimmed_path;
    }
    // handle relative path
    if (trimmed_path[0] != '/')
    {
        string current_path = pShell->getCwe().path();
        if (current_path != "/")
            parent_path = current_path + "/" + parent_path;
        else
            parent_path = current_path + parent_path;
    }
    else if (parent_path.empty())
        parent_path = "/";

    ObjId parent_id(parent_path);
    if (parent_id.bad() )
    {
        string message = "Parent element does not exist: ";
        message += parent_path;
        throw std::runtime_error(message);
        return Id();
    }

    Id nId =  pShell->doCreate(type,
                                 parent_id,
                                 string(name),
                                 numData,
                                 MooseGlobal
                                );

    if (nId == Id() && trimmed_path != "/" && trimmed_path != "/root")
    {
        string message = "no such moose class : " + type;
        throw std::runtime_error(message);
    }

    return nId;
}

