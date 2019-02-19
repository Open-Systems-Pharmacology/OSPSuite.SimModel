#ifdef _WINDOWS_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/BandwidthReduction.h"
#include "SimModel/Rcm.h"
#include <set>
#include <map>

#ifdef _WINDOWS_PRODUCTION
#pragma managed(pop)
#endif

namespace SimModelNative
{

using namespace std;

BandwidthReductionTask::BandwidthReductionTask(Simulation * sim)
{
	_sim = sim;
	_lowerHalfBandWidth = 0;
	_upperHalfBandWidth = 0;
}

int BandwidthReductionTask::GetLowerHalfBandWidth()
{
	return _lowerHalfBandWidth;
}

int BandwidthReductionTask::GetUpperHalfBandWidth()
{
	return _upperHalfBandWidth;
}

void BandwidthReductionTask::ReorderDEVariables()
{
	//---- get variable dependencies
//	CacheRHSUsedVariables();
	vector<vector<bool> > dependencyMatrix = getDependencyMatrix();

	//---- get permutation of the variables indices required for minimal bandwidth
	Rcm rcm;
	vector<unsigned int> indicesPermutation = rcm.GenRcm(dependencyMatrix);

	////4debug only!
	//WriteRHSDependencyMatrix("C:\\VSS\\SimModel\\branches\\6.0\\Test\\RHSDepMatrix.txt");

	//---- reorder variables according to the obtained indices permutation
	reorderDEVariables(indicesPermutation);

	////4debug only!
	//WriteRHSDependencyMatrix("C:\\VSS\\SimModel\\branches\\6.0\\Test\\RHSDepMatrix_after.txt");

	//---- set half band width of the new system
	calculateHalfBandWidths();

}

void BandwidthReductionTask::calculateHalfBandWidths()
{
	_lowerHalfBandWidth = 0;
	_upperHalfBandWidth = 0;

	vector<Species *> & DE_Variables = _sim->DE_Variables();
	size_t numberOfVariables = DE_Variables.size();

	size_t i;

	for(i=0; i<numberOfVariables; i++)
	{
		Species * species = DE_Variables[i];

		int speciesLowerHalfBandWidth, speciesUpperHalfBandWidth;
		species->GetRHSUsedBandRange(speciesUpperHalfBandWidth, speciesLowerHalfBandWidth);

		_lowerHalfBandWidth = max(_lowerHalfBandWidth, speciesLowerHalfBandWidth);
		_upperHalfBandWidth = max(_upperHalfBandWidth, speciesUpperHalfBandWidth);
	}
}

void BandwidthReductionTask::reorderDEVariables(std::vector<unsigned int> indicesPermutation)
{
	vector<Species *> & DE_Variables = _sim->DE_Variables();
	unsigned int numberOfVariables = (unsigned int)DE_Variables.size();
	unsigned int i;

	//---- create mapping (<Old DE index>, <New DE index>) for all system variables
	map<unsigned int, unsigned int> indexMap;
	for(i=0; i<numberOfVariables; i++)
		indexMap[indicesPermutation[i]] = i;

	//---- reorder variables in the list
	vector<Species *> permutatedVariables; //temporary vector for saving variables in the new order

	for(i=0; i<numberOfVariables; i++)
		permutatedVariables.push_back(DE_Variables[indicesPermutation[i]]);

	DE_Variables.clear();
	
	for(i=0; i<numberOfVariables; i++)
		DE_Variables.push_back(permutatedVariables[i]);

	//---- change index of the variables
	//     also change index of variables used in the RHS
	for(i=0; i<numberOfVariables; i++)
	{
		Species * species = DE_Variables[i];

		species->SetODEIndex(i);

		//ChangeIndicesOfRHSUsedVariables only updates the CACHED indices of the used
		//variables. It does not adjust the indices of referenced variables in the RHS formulas
		species->ChangeIndicesOfRHSUsedVariables(indexMap);
	}

	//---- change index of referenced variables in all formulas
	for(int formulaIdx=0; formulaIdx<_sim->Formulas().size(); formulaIdx++)
		_sim->Formulas()[formulaIdx]->UpdateIndicesOfReferencedVariables();

	//change indices of changed species in switches (if any)
	for(i=0; i<(unsigned int)_sim->Switches().size(); i++)
		_sim->Switches()[i]->UpdateDEIndexOfTargetSpecies();
}

vector<vector<bool> > BandwidthReductionTask::getDependencyMatrix()
{
	vector<vector<bool> > dependencyMatrix;

	vector<Species *> & DE_Variables = _sim->DE_Variables();
	unsigned int numberOfVariables = (unsigned int)DE_Variables.size();

	for(size_t i=0; i<numberOfVariables; i++)
	{
		dependencyMatrix.push_back(DE_Variables[i]->RHSDependencyVector(numberOfVariables));
	}

	return dependencyMatrix;
}

}//.. end "namespace SimModelNative"
