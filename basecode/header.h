/**********************************************************************
** This program is part of 'MOOSE', the
** Messaging Object Oriented Simulation Environment,
** also known as GENESIS 3 base code.
**           copyright (C) 2003-2004 Upinder S. Bhalla. and NCBS
** It is made available under the terms of the
** GNU Lesser Public License version 2.1
** See the file COPYING.LIB for the full notice.
**********************************************************************/

#ifndef _BASECODE_HEADER_H
#define _BASECODE_HEADER_H

#include <math.h>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <typeinfo> // used in Conv.h to extract compiler independent typeid

#include <cassert>

using namespace std;


/**
 * Looks up and uniquely identifies functions, on a per-Cinfo basis.
 * These are NOT global indices to identify the function.
 */
typedef unsigned int FuncId;

/**
 * Looks up data entries.
 */
typedef unsigned int DataId;

/**
 * Identifies data entry on an Element. This is a global index,
 * in that it does not refer to the array on any given node, but uniquely
 * identifies the entry over the entire multinode simulation.
 */
/**
 * Index into Element::vector< vector< MsgFuncBinding > > msgBinding_;
 */
typedef unsigned short BindIndex;

constexpr double PI = 3.141592653589793;
constexpr double NA = 6.0221415e23;
constexpr double FaradayConst =  96485.3415; // s A / mol
constexpr double GasConst = 8.3144621; // R, units are J/(K.mol)
constexpr unsigned int ALLDATA = ~0U;
constexpr unsigned int BADINDEX = ~1U;

class Element;
class Eref;
class OpFunc;
class Cinfo;
class SetGet;
class FuncBarrier;
class ObjId;
class SrcFinfo;

#include "../builtins/Variable.h"
#include "doubleEq.h"
#include "Id.h"
#include "ObjId.h"
#include "Finfo.h"
#include "DestFinfo.h"
#include "ProcInfo.h"
#include "Cinfo.h"
#include "MsgFuncBinding.h"
#include "../msg/Msg.h"
#include "Dinfo.h"
#include "MsgDigest.h"
#include "Element.h"
#include "DataElement.h"
#include "GlobalDataElement.h"
#include "LocalDataElement.h"
#include "Eref.h"
#include "Conv.h"
#include "SrcFinfo.h"

extern DestFinfo* receiveGet();
class Neutral;
#include "OpFuncBase.h"
#include "HopFunc.h"
#include "SetGet.h"
#include "OpFunc.h"
#include "EpFunc.h"
#include "ProcOpFunc.h"
#include "ValueFinfo.h"
#include "LookupValueFinfo.h"
#include "ValueFinfo.h"
#include "SharedFinfo.h"
#include "FieldElementFinfo.h"
#include "FieldElement.h"
#include "../shell/Neutral.h"

#endif 
