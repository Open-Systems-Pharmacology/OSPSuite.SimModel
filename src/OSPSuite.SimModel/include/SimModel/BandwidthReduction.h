#ifndef _BandwidthReduction_H_
#define _BandwidthReduction_H_

#include "SimModel/Simulation.h"

namespace SimModelNative
{

	class BandwidthReductionTask
	{
	private:
		Simulation * _sim;

		//---- lower and upper half band range of the ODE system
	    //The half-bandwidths are set such that the nonzero locations (i, j) 
	    //in the banded Jacobian satisfy 
        // -lowerHalfBandWidth <= j-i <= upperHalfBandWidth
		int _lowerHalfBandWidth;
		int _upperHalfBandWidth;

	public:
		BandwidthReductionTask(Simulation * sim);
		void ReorderDEVariables();

		int GetLowerHalfBandWidth();
		int GetUpperHalfBandWidth();

	protected:
		std::vector<std::vector<bool> > getDependencyMatrix();

		//Reorder DE variables according to the given permutation
		void reorderDEVariables(std::vector<unsigned int> indicesPermutation);

		void calculateHalfBandWidths();
	};

}//.. end "namespace SimModelNative"


#endif //_BandwidthReduction_H_

