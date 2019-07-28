#include "SimModel/Simulation.h"
#include "SimModel/DESolver.h"
#include "SimModel/MatlabODEExporter.h"
#include "SimModel/DESolverProperties.h"
#include "SimModel/SimulationTask.h"
#include "SimModel/OutputSchema.h"
#include "SimModel/TableFormula.h"

#ifdef _WINDOWS
#pragma warning(disable:4996)
#endif

namespace SimModelNative
{

using namespace std;

void MatlabODEExporter::WriteMatlabCodeFromFile (const string & sFileName, const string & OutDir, bool FullMode)
{
	Simulation * Sim = NULL;
	
	try
	{
		Sim = new Simulation();
		Sim->LoadFromXMLFile(sFileName);
		
		WriteMatlabCode(Sim, OutDir, FullMode);
		
		delete Sim;
		Sim = NULL;
	}
	catch(...)
	{
		if (Sim != NULL)
			delete Sim;
		
		throw;
	}
}

void MatlabODEExporter::WriteMatlabCode (const string & sSimulationXML, const string & OutDir, bool FullMode)
{
	Simulation * Sim = NULL;
	
	try
	{
		Sim = new Simulation();
		Sim->LoadFromXMLString(sSimulationXML);
		
		WriteMatlabCode(Sim, OutDir, FullMode);
		
		delete Sim;
		Sim = NULL;
	}
	catch(...)
	{
		if (Sim != NULL)
			delete Sim;
		
		throw;
	}
}

void MatlabODEExporter::WriteMatlabCode (Simulation * sim, const string & OutDir, bool FullMode)
{
	if (!sim->IsFinalized())
	{
		if (FullMode)
		{
			for (int i=0;i<sim->Parameters().size();i++)
				sim->Parameters()[i]->SetIsFixed(false);
		}
		sim->Finalize();
	}
	
	CheckIfSimulationCanBeExported(sim);

	sim->MarkQuantitiesUsedBySwitches();

	////simplify parameters that could not be simplified earlier (in Finalize)
	////(e.g. parameters that depend on not fixed constant parameters)
	//sim->SimplifyObjects(false);

	string ODEOptionsFile = OutDir+"\\ODEoptions.m";		
	WriteODEOptionsFile(sim, ODEOptionsFile);
	
	string ODEInitValuesFile = OutDir+"\\ODEInitialValues.m";
	WriteODEInitialValuesFile(sim, ODEInitValuesFile);
	
	string RHSFunctionFile=OutDir+"\\ODERHSFunction.m";
	WriteODERHSFile(sim, RHSFunctionFile);
	
	string ODEMainFile = OutDir+"\\ODEMain.m";
	WriteODEMainFile(sim, ODEMainFile);
	
	string switchesFile=OutDir+"\\PerformSwitches.m";
	WriteSwitchFile(sim, switchesFile);

	string tableParametersFile=OutDir+"\\SetupTableParameters.m";
	WriteTableParametersFile(sim, tableParametersFile);

	WriteHelperFunctions(OutDir);
}

void MatlabODEExporter::WriteTableParametersFile (Simulation * sim, const string & tableParametersFile)
{
	ofstream outfile;
	
	try
	{
		int i;
		vector <Parameter *> tableParameters;

		outfile.open(tableParametersFile.c_str());
		outfile.precision(16);

		outfile<<"function SetupTableParameters"<<endl<<endl;

		//---- assign table formula to every table parameter
		for(i=0; i<sim->Parameters().size();i++)
		{
			Parameter * param = sim->Parameters()[i];
			if (!param->IsTable())
				continue;

			string paramName = param->GetShortUniqueName();

			//declare as global
			outfile<<"    global "<<paramName<<";"<<endl;

			//assign table function
			outfile<<"    "<<paramName<<" = @(Time,y) "<<param->TableFunctionNameForMatlab()<<"(Time,y);"<<endl<<endl;

			//cache table parameter 
			tableParameters.push_back(param);
		}
		
		//---- now write table functions itself
		for(size_t j=0; j<tableParameters.size(); j++)
			tableParameters[j]->WriteTableFunctionForMatlab(outfile);

		outfile.close();
	}
	catch(...)
	{
		if (outfile.is_open())
			outfile.close();
				
		throw;
	}
}

void MatlabODEExporter::CheckIfSimulationCanBeExported(Simulation * sim)
{
	int i;
	
	double simStartTime = sim->GetStartTime();

	//---- table formulas with any restart time > simulation start time are not handled at the moment
	for(i=0;i<sim->Formulas().size();i++)
	{
		TableFormula * tableFormula = dynamic_cast <TableFormula *>(sim->Formulas()[i]);

		if(tableFormula == NULL)
			continue; //not a table formula

		vector <double> restartTimePoints = tableFormula->RestartTimePoints();

		if(restartTimePoints.size() == 0)
			continue;

		if ((restartTimePoints.size() >= 2) || (restartTimePoints[0] > simStartTime))
			throw ErrorData(ErrorData::ED_ERROR, "MatlabODEExporter::WriteMatlabCode", "Table formulas with required restart times>0 are currently not supported by matlab export");
	}
}

void MatlabODEExporter::WriteODEOptionsFile (Simulation * sim, const string & ODEOptionsFile)
{
	ofstream outfile;
	
	try
	{	
		const DESolverProperties & solverProperties = sim->GetSolver().GetSolverProperties();

		string BDFOption = "'on'"; 

		outfile.open(ODEOptionsFile.c_str());
		
		outfile<<"function opt = ODEoptions"<<endl<<endl;
		
		outfile<<"    global eventsFunctionHandle;"<<endl<<endl;

		outfile<<"    opt = odeset('AbsTol',"<<solverProperties.GetAbsTol()<<", ..."<<endl;
		outfile<<"                 'BDF',"<<BDFOption<<", ..."<<endl;
		outfile<<"                 'InitialStep',"<<solverProperties.GetH0()<<", ..."<<endl;
		
		outfile<<"                 'MaxOrder',"<<5<<", ..."<<endl;
		outfile<<"                 'MaxStep',"<<solverProperties.GetMxStep()<<", ..."<<endl;
		outfile<<"                 'RelTol',"<<solverProperties.GetRelTol()<<", ..."<<endl;
		outfile<<"                 'Events',eventsFunctionHandle ..."<<endl;
		outfile<<"                );"<<endl;

		outfile.close();
	}
	catch(...)
	{
		if (outfile.is_open())
			outfile.close();
		
		throw;
	}
}

void MatlabODEExporter::WriteODEInitialValuesFile (Simulation * sim, const string & ODEInitValuesFile)
{
	ofstream outfile;
	double * InitialValues = NULL;
	
	try
	{	
		InitialValues = sim->GetDEInitialValues();

		outfile.open(ODEInitValuesFile.c_str());
		
		outfile<<"function y = ODEInitialValues"<<endl<<endl;
		
		for (int i=0;i<sim->GetODENumUnknowns();i++)
		{
			Species * spec=(Species*) sim->GetDEVariableFromIndex(i);
			
			//Simmodel indexing starts at 0, Matlab indexing at 1 (!)
			outfile<<"    y("<<i+1<<") = " <<InitialValues[i]<<"; % "<<spec->GetFullName()<<endl;
		}
		
		outfile<<endl<<"    y=y';"<<endl;
		
		outfile.close();
		
		delete[] InitialValues;
		InitialValues = NULL;		
	}
	catch(...)
	{
		if (outfile.is_open())
			outfile.close();
		
		if (InitialValues)
			delete[] InitialValues;
		
		throw;
	}
}

void MatlabODEExporter::WriteODERHSFile (Simulation * sim, const string & RHSFunctionFile)
{
	ofstream outfile;

	try
	{	
		outfile.open(RHSFunctionFile.c_str());
		
		outfile<<"function dy = ODERHSFunction(Time,y)"<<endl<<endl;
		
		//---- write parameters
		
		vector <Simulation::HierarchicalFormulaObjectVector> leveledHierarchicalFormulaObjects = 
			sim->GetLeveledHierarchicalFormulaObjects();

		int paramIdx = 0;

		for (unsigned int HLevelIdx=0; HLevelIdx<leveledHierarchicalFormulaObjects.size(); HLevelIdx++)
		{
			vector <HierarchicalFormulaObject *> HFObjectsForLevel = leveledHierarchicalFormulaObjects[HLevelIdx];

			for (unsigned int HFObjectIdx = 0; HFObjectIdx<HFObjectsForLevel.size(); HFObjectIdx++)
			{
				HierarchicalFormulaObject * HObject = HFObjectsForLevel[HFObjectIdx];

				Parameter * param = dynamic_cast<Parameter *>(HObject);
				if (!param)
					continue;

				//set parameter short unique name to be used for generating ODE system
				param->SetShortUniqueNameForDESystem(GetShortUniqueParameterName(paramIdx));
				paramIdx++;

				//declare parameters used and/or changed by switches and also table parameters as global 
				if (param->ExportAsGlobalForMatlab())
					outfile<<"global "<<param->GetShortUniqueName()<<";"<<endl;

				param->WriteMatlabCode(outfile, true);
			}
		}

		//---- write ode equations
		outfile<<endl;
		
		for(int iSpecies=0;iSpecies<sim->GetODENumUnknowns();iSpecies++)
		{
			Species * species = sim->GetDEVariableFromIndex(iSpecies);
			assert(species != NULL);

			species->WriteMatlabCode(outfile);
		}
		
		outfile<<"dy = dy';"<<endl;
		
		outfile.close();
	}
	catch(...)
	{
		if (outfile.is_open())
			outfile.close();
		
		throw;
	}

}

void MatlabODEExporter::WriteSwitchFile (Simulation * sim, const std::string & switchFile)
{
	ofstream outfile;
	
	try
	{
		int i;

		outfile.open(switchFile.c_str());
		outfile.precision(16);

		outfile<<"function [yOut switchUpdate] = PerformSwitches(Time, y)"<<endl<<endl;

		outfile<<"    switchUpdate = false;"<<endl;
		outfile<<"    yOut = y;"<<endl<<endl;

		//---- in case any switch fires, switch function will be called second
		//     time (after ode system is restarted). To make sure that no  
		//     formula change is performed twice, all time points at which
		//     any switch was fired, will be cached

		outfile<<"    global SwitchUpdateTimePoints;"<<endl<<endl;

		//declare all parameters as global
		for(i=0; i<sim->Parameters().size();i++)
		{
			Parameter * param = sim->Parameters()[i];

			if (param->ExportAsGlobalForMatlab())
				outfile<<"    global "<<param->GetShortUniqueName()<<";"<<endl;
		}
		
		outfile<<endl<<endl;

		string switchIdxAsStr;

		for(i=0; i<sim->Switches().size(); i++)
			sim->Switches()[i]->WriteMatlabCode(outfile);

		outfile<<"    if switchUpdate"<<endl;
		outfile<<"        SwitchUpdateTimePoints = [SwitchUpdateTimePoints Time];"<<endl;
		outfile<<"    end"<<endl<<endl;

		outfile.close();
	}
	catch(...)
	{
		if (outfile.is_open())
			outfile.close();
				
		throw;
	}
}

void MatlabODEExporter::WriteODEMainFile (Simulation * sim, const string & ODEMainFile)
{
	ofstream outfile;
	
	try
	{
		//get unique application start times for the simulation
		std::set<double> switchTimes = sim->GetSwitchTimes();

		outfile.open(ODEMainFile.c_str());
		outfile.precision(16);

		outfile<<"function [tout, yout] = ODEMain"<<endl<<endl;

		outfile<<"    tic"<<endl<<endl;
		
		outfile<<"    clear global;"<<endl<<endl;

		outfile<<"    global eventsFunctionHandle;"<<endl;
		outfile<<"    eventsFunctionHandle = @events;"<<endl<<endl;
		
		//setup table parameters
		outfile<<"    SetupTableParameters;"<<endl<<endl;

		//write start values and initial time
		outfile<<"    y0 = ODEInitialValues;"<<endl;
		outfile<<"    tstart = "<<sim->GetStartTime()<<";"<<endl;
		outfile<<"    tout = tstart;"<<endl<<endl;

		//perform initial switch update
		outfile<<"    %call ODE RHS function in order to init all parameters"<<endl;
		outfile<<"    ODERHSFunction(tstart, y0);"<<endl;
		outfile<<"    %perform initial switches update"<<endl;
		outfile<<"    [y0 switchUpdate] = PerformSwitches(tstart, y0);"<<endl<<endl;

		//append start values into output
		outfile<<"    yout = y0.';"<<endl<<endl;

		//write output times vector
		vector <OutputTimePoint> outputTimePoints = SimulationTask::OutputTimePoints(sim);

		outfile<<"    outtimes = [ "<<sim->GetStartTime()<<" ";
		for(unsigned int timeStepIdx=0; timeStepIdx<outputTimePoints.size(); timeStepIdx++)
		{
			OutputTimePoint outTimePoint = outputTimePoints[timeStepIdx];

			if(outTimePoint.SaveSystemSolution())
				outfile<<outTimePoint.Time()<<" ";
		}

		outfile<<"];"<<endl<<endl;

		//write application start AND end times vector
		outfile<<"    global switchtimes;"<<endl;
		outfile<<"    switchtimes = [ ";
		for (auto sIter = switchTimes.begin(); sIter != switchTimes.end(); ++sIter)
		{
			outfile << *sIter << " ";
		}
		outfile<<"];"<<endl<<endl;

		outfile<<"    NextRestartTime = tstart;"<<endl;

		//write main loop
		outfile<<"    while NextRestartTime < outtimes(end)"<<endl<<endl;

		outfile<<"        switchtimes = switchtimes(switchtimes > NextRestartTime);"<<endl;
		outfile<<"        if isempty(switchtimes)"<<endl;
		outfile<<"            NextRestartTime = outtimes(end);"<<endl;
		outfile<<"        else"<<endl;
		outfile<<"            NextRestartTime = switchtimes(1);"<<endl;
		outfile<<"        end"<<endl<<endl;

		outfile<<"        while (1)"<<endl;
		outfile<<"            outtimes_step_i = [tstart outtimes(outtimes > tstart & outtimes <= NextRestartTime)];"<<endl;
		outfile<<"            if (outtimes_step_i(end) < NextRestartTime)"<<endl;
		outfile<<"                outtimes_step_i = [outtimes_step_i NextRestartTime]; %#ok<AGROW>"<<endl;
		outfile<<"            end"<<endl<<endl;

		outfile<<"            [t, y] = ode15s(@ODERHSFunction, outtimes_step_i, y0, ODEoptions);"<<endl<<endl;

		outfile<<"            nt = length(t);"<<endl;
		outfile<<"            tout = [tout; t(2:nt)]; %#ok<AGROW>"<<endl;
		outfile<<"            yout = [yout; y(2:nt,:)]; %#ok<AGROW>"<<endl<<endl;

		outfile<<"            y0 = y(nt,:);"<<endl;
		outfile<<"            tstart = t(nt);"<<endl<<endl;
		
		outfile<<"            %update start vector by switches (if applies)"<<endl;
		outfile<<"            [y0 switchUpdate] = PerformSwitches(tstart, y0);"<<endl<<endl;

		outfile<<"            if (tstart==outtimes_step_i(end))"<<endl;
		outfile<<"                break"<<endl;
		outfile<<"            end"<<endl;
		outfile<<"        end"<<endl;

		outfile<<"    end"<<endl<<endl;

		outfile<<"    [tout idx_out] = intersect(tout, outtimes);"<<endl;
		outfile<<"    yout = yout(idx_out, :);"<<endl<<endl;

		outfile<<"    toc"<<endl<<endl;
		
		//---- write events function
		outfile<<"function [value,isterminal,direction] = events(Time,y)"<<endl<<endl;

		outfile<<"    value = 1;      %no event per default"<<endl;
		outfile<<"    isterminal = 1; % stop the integration"<<endl;
		outfile<<"    direction = 0;  % negative direction"<<endl<<endl;

		outfile<<"    global switchtimes;"<<endl;
		outfile<<"    if (min(abs(switchtimes - Time)) == 0.0)"<<endl;
		outfile<<"        return"<<endl;
		outfile<<"    end"<<endl<<endl;

		outfile<<"    [yOut switchUpdate] = PerformSwitches(Time,y);"<<endl<<endl;

		outfile<<"    if ~switchUpdate"<<endl;
		outfile<<"        %no switch fired"<<endl;
		outfile<<"        return"<<endl;
		outfile<<"    end"<<endl<<endl;

		outfile<<"    value = 0;      % force system restart"<<endl;

		outfile.close();
	}
	catch(...)
	{
		if (outfile.is_open())
			outfile.close();
				
		throw;
	}
}

void MatlabODEExporter::WriteHelperFunctions (const string & OutDir)
{
	ofstream outfile;
	
	try
	{
		//-------------- IIf (If-function) -----------------------
		string iifFile = OutDir+"\\IIf.m";
		outfile.open(iifFile.c_str());
		
		outfile<<"function y = IIf(condition, truepart, falsepart)"<<endl<<endl;
		outfile<<"    if condition"<<endl;
		outfile<<"        y = truepart;"<<endl;
		outfile<<"    else"<<endl;
		outfile<<"        y = falsepart;"<<endl;
		outfile<<"    end"<<endl;
		
		outfile.close();

		//-------------- EvalParameter -----------------------
		//Evaluates parameter depending on its type: numeric value or function handle
		string evalParameterFile = OutDir+"\\EvalParameter.m";
		outfile.open(evalParameterFile.c_str());
		
		outfile<<"function value = EvalParameter(P, t, y)"<<endl<<endl;
		outfile<<"    if isnumeric(P)"<<endl;
		outfile<<"        value = P;"<<endl;
		outfile<<"    else"<<endl;
		outfile<<"        value = P(t,y);"<<endl;
		outfile<<"    end"<<endl;
		
		outfile.close();

	}
	catch(...)
	{
		if (outfile.is_open())
			outfile.close();
				
		throw;
	}
}

string MatlabODEExporter::GetShortUniqueParameterName (int parameterIdx)
{
	char name[100];
	sprintf(name,"P_%d",parameterIdx);
	
	return name;
}

}//.. end "namespace SimModelNative"