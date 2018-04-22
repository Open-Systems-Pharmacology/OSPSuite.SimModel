#ifdef WIN32_PRODUCTION
#pragma managed(push,off)
#endif

#include "SimModel/Simulation.h"
#include "SimModel/DESolver.h"
#include "SimModel/CppODEExporter.h"
#include "SimModel/DESolverProperties.h"
#include "SimModel/SimulationTask.h"
#include "SimModel/OutputSchema.h"
#include "XMLWrapper/XMLHelper.h"
#include "SimModel/TableFormula.h"

#ifdef WIN32_PRODUCTION
#pragma managed(pop)
#endif

#ifdef WIN32
#pragma warning(disable:4996)
#endif

namespace SimModelNative
{

	using namespace std;

	void CppODEExporter::WriteCppCodeFromFile(const string & sFileName, const string & OutDir, bool FullMode, string &name, const vector<int> &outputIDs)
	{
		Simulation * Sim = NULL;

		try
		{
			Sim = new Simulation();
			Sim->LoadFromXMLFile(sFileName);

			WriteCppCode(Sim, OutDir, FullMode, name, outputIDs);

			delete Sim;
			Sim = NULL;
		}
		catch (...)
		{
			if (Sim != NULL)
				delete Sim;

			throw;
		}
	}

	void CppODEExporter::WriteCppCode(const string & sSimulationXML, const string & OutDir, bool FullMode, string &name, const vector<int> &outputIDs)
	{
		Simulation * Sim = NULL;

		try
		{
			Sim = new Simulation();
			Sim->LoadFromXMLString(sSimulationXML);

			WriteCppCode(Sim, OutDir, FullMode, name, outputIDs);

			delete Sim;
			Sim = NULL;
		}
		catch (...)
		{
			if (Sim != NULL)
				delete Sim;

			throw;
		}
	}

	// utility: preparation for every string stream -> output with maximum precision
	inline void formatStream(ostream &os) {
		os.precision(outputPrecision);
		os << scientific;
	}

	// utility: check if directory end with a file seperator or not
	bool endsWithFileSep(std::string const &dir) {
		const string posixFileSep("/");
		const string windowsFileSep("\\");
		if (dir.length() > 1) {
			return (dir.compare(dir.length() - 1, 1, posixFileSep) == 0 || dir.compare(dir.length() - 1, 1, windowsFileSep) == 0);
		}
		else {
			return false;
		}
	}

	// utility: compress ODE indices if variables are reduces
	void updateODEIndex(Simulation * sim)
	{
		for (int formulaIdx = 0; formulaIdx < sim->Formulas().size(); formulaIdx++)
			sim->Formulas()[formulaIdx]->UpdateIndicesOfReferencedVariables();
		for (int i = 0; i < sim->Switches().size(); i++)
			sim->Switches()[i]->UpdateDEIndexOfTargetSpecies();
	}

	// utility: restore original variables indices
	void restoreODEIndex(Simulation * sim, const map<int, Species*> &mapSpecies)
	{
		// restore species indices
		if (mapSpecies.size() != sim->GetODENumUnknowns())
		{
			for (auto iter = mapSpecies.begin(); iter != mapSpecies.end(); ++iter)
			{
				int oldIndex = iter->first;
				iter->second->DE_SetSpeciesIndex(oldIndex); //increments automatically
			}
			updateODEIndex(sim);
		}
	}

	// write local parameters according to set usedIDs
	void CppODEExporter::writeParamLocal(const set<int> & usedIDs, ostream & os)
	{
		for (auto iter = listParamLocal.begin(); iter != listParamLocal.end(); ++iter) // go through list instead of set to preserve ordering
		{
			if (usedIDs.find(iter->p->GetId()) != usedIDs.end()) // parameter is used
			{
				if (iter->def.empty() && iter->isUnique)
				{
					ostringstream ossTemp;
					Formula * f = iter->p->GetFormula();
					if (f != NULL) {
						f = f->clone()->RecursiveSimplify(); // use clone to prevent change in original formula
						if (ParameterFormula * p = dynamic_cast<ParameterFormula*>(f)) {
							p->WriteCppCode(ossTemp);
							iter->isUnique = false;
						}
						if (iter->isUnique)
						{
							if (iter->global)
							{
								if (iter->p->IsTable())
									continue;
								ossTemp << "P[" << dimFreeParams + vecSwitched.size() << "]";
								vecSwitched.push_back(iter->p->GetId());
							}
							else
							{
								ossTemp << "P_" << iter->p->GetId() << "_"; // "_" at the end to allow to search for low numbered parameter names
							}
							iter->p->SetShortUniqueNameForDESystem(ossTemp.str());
							ossTemp.clear();
							ossTemp.str("");
							Formula * backup = iter->p->GetFormula();
							iter->p->SetFormula(f);
							iter->p->WriteCppCode(ossTemp, !iter->global);
							iter->p->SetFormula(backup);
							iter->def = ossTemp.str();
						}
						else
							iter->p->SetShortUniqueNameForDESystem(ossTemp.str());
						delete f;
					}
					else {
						iter->def = std::to_string(iter->p->GetValue(NULL, 0, IGNORE_SCALEFACTOR));
					}
				}
				if (iter->isUnique)
				{
					if (!iter->global)
						os << iter->def;
					iter->uses++;
				}
			}
		}
		os << endl;
	}

	void CppODEExporter::WriteCppCode(Simulation * sim, const string & OutDir, bool FullMode, string &name, const vector<int> &outputIDs)
	{
		if (name.empty())
			name.assign("Standard");

		if (!sim->IsFinalized())
		{
			if (FullMode)
			{
				for (int i = 0; i < sim->Parameters().size(); i++)
					sim->Parameters()[i]->SetIsFixed(false);
			}
			sim->Finalize();
		}

		sim->MarkQuantitiesUsedBySwitches();

		////simplify parameters that could not be simplified earlier (in Finalize)
		////(e.g. parameters that depend on not fixed constant parameters)
		//sim->SimplifyObjects(false);

		// reset global variables
		dimFreeParams = 0;
		mapSpecies.clear();
		vecSwitched.clear();
		listParamLocal.clear();

		vector<int> vecParameters;
		vector<int> vecTables;
		map<int, formulaParameterInfo > mapFormulaParameters;
		
		vector<Observer*> obs;

		// buffer for complete file (endl flushes file buffer)
		ostringstream ossBufferExport;
		ostringstream ossBufferJacDense;
		ostringstream ossBufferJacSparse;
		ostringstream ossBufferJacConstants;
		ostringstream ossBufferJacParameters;
		formatStream(ossBufferExport);
		formatStream(ossBufferJacDense);
		formatStream(ossBufferJacSparse);
		formatStream(ossBufferJacConstants);
		formatStream(ossBufferJacParameters);

		ossBufferExport << "#define MODELNAME " << name << endl;
		ossBufferExport << "#include \"ModelDerived.hpp\"" << endl << endl;

		//ossBufferJacDense << "#define MODELNAME " << name << endl;
		//ossBufferJacDense << "#include \"ModelDerived.hpp\"" << endl << endl;

		//ossBufferJacSparse << "#define MODELNAME " << name << endl;
		//ossBufferJacSparse << "#include \"ModelDerived.hpp\"" << endl << endl;

		ossBufferJacParameters << "#define MODELNAME " << name << endl;

		// buffer for names and such
		ostringstream ossTemp;
		formatStream(ossTemp);

		set<int> usedIDs;
		set<int> empty;

		// collect necessary observers first, for optional reduction of number of states
		if (outputIDs.empty()) // output all
		{
			for (int i = 0; i < sim->Observers().size(); i++)
			{
				Observer *o = sim->Observers()[i];

				//ossBufferExport << std::endl << "#####" << o->GetId() << "   " << o->GetFormulaId() << "   " << o->GetFullName() << std::endl;
				if (o->GetId()) { //TODO WArum diese Abfrage
					o->AppendUsedVariables(usedIDs, empty);
					obs.push_back(o);
				}
			}
		}
		else
		{
			for (size_t i = 0; i < outputIDs.size(); i++) {
				Observer* o = sim->Observers().GetObjectById(outputIDs[i]);
				if (o != NULL)
				{
					o->AppendUsedVariables(usedIDs, empty);
					obs.push_back(o);
				}
				//			else // TODO: error
			}
		}

		ofstream outfile;
		try
		{
			// optionally reduce number of states, this may alter the number of states -> restore afterwards and in catch
			const int size = sim->GetODENumUnknowns();
			if (REDUCESPECIES) //TODO als parameter einführen
			{
				for (int i = 0; i < size; i++)
					sim->GetDEVariableFromIndex(i)->CacheRHSUsedVariables(empty);
				size_t newSize = usedIDs.size();
				size_t oldSize;
				do {
					oldSize = newSize;
					for (set<int>::iterator iter = usedIDs.begin(); iter != usedIDs.end(); ++iter)
						for (int i = 0; i < size; i++)
							if (sim->GetDEVariableFromIndex(*iter)->RHSDependsOn(i))
								usedIDs.insert(i);
					newSize = usedIDs.size();
				} while (oldSize != newSize);

				int newIndex = 0;
				for (set<int>::iterator iter = usedIDs.begin(); iter != usedIDs.end(); ++iter)
				{
					Species * s = sim->GetDEVariableFromIndex(*iter);
					s->DE_SetSpeciesIndex(newIndex); // automatically incremented in function
					mapSpecies.emplace(*iter, s);
				}

				updateODEIndex(sim);
			}
			else {
				for (int i = 0; i < size; i++)
					mapSpecies.emplace(i, sim->GetDEVariableFromIndex(i));
			}

			//---- write parameters
			vector <Simulation::HierarchicalFormulaObjectVector> leveledHierarchicalFormulaObjects =
				sim->GetLeveledHierarchicalFormulaObjects();

			// put in non-fixed DEVariables first to prevent an offset later on in the corresponding function
			for (map<int, Species*>::iterator iter = mapSpecies.begin(); iter != mapSpecies.end(); ++iter)
			{
				if (!iter->second->IsFixed())
					vecParameters.push_back(iter->second->GetId());
			}

			// non-fixed parameters and parameters that are switched are stored in one array:
			// free initial values ... free parameters ... switched parameters
			// determine offset for switched parameters up front

			for (int i = 0; i < sim->Parameters().size(); i++)
			{
				Parameter * p = sim->Parameters()[i];
				if (!(p->IsFixed() || p->IsChangedBySwitch() || p->IsTable())) //TODO
					++dimFreeParams;
				if (p->IsChangedBySwitch()) //
				{
					if (!p->IsFormulaEqualTo(NULL)) {
						Formula * f = p->GetFormula();
						mapFormulaParameters.insert(pair<int, formulaParameterInfo >(p->GetId(), formulaParameterInfo(f, 1)));
					}
				}
			}
			for (int i = 0; i < sim->Switches().size(); i++) {
				sim->Switches()[i]->AppendFormulaParameters(mapFormulaParameters);
			}

			//int paramIdx = 0;
			int switchIndex = sim->Switches().size(); // reserve space for oneTime flags
			set<int> postponedIDs;

			// collect and classify parameters
			for (unsigned int HLevelIdx = 0; HLevelIdx < leveledHierarchicalFormulaObjects.size(); HLevelIdx++)
			{
				vector <HierarchicalFormulaObject *> HFObjectsForLevel = leveledHierarchicalFormulaObjects[HLevelIdx];

				for (unsigned int HFObjectIdx = 0; HFObjectIdx < HFObjectsForLevel.size(); HFObjectIdx++)
				{
					HierarchicalFormulaObject * HObject = HFObjectsForLevel[HFObjectIdx];

					Parameter * param = dynamic_cast<Parameter *>(HObject);
					if (!param)
						continue;

					ossTemp.clear();
					ossTemp.str("");

					// expect non fixed parameters as input, afterwards
					// declare only parameters used that are not changed by switches or are table parameters as local
					if (param->IsTable())
					{
						// insert table name directly
						ossTemp << "T_" << param->GetId() << "(Time)";
						param->SetShortUniqueNameForDESystem(ossTemp.str());
						vecTables.push_back(param->GetId());
					}
					else if (!param->IsFixed()) {
						ossTemp << "P[" << vecParameters.size() << "]";
						param->SetShortUniqueNameForDESystem(ossTemp.str());
						vecParameters.push_back(param->GetId());
					}
					else if (param->IsChangedBySwitch())
					{
						// determine switch type
						map<int, formulaParameterInfo >::iterator iter = mapFormulaParameters.find(param->GetId());
						if (iter == mapFormulaParameters.end())
						{
							// insert parameter with switched value without explicit formula, they are inserted at the end of P
							ossTemp << "P[" << dimFreeParams + vecSwitched.size() << "]";
							param->SetShortUniqueNameForDESystem(ossTemp.str());
							vecSwitched.push_back(param->GetId());
						}
						else {
							// careful, order seems to be only true for initial formula
							// postpone writing the actual definition as long as possible,
							// i.e. till one of the other parameters depends on this one
							postponedIDs.insert(param->GetId());
						}
					}
					else {
						// slow, but necessary to push back formulaParameterDeclaration
						if (!postponedIDs.empty())
						{
							set<int> usedIDsParam;
							param->AppendUsedParameters(usedIDsParam);
							set<int> intersection;
							set_intersection(usedIDsParam.begin(), usedIDsParam.end(), postponedIDs.begin(), postponedIDs.end(),
								std::inserter(intersection, intersection.begin()));
							for (set<int>::iterator iter = intersection.begin(); iter != intersection.end(); ++iter)
							{
								map<int, formulaParameterInfo >::iterator iterMap = mapFormulaParameters.find(*iter);
								Parameter * postponedP = sim->Parameters().GetObjectById(*iter);
								// *** postponed switch parameter ***
								ossTemp << "P_s_" << postponedP->GetId() << "_"; // "_" at the end to allow to search for low numbered parameter names
								postponedP->SetShortUniqueNameForDESystem(ossTemp.str());
								// write switched assignment: case 0 for "isUsedAsValue"/Constants, case 1-n for formulas
								ossTemp.clear();
								ossTemp.str("");
								set<int> usedIDsFormulas;
								for (size_t i = 0; i < iterMap->second.vecFormulas.size(); i++) {
									iterMap->second.vecFormulas[i]->AppendUsedParameters(usedIDsFormulas);
								}
								writeParamLocal(usedIDsFormulas, ossTemp);
								ossTemp << "    double " << postponedP->GetShortUniqueName() << ";" << endl;
								iterMap->second.switchIndex = switchIndex;
								ossTemp << "    switch( S[" << iterMap->second.switchIndex << "]) {" << endl;
								++switchIndex;
								iterMap->second.valueIndex = (int) (dimFreeParams + vecSwitched.size()); // explicit cast for size_t -> int
								ossTemp << "        default:" << endl; // disables warning, might be dangerous if index is wrong
								ossTemp << "        case 0: " << postponedP->GetShortUniqueName() << " = " << "P[" << iterMap->second.valueIndex << "]; break;" << endl;
								vecSwitched.push_back(postponedP->GetId());
								for (size_t i = 0; i < iterMap->second.vecFormulas.size(); i++) {
									ossTemp << "        case " << i + 1 << ": ";
									ossTemp << postponedP->GetShortUniqueName() << " = ";
									iterMap->second.vecFormulas[i]->WriteCppCode(ossTemp);
									ossTemp << "; break;" << endl;
								}
								ossTemp << "    }" << endl;
								// add to local dependency list
								listParamLocal.push_back(ParamLocal(postponedP, ossTemp.str(), false));
								ossTemp.clear();
								ossTemp.str("");
								postponedIDs.erase(*iter);
								////////////////////////////////
							}
						}

						// local parameter pool
						static int no = 0;
						std::set<int> dep;
						param->AppendUsedVariables(dep, empty);
						bool global = dep.empty(); //TODO check was if von der Zeit abhängig
						if (global) {
							dep.clear();
							param->AppendUsedParameters(dep);
							global &= dep.find(0) == dep.end();
							for (auto iter = mapFormulaParameters.begin(); global && iter != mapFormulaParameters.end(); ++iter)
								global &= dep.find(iter->first) == dep.end();
						}
						listParamLocal.push_back(ParamLocal(param, "", global));
					}
				}
			}

			// insert remaining postponed parameters
			for (set<int>::iterator iter = postponedIDs.begin(); iter != postponedIDs.end(); ++iter)
			{
				ossTemp.clear();
				ossTemp.str("");
				map<int, formulaParameterInfo >::iterator iterMap = mapFormulaParameters.find(*iter);
				Parameter * postponedP = sim->Parameters().GetObjectById(*iter);
				// *** postponed switch parameter ***
				ossTemp << "P_s_" << postponedP->GetId() << "_"; // "_" at the end to allow to search for low numbered parameter names
				postponedP->SetShortUniqueNameForDESystem(ossTemp.str());
				// write switched assignment: case 0 for "isUsedAsValue"/Constants, case 1-n for formulas
				ossTemp.clear();
				ossTemp.str("");
				set<int> usedIDs;
				for (size_t i = 0; i < iterMap->second.vecFormulas.size(); i++) {
					iterMap->second.vecFormulas[i]->AppendUsedParameters(usedIDs);
				}
				writeParamLocal(usedIDs, ossTemp);
				ossTemp << "    double " << postponedP->GetShortUniqueName() << ";" << endl;
				iterMap->second.switchIndex = switchIndex;
				ossTemp << "    switch( S[" << iterMap->second.switchIndex << "]) {" << endl;
				++switchIndex;
				iterMap->second.valueIndex = (int)(dimFreeParams + vecSwitched.size()); // explicit cast for size_t -> int
				ossTemp << "        default:" << endl; // disables warning, might be dangerous if index is wrong
				ossTemp << "        case 0: " << postponedP->GetShortUniqueName() << " = " << "P[" << iterMap->second.valueIndex << "]; break;" << endl;
				vecSwitched.push_back(postponedP->GetId());
				for (size_t i = 0; i < iterMap->second.vecFormulas.size(); i++) {
					ossTemp << "        case " << i + 1 << ": ";
					ossTemp << postponedP->GetShortUniqueName() << " = ";
					iterMap->second.vecFormulas[i]->WriteCppCode(ossTemp);
					ossTemp << "; break;" << endl;
				}
				ossTemp << "    }" << endl;
				// add to local dependency list
				listParamLocal.push_back(ParamLocal(postponedP, ossTemp.str(), false));
			}

			WriteODERHSFunction(sim, ossBufferExport);
			WriteODESwitches(sim, mapFormulaParameters, outputIDs, ossBufferExport);
			WriteODETableParameters(sim, vecTables, ossBufferExport);

			ossBufferExport << "void CLASSNAME(MODELNAME)::ODEObservers(const double Time, const double *__restrict__ y, const double *__restrict__ P, const unsigned int *__restrict__ S, double *__restrict__ obs) {" << endl;

			// collect dependencies on parameters
			usedIDs.clear();
			for (size_t i = 0; i < obs.size(); i++)
			{
				obs[i]->AppendUsedParameters(usedIDs);
			}

			writeParamLocal(usedIDs, ossBufferExport);
			for (size_t i = 0; i < obs.size(); i++)
			{
				ossBufferExport << "    obs[" << i << "] = ";
				Formula *f = obs[i]->getValueFormula();
				if (f == NULL)
					ossBufferExport << obs[i]->GetValue(NULL, 0.0, IGNORE_SCALEFACTOR);
				else
					obs[i]->getValueFormula()->WriteCppCode(ossBufferExport);
				ossBufferExport << ";" << endl;
			}
			ossBufferExport << "}" << endl << endl;

			// insert switch parameters at the end
			vecParameters.insert(vecParameters.end(), vecSwitched.begin(), vecSwitched.end());

			// write Jacobians first to determine nnz
			// use seperate buffers for additional files, determining derivatives is mainly common work though
			WriteODEJacobian(sim, ossBufferJacDense, ossBufferJacSparse, ossBufferJacConstants);
			WriteSensJacobian(sim, vecParameters, ossBufferJacDense, ossBufferJacSparse, ossBufferJacConstants);

			ossBufferExport << "void CLASSNAME(MODELNAME)::ODEInitialParameters(double *__restrict__ P) {" << endl;
			for (list<ParamLocal>::iterator iter = listParamLocal.begin(); iter != listParamLocal.end(); ++iter)
				if (iter->global && iter->uses && iter->isUnique)
					ossBufferExport << "    " << iter->def;
/*			map<int, std::string> sortedMapNewP;
			for (map<std::string, ParameterFormula * >::iterator iter = mapNewP.begin(); iter != mapNewP.end(); ++iter)
			{
				ossTemp.clear();
				ossTemp.str("");
				ossTemp << "    ";
				iter->second->WriteCppCode(ossTemp); // parameter name
				ossTemp << " = " << iter->first << ";" << endl;
				sortedMapNewP.emplace(iter->second->GetId(), ossTemp.str());
			}
			for (map<int, std::string>::iterator iter = sortedMapNewP.begin(); iter != sortedMapNewP.end(); ++iter) {
				ossBufferExport << iter->second;
			}*/
			ossBufferExport << "}" << endl << endl;

			WriteODEParameterAndObserver(sim, vecParameters, mapFormulaParameters, ossBufferJacConstants.str(), obs, ossBufferExport);

			ossBufferExport << "#define CONVERT(x) #x" << endl;
			ossBufferExport << "#define STRING(x) CONVERT(x)" << endl;
			ossBufferExport << "CLASSNAME(MODELNAME)::CLASSNAME(MODELNAME)() : Model(STRING(MODELNAME), _dimY(), _dimP(), _dimP_free(), _dimS(), _dimOutputTimePoints()," << endl;
			ossBufferExport << "                                              _dimImplicitSwitches(), _dimObservers(), _dimNNZ(), _dimNNZ_fp(), _dimNNZ_ox(), _dimNNZ_op()," << endl;
			ossBufferExport << "                                              _P_map(), _O_map(), _Y_map(), _Y_sca(), _P_init(), _S_init(), _outputTimePoints()," << endl;
			ossBufferExport << "                                              _jacRows(), _jacCols(), _jacRows_fp(), _jacCols_fp(), _jacRows_op(), _jacCols_op()," << endl;
			ossBufferExport << "                                              _jacRows_op(), _jacCols_op(), _hash(), _explicitSwitches) {" << endl;
			ossBufferExport << "}" << endl;
			ossBufferExport << "static CLASSNAME(MODELNAME) MODELNAME;" << endl;

			// buffered output finished, write buffer
			string filename;
			string filenameBase;
			if (!endsWithFileSep(OutDir))
				filenameBase = OutDir + "/" + name;
			else
				filenameBase = OutDir + name;

			filename = filenameBase + ".cpp";// +"Base.cpp";
			outfile.open(filename.c_str());
			outfile << ossBufferExport.str();
			//outfile.close();
			//filename = filenameBase + "JacDense.cpp";
			//outfile.open(filename.c_str());
			outfile << ossBufferJacDense.str();
			//outfile.close();
			//filename = filenameBase + "JacSparse.cpp";
			//outfile.open(filename.c_str());
			outfile << ossBufferJacSparse.str();
		}
		catch (exception e)
		{
			// catch exception to tidy up and restore the ODEIndex, but do nothing
		}
		if (outfile.is_open())
			outfile.close();
		restoreODEIndex(sim, mapSpecies);
	}

	void CppODEExporter::WriteODEParameterAndObserver(Simulation * sim, vector<int> &vecParameters,
		const std::map<int, formulaParameterInfo > & formulaParameterIDs, const std::string &jacConstants,
		const std::vector<Observer*> &obs, ostream & os) {

		// ODE initial values, dont use GetDEInitialValues, because some might be defined as a formula
		os << "void CLASSNAME(MODELNAME)::ODEInitialValues(double *__restrict__ y, double *__restrict__ P, unsigned int *__restrict__ S) {" << endl;
		vector<double> vecInitialValues;
		set<int> usedIDs;
		for (map<int, Species*>::iterator iter = mapSpecies.begin(); iter != mapSpecies.end(); ++iter)
		{
			Formula * f = iter->second->GetInitialFormula();
			if (iter->second->IsFixed() && f != NULL)
				f->AppendUsedParameters(usedIDs);
		}
		writeParamLocal(usedIDs, os);

		int indexP = 0;
		vecInitialValues.resize(mapSpecies.size());
		for (map<int, Species*>::iterator iter = mapSpecies.begin(); iter != mapSpecies.end(); ++iter)
		{
			Species * spec = iter->second;
			if (spec->IsFixed()) {
				Formula * f = spec->GetInitialFormula();
				os << "    y[" << spec->GetODEIndex() << "] = ";
				if (f)
				{
					vecInitialValues[spec->GetODEIndex()] = f->DE_Compute(NULL, sim->GetStartTime(), IGNORE_SCALEFACTOR);
					f->WriteCppCode(os);
				}
				else
				{
					double value = spec->GetInitialValue(NULL, sim->GetStartTime());
					vecInitialValues[spec->GetODEIndex()] = value;
					os << value;
				}
				os << "; // " << spec->GetFullName() << endl;
			}
			else {
				os << "    y[" << spec->GetODEIndex() << "] = P[ " << indexP << "]; // " << spec->GetFullName() << endl;
				++indexP;
			}
		}

		os << endl << "}" << endl << endl;

		// initialize on first use and anonymous namespace
		os << "namespace {" << endl;
		os << "unsigned int _dimY() { static const unsigned int dimY = " << mapSpecies.size() << "; return dimY; }" << endl << endl;

		// write output times vector
		vector <OutputTimePoint> outputTimePoints = SimulationTask::OutputTimePoints(sim);

		int dimOutput = 1; // startTime
		os << "const double* _outputTimePoints() { static const double outputTimePoints[] = { " << sim->GetStartTime();
		for (size_t timeStepIdx = 0; timeStepIdx < outputTimePoints.size(); timeStepIdx++)
		{
			OutputTimePoint outTimePoint = outputTimePoints[timeStepIdx];

			if (outTimePoint.SaveSystemSolution())
			{
				os << ", " << outTimePoint.Time();
				if (timeStepIdx % 100 == 0)
					os << endl << "                                                                               ";
				++dimOutput;
			}
		}
		os << " }; return outputTimePoints; }" << endl;
		os << "unsigned int _dimOutputTimePoints() { static const unsigned int dimOutputTimePoints = " << dimOutput << "; return dimOutputTimePoints; }" << endl << endl;

		os << "const unsigned int* _positiveStateIndices() { static const unsigned int positiveStateIndices[] = { ";
		int no = 0;
		for (map<int, Species*>::iterator iter = mapSpecies.begin(); iter != mapSpecies.end(); ++iter)
		{
			if (!iter->second->NegativeValuesAllowed()) {
				if (no != 0)
				{
					os << ", ";
					if (no % 100 == 0)
						os << endl << "                                                                                 ";
				}
				os << iter->second->GetODEIndex();
				++no;
			}
		}
		os << "}; return positiveStateIndices; }" << endl;
		os << "const unsigned int _dimPositiveStateIndices() { static const unsigned int dimPositiveStateIndices = " << no << "; return dimPositiveStateIndices; }" << endl << endl;

		//get unique application start times for the simulation
		std::set<double> fixedSwitches = sim->GetSwitchTimes();

		std::vector<Formula*> vecExplicit;
		std::vector<Formula*> vecImplicit;
		for (int i = 0; i < sim->Switches().size(); i++) {
			sim->Switches()[i]->SwitchFormulaFromComparisonFormula(vecExplicit, vecImplicit);
		}

		os << "std::set<double> _explicitSwitches(const double *P) {" << endl;
		os << "    std::set<double> s;" << endl;

		// collect dependencies of formulas
		usedIDs.clear();
		for (size_t i = 0; i < vecExplicit.size(); i++)
			vecExplicit[i]->AppendUsedParameters(usedIDs);

		writeParamLocal(usedIDs, os);

		// add formulas to be evaluated
		for (size_t i = 0; i < vecExplicit.size(); i++) {
			// constants can be merged with fixed switches
			if (vecExplicit[i]->IsConstant(CONSTANT_CURRENT_RUN))
			{
				fixedSwitches.emplace(vecExplicit[i]->DE_Compute(NULL, 0.0, USE_SCALEFACTOR));
			}
			else
			{
				os << "    s.insert(";
				vecExplicit[i]->WriteCppCode(os);
				os << ");" << endl;
			}
		}

		// add fixed switches
		for (auto sIter = fixedSwitches.begin(); sIter != fixedSwitches.end(); ++sIter)
		{
			os << "    s.insert(" << *sIter << ");" << endl;
		}

		os << "    return s;" << endl;
		os << "}" << endl << endl;

		// TODO: fix
		os << "unsigned int _dimImplicitSwitches() { static const unsigned int dimImplicitSwitches = " << "0; return dimImplicitSwitches; }" << endl;

		// ODE initial values and initial time to evaluate global parameter initial values
		//double * InitialValues = sim->GetDEInitialValues();

		os << "const double *_P_init() { static const double P_init[] = { ";
		for (size_t i = 0; i < vecParameters.size(); i++) {
			if (i != 0) {
				os << ", ";
				if (i % 100 == 0)
					os << endl << "                                                           ";
			}

			double value = NAN;
			Parameter* p = sim->Parameters().GetObjectById(vecParameters[i]);
			if (p != NULL) {
				value = p->GetValue(vecInitialValues.data(), sim->GetStartTime(), IGNORE_SCALEFACTOR);
			}
			else {
				Species* s = sim->SpeciesList().GetObjectById(vecParameters[i]);
				value = vecInitialValues[s->GetODEIndex()];
			}
			if (::isnan(value)) // explicit qualifier "::" to prevent ambigiouty with std::isnan
				os << "0.0"; //NAN
			else
				os << value;
		}
		for (size_t i = 0; i < vecParameters.size(); i++)
		{
			if (i % 100 == 0)
				os << endl << "                                                           ";
			os << ", 0.0";
		}
		os << "}; return P_init; }" << endl;

		// TODO: use real map instead to improve ODEParamValues?
		os << "const unsigned int *_P_map() { static const unsigned int P_map[] = {";

		if (!vecParameters.empty()) {
			os << vecParameters[0]; // no comma at the beginning
			for (size_t i = 1; i < vecParameters.size(); i++) {
				os << ", " << vecParameters[i];
				if (i % 100 == 0)
					os << endl << "                                                  ";
			}
		}
		os << "}; return P_map; }" << endl << endl;

		os << "const unsigned int *_O_map() { static const unsigned int O_map[] = {";
		if (!obs.empty()) {
			os << obs[0]->GetId();  // no comma at the beginning
			for (size_t i = 1; i < obs.size(); i++) {
				os << ", " << obs[i]->GetId();
				if (i % 100 == 0)
					os << endl << "                                                  ";
			}
		}
		os << "}; return O_map; }" << endl << endl;

		os << "const unsigned int *_Y_map() { static const unsigned int Y_map[] = {";
		if (!mapSpecies.empty()) {
			map<int, Species*>::iterator iter = mapSpecies.begin();
			os << iter->second->GetId(); // no comma at the beginning
			++iter;
			int i = 0;
			for (; iter != mapSpecies.end(); ++iter)
			{
				os << ", " << iter->second->GetId();
				if ((++i) % 100 == 0)
					os << endl << "                                                  ";
			}
		}
		os << "}; return Y_map; }" << endl << endl;

		os << "const double *_Y_sca() { static const double Y_sca[] = {";
		if (!mapSpecies.empty()) {
			map<int, Species*>::iterator iter = mapSpecies.begin();
			os << iter->second->GetODEScaleFactor(); // no comma at the beginning
			++iter;
			int i = 0;
			for (; iter != mapSpecies.end(); ++iter)
			{
				os << ", " << iter->second->GetODEScaleFactor();
				if ((++i) % 100 == 0)
					os << endl << "                                                  ";
			}
		}
		os << "}; return Y_sca; }" << endl << endl;

		os << "const unsigned int *_S_init() { static const unsigned int S_init[] = {";
		for (int i = 0; i < sim->Switches().size(); i++) {
			if (i != 0)
			{
				os << ", ";
				if (i % 100 == 0)
					os << endl << "                                                    ";
			}
			os << "1";
		}
		os << endl << "                                                    ";
		for (std::map<int, formulaParameterInfo >::const_iterator iter = formulaParameterIDs.begin(); iter != formulaParameterIDs.end(); ++iter)
		{
			os << ", " << iter->second.initialIndex;
		}
		os << "}; return S_init; }" << endl << endl;

		//os << "// " << vecParameters.size() << "   " << offsetNewP << endl;
		os << "unsigned int _dimP() { static const unsigned int dimP = " << vecParameters.size() << "; return dimP; }" << endl; //vecParameters.size() + 
		os << "unsigned int _dimP_free() { static const unsigned int dimP_free = " << dimFreeParams << "; return dimP_free; }" << endl;
		os << "unsigned int _dimS() { static const unsigned int dimS = " << sim->Switches().size() + formulaParameterIDs.size() << "; return dimS; }" << endl;

		// calc results for selected observers
		os << "unsigned int _dimObservers() { static const unsigned int dimObservers = " << obs.size() << "; return dimObservers; }" << endl << endl;

		// calculate hash from xml_string, free parameters and selected observers
		std::hash<std::string> hash_string;
		ostringstream ossTemp;
		for (size_t i = 0; i < dimFreeParams; i++)
			ossTemp << ", " << vecParameters[i];
		for (size_t i = 1; i < obs.size(); i++)
			ossTemp << ", " << obs[i]->GetId();
		size_t simHash = hash_string(sim->GetSimulationXMLString() + ossTemp.str());
		os << "size_t _hash() { static const size_t hash = " << simHash << "U; return hash; }" << endl;

		os << jacConstants;

		os << "}" << endl << endl;

		// function ODEOptions
		const DESolverProperties & solverProperties = sim->GetSolver().GetSolverProperties();
		os << "void CLASSNAME(MODELNAME)::ODEOptions(double &relTol, double &absTol, double &hInit, double &hMin, double &hMax, long int &maxSteps, bool &useJac, const double *__restrict__ P) {" << endl;
		os << "    relTol = " << solverProperties.GetRelTol() << ";" << endl;
		os << "    absTol = " << solverProperties.GetAbsTol() << ";" << endl;
		os << "    hInit  = " << solverProperties.GetH0() << ";" << endl;
		os << "    hMin   = " << solverProperties.GetHMin() << ";" << endl;
		os << "    hMax   = " << solverProperties.GetHMax() << ";" << endl;
		os << "    maxSteps = " << solverProperties.GetMxStep() << ";" << endl;
		os << "    useJac = " << solverProperties.GetUseJacobian() << ";" << endl;
		//TODO
		//os << "                 'MaxOrder'," << 5 << ", ..." << endl;//solverProperties.GetMaxOrd()<<", ..."<<endl;
		os << "}" << endl << endl;
	}

	void CppODEExporter::WriteODERHSFunction(Simulation * sim, ostream & os)
	{
		os << "void CLASSNAME(MODELNAME)::ODERHSFunction(const double Time, const double *__restrict__ y, const double *__restrict__ P, const unsigned int *__restrict__ S, double *__restrict__ dy) {" << endl;

		// collect dependencies on parameters
		set<int> usedIDs;
		for (map<int, Species*>::iterator iter = mapSpecies.begin(); iter != mapSpecies.end(); ++iter)
			iter->second->AppendUsedParameters(usedIDs);

		writeParamLocal(usedIDs, os);

		//---- write ode equations
		for (auto iter = mapSpecies.begin(); iter != mapSpecies.end(); ++iter)
			iter->second->WriteCppCode(os);

		os << "}" << endl << endl;
	}

	void CppODEExporter::WriteODESwitches(Simulation * sim,
		const map<int, formulaParameterInfo > & formulaParameterIDs, const std::vector<int> &outputIDs, ostream & os)
	{
		int idFcnPtr = 0;
		ostringstream osFcn;
		formatStream(osFcn);

		// explicit switches
		os << "bool CLASSNAME(MODELNAME)::ODEExplicitSwitch(const double Time, double *__restrict__ y, double *__restrict__ P, unsigned int *__restrict__ S) {" << endl;
		os << "    bool switchUpdate = false;" << endl;

		// collect dependencies on parameters
		set<int> usedIDs;
		for (int i = 0; i < sim->Switches().size(); i++) {
			sim->Switches()[i]->AppendUsedParameters(usedIDs);
		}

		writeParamLocal(usedIDs, os);

		for (int i = 0; i < sim->Switches().size(); i++)
			sim->Switches()[i]->WriteCppCode(i, formulaParameterIDs, usedIDs, os);

		os << "    return switchUpdate;" << endl;
		os << "}" << endl << endl;

		os << osFcn.str();

		os << "void CLASSNAME(MODELNAME)::ODEImplicitSwitch(const double Time, double *__restrict__ y, double *__restrict__ P, unsigned int *__restrict__ S, double *__restrict__ gout) {" << endl;
		os << "}" << endl << endl;

		// explicit instantiations
		//os << "template void ODERHSFunction<double>(const double& Time, const double* y, const double* P, double* dy);" << endl;
		//os << "#ifdef DER_CPPAD" << endl;
		//os << "template void ODERHSFunction<CppAD::AD<double> >(const CppAD::AD<double>& Time, const CppAD::AD<double>* y, const double* P, CppAD::AD<double>* dy);" << endl;
		//os << "#endif" << endl;
	}

	void CppODEExporter::WriteODETableParameters(Simulation * sim, const vector<int> &vecTables, ostream & os)
	{
		//vector <Parameter *> tableParameters;

		////---- assign table formula to every table parameter
		//for (int i = 0; i < sim->Parameters().size(); i++)
		//{
		//	Parameter * param = sim->Parameters()[i];
		//	if (!param->IsTable())
		//		continue;

		//	//cache table parameter
		//	tableParameters.push_back(param);
		//}

		////---- now write table functions itself
		//for (size_t i = 0; i < tableParameters.size(); i++)
		//	tableParameters[i]->WriteTableFunctionForCpp(os);

		for (auto iter = vecTables.begin(); iter != vecTables.end(); ++iter)
			sim->Parameters().GetObjectById(*iter)->WriteTableFunctionForCpp(os);
	}

	void CppODEExporter::WriteODEJacobian(Simulation * sim, ostream & dense, ostream & sparse, std::ostream & osConsts)
	{
		ostringstream ossFormula;
		formatStream(ossFormula);
		vector<JacFormula> vecJacFormulas;
		const size_t noRows = mapSpecies.size();
		const size_t noCols = mapSpecies.size();
		vecJacFormulas.reserve(noRows*noCols);

		set<int> usedIDs; // collect dependencies on parameters

		for (size_t col = 0; col < noCols; col++) // ! col wise !
		{
			size_t row = 0;
			for (map<int, Species*>::iterator iter = mapSpecies.begin(); iter != mapSpecies.end(); ++iter)
			{
				Formula *f = iter->second->DE_Jacobian((int)col);// species indices are stored as int -> explicit cast

				f = f->RecursiveSimplify();

				bool isZero = f->IsZero();
				// even the dense Jacobian needs non-zero elements only
				if (!isZero) {
					f->AppendUsedParameters(usedIDs);

					ossFormula.clear();
					ossFormula.str("");
					f->WriteCppCode(ossFormula);
					vecJacFormulas.push_back(JacFormula(row, col, ossFormula.str()));
				}
				++row;

				// clean up
				delete f;
			}
		}

		// nzz structure, write rows directly, use ossFormula to cache column positions
		WriteSparseStructure("", usedIDs, vecJacFormulas, noRows, noCols, dense, sparse, osConsts);
	}

	void CppODEExporter::WriteSensJacobian(Simulation * sim, vector<int> & vecParameters, ostream & dense, ostream & sparse, std::ostream & osConsts)
	{
		// fp
		ostringstream ossFormula;
		formatStream(ossFormula);
		vector<JacFormula> vecJacFormulas;
		size_t noCols = dimFreeParams;
		size_t noRows = mapSpecies.size();
		vecJacFormulas.reserve(noCols*noRows);
		set<int> usedIDs; // collect dependencies on parameters

		for (size_t col = 0; col < noCols; col++) // ! col wise !
		{
			size_t row = 0;
			for (map<int, Species*>::iterator iter = mapSpecies.begin(); iter != mapSpecies.end(); ++iter)
			{
				Formula *f = iter->second->DE_Jacobian(-vecParameters[col]);

				f = f->RecursiveSimplify();

				bool isZero = f->IsZero();
				// even the dense Jacobian needs non-zero elements only
				if (!isZero) {
					f->AppendUsedParameters(usedIDs);
					//if (CheckNewParameter(&f, mapNewP))
					//	f->InsertNewParameters(mapNewP);

					ossFormula.clear();
					ossFormula.str("");
					f->WriteCppCode(ossFormula);
					vecJacFormulas.push_back(JacFormula(row, col, ossFormula.str()));
				}
				++row;

				// clean up
				delete f;
			}
		}
		
		// nzz structure, write rows directly, use ossFormula to cache column positions
		WriteSparseStructure("_fp", usedIDs, vecJacFormulas, noRows, noCols, dense, sparse, osConsts);

		// nzz structure, write rows directly, use ossFormula to cache column positions
		WriteSparseStructure("_ox", usedIDs, vecJacFormulas, noRows, noCols, dense, sparse, osConsts);

		// nzz structure, write rows directly, use ossFormula to cache column positions
		WriteSparseStructure("_op", usedIDs, vecJacFormulas, noRows, noCols, dense, sparse, osConsts);
	}

	void CppODEExporter::WriteSparseStructure(const std::string &nameExt, const std::set<int> usedIDs, const std::vector<JacFormula> &vecJacFormulas,
		const size_t noRows, const size_t noCols, std::ostream & dense, std::ostream & sparse, std::ostream & osConsts) {
		ostringstream ossFormula;
		formatStream(ossFormula);

		// write dense matrix
		dense << "void CLASSNAME(MODELNAME)::ODEJacDense" << nameExt << "(const double Time, const double *__restrict__ y, const double *__restrict__ P, const unsigned int *__restrict__ S, double *__restrict__ J) {" << endl;
		writeParamLocal(usedIDs, dense);
		for (vector<JacFormula>::const_iterator iter = vecJacFormulas.begin(); iter != vecJacFormulas.end(); ++iter)
			dense << "    J[" << iter->col*noRows + iter->row << "] = " << iter->str << ";" << endl;
		dense << "}" << endl << endl;

		// write sparse matrix
		sparse << "void CLASSNAME(MODELNAME)::ODEJacSparse" << nameExt << "(const double Time, const double *__restrict__ y, const double *__restrict__ P, const unsigned int *__restrict__ S, double *__restrict__ J) {" << endl;
		writeParamLocal(usedIDs, sparse);
		for (size_t i = 0; i < vecJacFormulas.size(); ++i)
			sparse << "    J[" << i << "] = " << vecJacFormulas[i].str << ";" << endl;
		sparse << "}" << endl << endl;

		// write sparse structure
		osConsts << "int _dimNNZ" << nameExt << "() { static int dimNNZ" << nameExt << " = " << vecJacFormulas.size() << "; return dimNNZ" << nameExt << "; }" << endl;

		std::vector<int> sparseI(noCols+1);
		sparseI[0] = 0;
		for (size_t i = 0; i < vecJacFormulas.size(); i++)
			++sparseI[vecJacFormulas[i].col+1];
		std::partial_sum(sparseI.begin(), sparseI.end(), sparseI.begin());

		osConsts << "const int* _jacRows" << nameExt << "() { static const int jacRows" << nameExt << "[] = {";
		int index = 0;
		for (size_t i=0; i < vecJacFormulas.size();) // increment below
		{
			if (i==0)
				osConsts << vecJacFormulas[i].row; // first without comma
			else
				osConsts << ", " << vecJacFormulas[i].row;
			if ((++i) % 100 == 0)
				osConsts << endl << "                                                      ";
		}
		osConsts << "}; return jacRows" << nameExt << "; }" << endl;

		osConsts << "const int* _jacCols" << nameExt << "() { static const int jacCols" << nameExt << "[] = {";
		for (size_t i = 0; i < sparseI.size();)  // increment below
		{
			if (i == 0)
				osConsts << sparseI[i]; // first without comma
			else
				osConsts << ", " << sparseI[i];
			if ((++i) % 100 == 0)
				osConsts << endl << "                                                      ";
		}
		osConsts << "}; return jacCols" << nameExt << "; }" << endl;
	}

}//.. end "namespace SimModelNative"
