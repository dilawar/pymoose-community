// =====================================================================================
//
//       Filename:  pymoose.cpp
//
//    Description:  
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

#include <vector>
#include <typeinfo>
#include <utility>

#include "../basecode/header.h"
#include "../shell/Shell.h"
#include "../scheduling/Clock.h"
#include "../utility/print_function.hpp"
#include "../utility/utility.h"
#include "../utility/strutil.h"
#include "../mpi/PostMaster.h"

#include "pymoose.h"

class Pool;
class BufPool;

#include "../external/pybind11/include/pybind11/pybind11.h"


using namespace std;
namespace py = pybind11;

const vector<const char*> classMap  { "AdExIF", "AdThreshIF", "Adaptor",
    "Annotator", "Arith", "BufPool", "CaConc", "Cell", "ChemCompt", "Cinfo",
    "Clock", "Compartment", "ConcChan",  "CubeMesh", "CylMesh", "DestField",
    "DiagonalMsg", "DifBuffer",  "DifShell", "DiffAmp", "Dsolve",
    "ElementField", "EndoMesh", "Enz", "ExIF", "Finfo", "Function",
    "GapJunction", "GraupnerBrunel2012CaPlasticitySynHandler", "Group",
    "Gsolve", "HHChannel", "HHChannel2D",  "HHGate", "HHGate2D", "HSolve",
    "INFINITE", "IntFire",  "Interpol", "Interpol2D", "IzhIF", "IzhikevichNrn",
    "Ksolve", "LIF", "Leakage", "LookupField", "MMPump", "MMenz",
    "MarkovChannel", "MarkovGslSolver", "MarkovRateTable", "MarkovSolver",
    "MeshEntry", "MgBlock", "Msg", "Mstring", "NMDAChan", "Nernst",
    "NeuroMesh", "Neuron", "Neutral", "OneToAllMsg", "OneToOneDataIndexMsg",
    "OneToOneMsg", "PIDController", "Pool",  "PostMaster", "PsdMesh",
    "PulseGen", "PyRun", "QIF", "RC", "RandSpike", "Reac",  "STDPSynHandler",
    "STDPSynapse", "SeqSynHandler", "Shell", "SimpleSynHandler", "SingleMsg",
    "SocketStreamer", "SparseMsg", "Species", "SpikeGen", "SpikeStats",
    "Spine", "SpineMesh", "Stats", "SteadyState", "StimulusTable", "Stoich",
    "Streamer", "SymCompartment", "SynChan",  "Synapse", "Table", "Table2",
    "TimeTable", "VClamp" };

Id initShell(void)
{
    Cinfo::rebuildOpIndex();
    Id shellId;
    Element* shelle = new GlobalDataElement( shellId, Shell::initCinfo(), "root", 1 );

    Id clockId = Id::nextId();
    assert( clockId.value() == 1 );
    Id classMasterId = Id::nextId();
    Id postMasterId = Id::nextId();

    Shell* s = reinterpret_cast< Shell* >( shellId.eref().data() );
    s->setHardware(1, 1, 0);
    s->setShellElement( shelle );

    /// Sets up the Elements that represent each class of Msg.
    auto numMsg = Msg::initMsgManagers();

    new GlobalDataElement( clockId, Clock::initCinfo(), "clock", 1 );
    new GlobalDataElement( classMasterId, Neutral::initCinfo(), "classes", 1);
    new GlobalDataElement( postMasterId, PostMaster::initCinfo(), "postmaster", 1 );

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


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  getShell.
 *
 * @Param argc
 * @Param argv
 *
 * @Returns   
 */
/* ----------------------------------------------------------------------------*/
Id getShell(int argc, char ** argv)
{
    static int inited = 0;
    if (inited)
        return Id(0);

    Id shellId = initShell();
    inited = 1;

    Shell * shellPtr = reinterpret_cast<Shell*>(shellId.eref().data());
    if ( shellPtr->myNode() == 0 )
    {
        if ( Shell::numNodes() > 1 )
        {
            // Use the last clock for the postmaster, so that it is called
            // after everything else has been processed and all messages
            // are ready to send out.
            shellPtr->doUseClock( "/postmaster", "process", 9 );
            shellPtr->doSetClock( 9, 1.0 ); // Use a sensible default.
        }
    }
    return shellId;
} 

/**
   Utility function to create objects from full path, dimensions
   and classname.
*/
Id createIdFromPath(string path, string type, size_t numData)
{
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
        string current_path = SHELLPTR->getCwe().path();
        if (current_path != "/")
        {
            parent_path = current_path + "/" + parent_path;
        }
        else
        {
            parent_path = current_path + parent_path;
        }
    }
    else if (parent_path.empty())
    {
        parent_path = "/";
    }
    ObjId parent_id(parent_path);
    if (parent_id.bad() )
    {
        string message = "Parent element does not exist: ";
        message += parent_path;
        PyErr_SetString(PyExc_ValueError, message.c_str());
        return Id();
    }

    Id nId =  SHELLPTR->doCreate(type,
                                 parent_id,
                                 string(name),
                                 numData,
                                 MooseGlobal
                                );

    if (nId == Id() && trimmed_path != "/" && trimmed_path != "/root")
    {
        string message = "no such moose class : " + type;
        PyErr_SetString(PyExc_TypeError, message.c_str());
    }

    return nId;
}

PYBIND11_MODULE(_moose, m)
{
    m.doc() = R"moosedoc(moose module.
    )moosedoc";

    py::class_<ObjId>(m, "ObjId")
        .def(py::init<>())
        ;

    py::class_<Id>(m, "Id")
        .def(py::init<>())
        ;

    // Add Shell Class.
    py::class_<Shell>(m, "Shell")
        .def(py::init<>())
        .def("doCreate", &Shell::doCreate)
        .def("doDelete", &Shell::doDelete)
        .def("doAddMsg", &Shell::doAddMsg)
        .def("doQuit", &Shell::doQuit)
        .def("doStart", &Shell::doStart)
        .def("doReinit", &Shell::doReinit)
        .def("doStop", &Shell::doStop)
        .def("doMove", &Shell::doMove)
        .def("doCopy", &Shell::doCopy)
        .def("destroy", &Shell::destroy)
        .def("doFind", &Shell::doFind)
        .def("doLoadModel", &Shell::doLoadModel)
        .def("doSaveModel", &Shell::doSaveModel)
        .def("handleCreate", &Shell::handleCreate)
        .def("handleCopy", &Shell::handleCopy)
        .def("handleQuit", &Shell::handleQuit)
        ;

    m.def("create", &createIdFromPath);

    m.attr("__version__") = MOOSE_VERSION;

}
