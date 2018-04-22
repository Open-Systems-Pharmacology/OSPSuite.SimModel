//..begin "Ifdef"
#ifndef _CppODEExporter_H_
#define _CppODEExporter_H_
//..end "Ifdef"

#include <iostream>
#include <ostream>
#include <sstream>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <numeric>
#include <iterator>

const bool REDUCESPECIES = false; // false for BCS

// precision should be 17 for double: http://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html
// defined as dbl::max_digits10 in C++11
#if __cplusplus < 201103L
const int outputPrecision = 17;
#else
#include <limits>
const int outputPrecision = std::numeric_limits<double>::max_digits10;
#endif

#include "GlobalConstants.h"
#include "Simulation.h"
#include "FormulaChange.h"
#include "ParameterFormula.h"
#include "Species.h"

namespace SimModelNative
{

struct ParamLocal
{
	Parameter * p;
	std::string def;
	int uses;
	bool global;
	bool isUnique;
	ParamLocal(Parameter * _p, std::string _def, bool _global) : p(_p), def(_def), uses(0), global(_global), isUnique(true) {}
};

// structure to collect info for both sparse and dense jacobian
struct JacFormula {
	size_t row;
	size_t col;
	std::string str;
	JacFormula(size_t _row, size_t _col, std::string _str) : row(_row), col(_col), str(_str) {}
};

class CppODEExporter
{
	public:
		SIM_EXPORT void WriteCppCodeFromFile(const std::string & sFileName, const std::string & OutDir, bool FullMode, std::string &name = std::string(""), const std::vector<int> &outputIDs = std::vector<int>());
		SIM_EXPORT void WriteCppCode(const std::string & sSimulationXML, const std::string & OutDir, bool FullMode, std::string &name = std::string(""), const std::vector<int> &outputIDs = std::vector<int>());
		SIM_EXPORT void WriteCppCode(Simulation * sim, const std::string & OutDir, bool FullMode, std::string &name = std::string(""), const std::vector<int> &outputIDs = std::vector<int>());

	private:
		size_t dimFreeParams;
		std::map<int, Species*> mapSpecies;
		std::vector<int> vecSwitched;
		std::list< ParamLocal > listParamLocal;
		void writeParamLocal(const std::set<int> & usedIDs, std::ostream & os);

		void WriteODEParameterAndObserver(Simulation * sim, std::vector<int> &vecParameters, const std::map<int, formulaParameterInfo > & formulaParameterIDs, const std::string &jacConstants, const std::vector<Observer*> &obs, std::ostream & os);
		void WriteODERHSFunction(Simulation * sim, std::ostream & os);
		void WriteODESwitches(Simulation * sim, const std::map<int, formulaParameterInfo > & formulaParameterIDs, const std::vector<int> &outputIDs, std::ostream & os);
		void WriteODETableParameters(Simulation * sim, const std::vector<int> &vecTables, std::ostream & os);
		void WriteODEJacobian(Simulation * sim, std::ostream & osDense, std::ostream & osSparse, std::ostream & osConsts);
		void WriteSensJacobian(Simulation * sim, std::vector<int> & vecParameters, std::ostream & dense, std::ostream & sparse, std::ostream & osConsts);
		void WriteSparseStructure(const std::string &nameExt, const std::set<int> usedIDs, const std::vector<JacFormula> &vecJacFormulas, const size_t noRows, const size_t noCols, std::ostream & osDense, std::ostream & osSparse, std::ostream & osConsts);
	};


}//.. end "namespace SimModelNative"

//..begin "Endif"
#endif //_CppODEExporter_H_
//..end "Endif"
