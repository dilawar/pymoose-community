/**********************************************************************
** This program is part of 'MOOSE', the
** Messaging Object Oriented Simulation Environment.
**           Copyright (C) 2003-2014 Upinder S. Bhalla. and NCBS
** It is made available under the terms of the
** GNU Lesser General Public License version 2.1
** See the file COPYING.LIB for the full notice.
**********************************************************************/

#ifndef _GSOLVE_H
#define _GSOLVE_H
#include <omp.h>

class Stoich;

//////////////////////////////////////////////////////////////////
// NTHREADS = NUM_THREADS
//////////////////////////////////////////////////////////////////
const char *env_value1 = getenv("NUM_THREADS");
int NTHREADS  = atoi(env_value1);

#define _GSOLVE_SEQ 0
#define _GSOLVE_OPENMP 0
#define _GSOLVE_PTHREADS 1

#if _GSOLVE_PTHREADS
#include <pthread.h>
#include <semaphore.h>

struct pthreadGsolveWrap
{
        long tid;
        ProcPtr *P;
        GssaSystem** sysPtr;
        GssaVoxelPools** poolsIndex;
        int *blockSize;
        bool* destroySig; 
        sem_t *sThread, *sMain;

        pthreadGsolveWrap(sem_t* S1, sem_t* S2, long Id, ProcPtr* ptr, GssaSystem** sptr, GssaVoxelPools** pI, int* blz, bool* destroySignal) : sThread(S1), sMain(S2), tid(Id), P(ptr), sysPtr(sptr), poolsIndex(pI), blockSize(blz), destroySig(destroySignal) {} ;
};

#endif //_GSOLVE_PTHREADS

class Gsolve: public ZombiePoolInterface
{
	public: 
		Gsolve();
		~Gsolve();

#if _GSOLVE_PTHREADS

      pthread_t* threads;
      GssaSystem** sPtr;
		bool* destroySignal; 
		ProcPtr *pthreadP; 
		GssaVoxelPools** poolArray_; 
		int *pthreadBlock; 
		sem_t* mainSemaphor; 
		sem_t* threadSemaphor; 

#endif

		//////////////////////////////////////////////////////////////////
		// Field assignment stuff
		//////////////////////////////////////////////////////////////////
		Id getStoich() const;
		void setStoich( Id stoich ); /// Inherited from ZombiePoolInterface.
		Id getCompartment() const;
		void setCompartment( Id compt );

		unsigned int getNumLocalVoxels() const;
		unsigned int getNumAllVoxels() const;
		/**
		 * Assigns the number of voxels used in the entire reac-diff 
		 * system. Note that fewer than this may be used on any given node.
		 */
		void setNumAllVoxels( unsigned int num );

		/**
		 * Assigns number of different pools (chemical species) present in
		 * each voxel.
		 */
		void setNumPools( unsigned int num ); /// Inherited.
		unsigned int getNumPools() const; /// Inherited.
		VoxelPoolsBase* pools( unsigned int i ); /// Inherited.
		double volume( unsigned int i ) const;

		/// Returns the vector of pool Num at the specified voxel.
		vector< double > getNvec( unsigned int voxel) const;
		void setNvec( unsigned int voxel, vector< double > vec );
		//////////////////////////////////////////////////////////////////
		// Dest Finfos
		//////////////////////////////////////////////////////////////////
		void process( const Eref& e, ProcPtr p );
		void reinit( const Eref& e, ProcPtr p );
		void initProc( const Eref& e, ProcPtr p );
		void initReinit( const Eref& e, ProcPtr p );

		/**
		 * Handles request to change volumes of voxels in this Ksolve, and
		 * all cascading effects of this. At this point it won't handle
		 * change in size of voxel array.
		 */
		void updateVoxelVol( vector< double > vols );

		//////////////////////////////////////////////////////////////////
		// Solver setup functions
		//////////////////////////////////////////////////////////////////
		void rebuildGssaSystem();
		void fillMmEnzDep();
		void fillPoolFuncDep();
		void fillIncrementFuncDep();
		void insertMathDepReacs( unsigned int mathDepIndex,
			unsigned int firedReac );
		void makeReacDepsUnique();

		//////////////////////////////////////////////////////////////////
		// Solver interface functions
		//////////////////////////////////////////////////////////////////
		unsigned int getPoolIndex( const Eref& e ) const;
		unsigned int getVoxelIndex( const Eref& e ) const;

		/**
		 * Inherited. Needed for reac-diff calculations so the Gsolve can
		 * orchestrate the data transfer between the itself and the 
		 * diffusion solver.
		 */
		void setDsolve( Id dsolve );
		
		//////////////////////////////////////////////////////////////////
		// ZombiePoolInterface inherited functions
		//////////////////////////////////////////////////////////////////

		void setN( const Eref& e, double v );
		double getN( const Eref& e ) const;
		void setNinit( const Eref& e, double v );
		double getNinit( const Eref& e ) const;
		void setDiffConst( const Eref& e, double v );
		double getDiffConst( const Eref& e ) const;

		void getBlock( vector< double >& values ) const;
		void setBlock( const vector< double >& values );

		/**
		 * Rescale specified voxel rate term following rate constant change 
		 * or volume change. If index == ~0U then does all terms.
		 */
		void updateRateTerms( unsigned int index );


		//////////////////////////////////////////////////////////////////
		/// Flag: returns true if randomized round to integers is done.
		bool getRandInit() const;
		/// Flag: set true if randomized round to integers is to be done.
		void setRandInit( bool val );

		/// Flag: returns true if randomized round to integers is done.
		bool getClockedUpdate() const;
		/// Flag: set true if randomized round to integers is to be done.
		void setClockedUpdate( bool val );

		//////////////////////////////////////////////////////////////////
		static SrcFinfo2< Id, vector< double > >* xComptOut();
		static const Cinfo* initCinfo();
	private:
		GssaSystem sys_;
		/**
		 * Each VoxelPools entry handles all the pools in a single voxel.
		 * Each entry knows how to update itself in order to complete 
		 * the kinetic calculations for that voxel. The ksolver does
		 * multinode management by indexing only the subset of entries
		 * present on this node.
		 */
		vector< GssaVoxelPools > pools_;

		/// First voxel indexed on the current node.
		unsigned int startVoxel_;

		/// Utility ptr used to help Pool Id lookups by the Ksolve.
		Stoich* stoichPtr_;

		/**
		 * Id of diffusion solver, needed for coordinating numerics.
		 */
		Id dsolve_;

		/// Pointer to diffusion solver
		ZombiePoolInterface* dsolvePtr_;
		
		/// Flag: True if atot should be updated every clock tick
		bool useClockedUpdate_;
};

#endif	// _GSOLVE_H
