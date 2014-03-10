/**********************************************************************
** This program is part of 'MOOSE', the
** Messaging Object Oriented Simulation Environment.
**           Copyright (C) 2003-2010 Upinder S. Bhalla. and NCBS
** It is made available under the terms of the
** GNU Lesser General Public License version 2.1
** See the file COPYING.LIB for the full notice.
**********************************************************************/
#include "header.h"
#ifdef USE_GSL
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv2.h>
#endif

#include "OdeSystem.h"
#include "VoxelPools.h"
#include "RateTerm.h"
#include "FuncTerm.h"
#include "SparseMatrix.h"
#include "KinSparseMatrix.h"
#include "Stoich.h"

//////////////////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////////////////

VoxelPools::VoxelPools()
	: 
		S_(1),
		Sinit_(1)
{
#ifdef USE_GSL
		driver_ = 0;
#endif
}

VoxelPools::~VoxelPools()
{
#ifdef USE_GSL
	if ( driver_ )
		gsl_odeiv2_driver_free( driver_ );
#endif
}

//////////////////////////////////////////////////////////////
// Array ops
//////////////////////////////////////////////////////////////
/// Using the computed array sizes, now allocate space for them.
void VoxelPools::resizeArrays( unsigned int totNumPools )
{
	S_.resize( totNumPools, 0.0 );
	Sinit_.resize( totNumPools, 0.0);
}

void VoxelPools::reinit()
{
	S_ = Sinit_;
}

//////////////////////////////////////////////////////////////
// Access functions
//////////////////////////////////////////////////////////////
const double* VoxelPools::S() const
{
	return &S_[0];
}

double* VoxelPools::varS()
{
	return &S_[0];
}

const double* VoxelPools::Sinit() const
{
	return &Sinit_[0];
}

double* VoxelPools::varSinit()
{
	return &Sinit_[0];
}

unsigned int VoxelPools::size() const
{
	return Sinit_.size();
}

//////////////////////////////////////////////////////////////
// Solver ops
//////////////////////////////////////////////////////////////
void VoxelPools::setStoich( const Stoich* s, const OdeSystem* ode )
{
#ifdef USE_GSL
	sys_ = ode->gslSys;
	if ( driver_ )
		gsl_odeiv2_driver_free( driver_ );
	driver_ = gsl_odeiv2_driver_alloc_y_new( 
		&sys_, ode->gslStep, ode->initStepSize, 
		ode->epsAbs, ode->epsRel );
#endif
}

void VoxelPools::advance( const ProcInfo* p )
{
#ifdef USE_GSL
	double t = p->currTime - p->dt;
	int status = gsl_odeiv2_driver_apply( driver_, &t, p->currTime, &S_[0]);
	if ( status != GSL_SUCCESS ) {
		cout << "Error: VoxelPools::advance: GSL integration error at time "
			 << t << "\n";
		assert( 0 );
	}
#endif
}

// static func. This is the function that goes into the Gsl solver.
int VoxelPools::gslFunc( double t, const double* y, double *dydt, 
						void* params )
{
	Stoich* s = reinterpret_cast< Stoich* >( params );
	double* q = const_cast< double* >( y ); // Assign the func portion.

	// Assign the buffered pools
	// Not possible because this is a static function
	// Not needed because dydt = 0;
	/*
	double* b = q + s->getNumVarPools();
	vector< double >::const_iterator sinit = Sinit_.begin() + s->getNumVarPools();
	for ( unsigned int i = 0; i < s->getNumBufPools(); ++i )
		*b++ = *sinit++;
		*/

	s->updateFuncs( q, t );
	s->updateRates( y, dydt );
#ifdef USE_GSL
	return GSL_SUCCESS;
#else
	return 0;
#endif
}

//////////////////////////////////////////////////////////////
// Zombie Pool Access functions
//////////////////////////////////////////////////////////////

void VoxelPools::setN( unsigned int i, double v )
{
	S_[i] = v;
}

double VoxelPools::getN( unsigned int i ) const
{
	return S_[i];
}

void VoxelPools::setNinit( unsigned int i, double v )
{
	Sinit_[i] = v;
}

double VoxelPools::getNinit( unsigned int i ) const
{
	return Sinit_[i];
}

void VoxelPools::setDiffConst( unsigned int i, double v )
{
		; // Do nothing.
}

double VoxelPools::getDiffConst( unsigned int i ) const
{
		return 0;
}

