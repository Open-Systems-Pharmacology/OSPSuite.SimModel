dllsPath = 'C:\VSS\PK-Sim\branches\6.3\Dev\PKSim\bin\Debug';
addpath(dllsPath);
setenv('path', [dllsPath ';' getenv('path')]);
warning('off', 'MATLAB:catenate:DimensionMismatch');

%Create the component
comp=DCIMatlabR2017b6_1('LoadComponent', [dllsPath '\OSPSuite_SimModelComp.xml']);

%Get parameter table
tab=DCIMatlabR2017b6_1('GetParameterTable',comp,1);

%%%%%%%%%%%%%%% Set Parameters %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%Simulation Schema
tab.Variables(1).Values={[dllsPath '\OSPSuite.SimModel.xsd']};

%Simulation File
tab.Variables(2).Values={'C:\VSS\SimModel\branches\6.0_Git\tests\TestData\SimModel4_ExampleInput06.xml'};

%Set parameter table into the component and configure
DCIMatlabR2017b6_1('SetParameterTable',comp,1,tab);
DCIMatlabR2017b6_1('Configure',comp);

% ------------------------------------------------------------------------------------
% Define column indices for the input tables 
%
% 1st input table (all parameters, read only) has following structure
% 'ID' 'Path' 'Value' 'Unit' 'IsFormula' 'Formula'
%
% 2nd input table (variable parameters) has following structure
% 'ID' 'Path' 'Value' 'Unit' 'IsFormula' 'Formula' 'IsVariable'
%
% 3rd input table (all species, read only) has following structure
% 'ID' 'Path' 'InitialValue' 'ScaleFactor' 'Unit' 'IsFormula' 'Formula'
%
% 4th input table (variable species) has following structure
% 'ID' 'Path' 'InitialValue' 'ScaleFactor' 'Unit' 'IsFormula' 'Formula' 'IsVariable'
%
% 5th input table (all observers, read only) has following structure
% 'ID' 'Path' 'Formula'
%
% 6th input table (time output schema) has following structure
% 'StartTime' 'EndTime' 'Unit' 'NoOfTimePoints' 'Distribution'
%
% 7th input table (time output schema for particular species) has following structure
% 'ID' 'Path' 'StartTime' 'EndTime' 'Unit' 'NoOfTimePoints' 'Distribution'
%
% ------------------------------------------------------------------------------------

%define column indices for input parameters tables (both all and variable)
ParamIDIdx=1; ParamPathIdx=2; ParamValueIdx=3; ParamUnitIdx=4; 
ParamIsFormulaIdx=5; ParamFormulaIdx=6; ParamDescriptionIdx=7; ParamIsVariableIdx=8;

%define column indices for input species tables (both all and variable)
SpeciesIDIdx=1; SpeciesPathIdx=2; SpeciesInitialValueIdx=3; SpeciesScaleFactorIdx=4;
SpeciesUnitIdx=5; SpeciesIsFormulaIdx=6; SpeciesFormulaIdx=7; SpeciesDescriptionIdx=8; SpeciesIsVariableIdx=9;

%define column indices for input observer table
ObserverIDIdx=1; ObserverPathIdx=2; ObserverUnitIdx=3; ObserverFormulaIdx=4; ObserverDescriptionIdx=5;

%define column indices for time output schema table
OutSchemaStartTimeIdx=1; OutSchemaEndTimeIdx=2; OutSchemaUnitIdx=3; 
OutSchemaNoOfTimePointsIdx=4; OutSchemaDistributionIdx=5; 

%define column indices for species specific time output schema table
OutSchemaSpeciesIDIdx=1; OutSchemaSpeciesPathIdx=2;
OutSchemaSpeciesStartTimeIdx=3; OutSchemaSpeciesEndTimeIdx=4; OutSchemaSpeciesUnitIdx=5; 
OutSchemaSpeciesNoOfTimePointsIdx=6; OutSchemaSpeciesDistributionIdx=7; 

%get input tables
inTab1 = DCIMatlabR2017b6_1('GetInputTable',comp,1); inTab2 = DCIMatlabR2017b6_1('GetInputTable',comp,2);
inTab3 = DCIMatlabR2017b6_1('GetInputTable',comp,3); inTab4 = DCIMatlabR2017b6_1('GetInputTable',comp,4);
inTab5 = DCIMatlabR2017b6_1('GetInputTable',comp,5); inTab6 = DCIMatlabR2017b6_1('GetInputTable',comp,6);
inTab7 = DCIMatlabR2017b6_1('GetInputTable',comp,7);

%show initial input tables
[{inTab1.Variables(1:end).Name}; num2cell(inTab1.Variables(ParamIDIdx).Values) inTab1.Variables(ParamPathIdx).Values num2cell(inTab1.Variables(ParamValueIdx).Values) inTab1.Variables(ParamUnitIdx).Values num2cell(inTab1.Variables(ParamIsFormulaIdx).Values) inTab1.Variables(ParamFormulaIdx).Values inTab1.Variables(ParamDescriptionIdx).Values]
[{inTab2.Variables(1:end).Name}; num2cell(inTab2.Variables(ParamIDIdx).Values) inTab2.Variables(ParamPathIdx).Values num2cell(inTab2.Variables(ParamValueIdx).Values) inTab2.Variables(ParamUnitIdx).Values num2cell(inTab2.Variables(ParamIsFormulaIdx).Values) inTab2.Variables(ParamFormulaIdx).Values inTab2.Variables(ParamDescriptionIdx).Values num2cell(inTab2.Variables(ParamIsVariableIdx).Values)]
[{inTab3.Variables(1:end).Name}; num2cell(inTab3.Variables(SpeciesIDIdx).Values) inTab3.Variables(SpeciesPathIdx).Values num2cell(inTab3.Variables(SpeciesInitialValueIdx).Values) num2cell(inTab3.Variables(SpeciesScaleFactorIdx).Values) inTab3.Variables(SpeciesUnitIdx).Values num2cell(inTab3.Variables(SpeciesIsFormulaIdx).Values) inTab3.Variables(SpeciesFormulaIdx).Values inTab3.Variables(SpeciesDescriptionIdx).Values]
[{inTab4.Variables(1:end).Name}; num2cell(inTab4.Variables(SpeciesIDIdx).Values) inTab4.Variables(SpeciesPathIdx).Values num2cell(inTab4.Variables(SpeciesInitialValueIdx).Values) num2cell(inTab4.Variables(SpeciesScaleFactorIdx).Values) inTab4.Variables(SpeciesUnitIdx).Values num2cell(inTab4.Variables(SpeciesIsFormulaIdx).Values) inTab4.Variables(SpeciesFormulaIdx).Values inTab4.Variables(SpeciesDescriptionIdx).Values num2cell(inTab4.Variables(SpeciesIsVariableIdx).Values)]
[{inTab5.Variables(1:end).Name}; num2cell(inTab5.Variables(ObserverIDIdx).Values) inTab5.Variables(ObserverPathIdx).Values inTab5.Variables(ObserverUnitIdx).Values inTab5.Variables(ObserverFormulaIdx).Values inTab5.Variables(ObserverDescriptionIdx).Values]
[{inTab6.Variables(1:end).Name}; num2cell(inTab6.Variables(OutSchemaStartTimeIdx).Values) num2cell(inTab6.Variables(OutSchemaEndTimeIdx).Values) inTab6.Variables(OutSchemaUnitIdx).Values num2cell(inTab6.Variables(OutSchemaNoOfTimePointsIdx).Values) inTab6.Variables(OutSchemaDistributionIdx).Values]
[{inTab7.Variables(1:end).Name}; num2cell(inTab7.Variables(OutSchemaSpeciesIDIdx).Values) inTab7.Variables(OutSchemaSpeciesPathIdx).Values num2cell(inTab7.Variables(OutSchemaStartTimeIdx).Values) num2cell(inTab7.Variables(OutSchemaEndTimeIdx).Values) inTab7.Variables(OutSchemaUnitIdx).Values num2cell(inTab7.Variables(OutSchemaNoOfTimePointsIdx).Values) inTab7.Variables(OutSchemaDistributionIdx).Values]

%------------------------------------------------------------------
% Setup parameters to be varied
%------------------------------------------------------------------

%Select parameters to be varied
inTab2.Variables(ParamIsVariableIdx).Values(4)=1;

%View changed table
[{inTab2.Variables(1:end).Name}; num2cell(inTab2.Variables(ParamIDIdx).Values) inTab2.Variables(ParamPathIdx).Values num2cell(inTab2.Variables(ParamValueIdx).Values) inTab2.Variables(ParamUnitIdx).Values num2cell(inTab2.Variables(ParamIsFormulaIdx).Values) inTab2.Variables(ParamFormulaIdx).Values inTab2.Variables(ParamDescriptionIdx).Values num2cell(inTab2.Variables(ParamIsVariableIdx).Values)]

%Save input table into the component and set variable parameters
DCIMatlabR2017b6_1('SetInputTable',comp,2,inTab2);

%------------------------------------------------------------------
% Setup species properties to be varied
%------------------------------------------------------------------

%Select species whose initial values should be changed/varied during optimization
inTab4.Variables(SpeciesIsVariableIdx).Values(3)=1;

%View changed table
[{inTab4.Variables(1:end).Name}; num2cell(inTab4.Variables(SpeciesIDIdx).Values) inTab4.Variables(SpeciesPathIdx).Values num2cell(inTab4.Variables(SpeciesInitialValueIdx).Values) num2cell(inTab4.Variables(SpeciesScaleFactorIdx).Values) inTab4.Variables(SpeciesUnitIdx).Values num2cell(inTab4.Variables(SpeciesIsFormulaIdx).Values) inTab4.Variables(SpeciesFormulaIdx).Values inTab4.Variables(SpeciesDescriptionIdx).Values num2cell(inTab4.Variables(SpeciesIsVariableIdx).Values)]

%Save input table into the component and set variable parameters
DCIMatlabR2017b6_1('SetInputTable',comp,4,inTab4);

%------------------------------------------------------------------
%ProcessMetaData leaves only parameters/species specified by user for optimization
DCIMatlabR2017b6_1('ProcessMetaData',comp);

%get input tables
inTab1 = DCIMatlabR2017b6_1('GetInputTable',comp,1); inTab2 = DCIMatlabR2017b6_1('GetInputTable',comp,2);
inTab3 = DCIMatlabR2017b6_1('GetInputTable',comp,3); inTab4 = DCIMatlabR2017b6_1('GetInputTable',comp,4);
inTab5 = DCIMatlabR2017b6_1('GetInputTable',comp,5); inTab6 = DCIMatlabR2017b6_1('GetInputTable',comp,6);
inTab7 = DCIMatlabR2017b6_1('GetInputTable',comp,7);

%show input tables
[{inTab1.Variables(1:end).Name}; num2cell(inTab1.Variables(ParamIDIdx).Values) inTab1.Variables(ParamPathIdx).Values num2cell(inTab1.Variables(ParamValueIdx).Values) inTab1.Variables(ParamUnitIdx).Values num2cell(inTab1.Variables(ParamIsFormulaIdx).Values) inTab1.Variables(ParamFormulaIdx).Values inTab1.Variables(ParamDescriptionIdx).Values]
[{inTab2.Variables(1:end).Name}; num2cell(inTab2.Variables(ParamIDIdx).Values) inTab2.Variables(ParamPathIdx).Values num2cell(inTab2.Variables(ParamValueIdx).Values) inTab2.Variables(ParamUnitIdx).Values num2cell(inTab2.Variables(ParamIsFormulaIdx).Values) inTab2.Variables(ParamFormulaIdx).Values inTab2.Variables(ParamDescriptionIdx).Values]
[{inTab3.Variables(1:end).Name}; num2cell(inTab3.Variables(SpeciesIDIdx).Values) inTab3.Variables(SpeciesPathIdx).Values num2cell(inTab3.Variables(SpeciesInitialValueIdx).Values) num2cell(inTab3.Variables(SpeciesScaleFactorIdx).Values) inTab3.Variables(SpeciesUnitIdx).Values num2cell(inTab3.Variables(SpeciesIsFormulaIdx).Values) inTab3.Variables(SpeciesFormulaIdx).Values inTab3.Variables(SpeciesDescriptionIdx).Values]
[{inTab4.Variables(1:end).Name}; num2cell(inTab4.Variables(SpeciesIDIdx).Values) inTab4.Variables(SpeciesPathIdx).Values num2cell(inTab4.Variables(SpeciesInitialValueIdx).Values) num2cell(inTab4.Variables(SpeciesScaleFactorIdx).Values) inTab4.Variables(SpeciesUnitIdx).Values num2cell(inTab4.Variables(SpeciesIsFormulaIdx).Values) inTab4.Variables(SpeciesFormulaIdx).Values inTab4.Variables(SpeciesDescriptionIdx).Values]
[{inTab5.Variables(1:end).Name}; num2cell(inTab5.Variables(ObserverIDIdx).Values) inTab5.Variables(ObserverPathIdx).Values inTab5.Variables(ObserverFormulaIdx).Values inTab5.Variables(ObserverDescriptionIdx).Values]
[{inTab6.Variables(1:end).Name}; num2cell(inTab6.Variables(OutSchemaStartTimeIdx).Values) num2cell(inTab6.Variables(OutSchemaEndTimeIdx).Values) inTab6.Variables(OutSchemaUnitIdx).Values num2cell(inTab6.Variables(OutSchemaNoOfTimePointsIdx).Values) inTab6.Variables(OutSchemaDistributionIdx).Values]
[{inTab7.Variables(1:end).Name}; num2cell(inTab7.Variables(OutSchemaSpeciesIDIdx).Values) inTab7.Variables(OutSchemaSpeciesPathIdx).Values num2cell(inTab7.Variables(OutSchemaStartTimeIdx).Values) num2cell(inTab7.Variables(OutSchemaEndTimeIdx).Values) inTab7.Variables(OutSchemaUnitIdx).Values num2cell(inTab7.Variables(OutSchemaNoOfTimePointsIdx).Values) inTab7.Variables(OutSchemaDistributionIdx).Values]

%------------------------------------------------------------------
% Update parameters for the next simulation run
%------------------------------------------------------------------

%Set new parameter values
inTab2.Variables(ParamValueIdx).Values(1)=2;

%View current values
[{inTab2.Variables(1:end).Name}; num2cell(inTab2.Variables(ParamIDIdx).Values) inTab2.Variables(ParamPathIdx).Values num2cell(inTab2.Variables(ParamValueIdx).Values) inTab2.Variables(ParamUnitIdx).Values num2cell(inTab2.Variables(ParamIsFormulaIdx).Values) inTab2.Variables(ParamFormulaIdx).Values inTab2.Variables(ParamDescriptionIdx).Values]

%Save new parameter values into the component
DCIMatlabR2017b6_1('SetInputTable',comp,2,inTab2);

%------------------------------------------------------------------
% Update species initial values and scale factor for the next simulation run
%------------------------------------------------------------------

%Set new initial values and scale factors
inTab4.Variables(SpeciesInitialValueIdx).Values(1)=0.1;
inTab4.Variables(SpeciesScaleFactorIdx).Values(1)=10;

%View current values
[{inTab4.Variables(1:end).Name}; num2cell(inTab4.Variables(SpeciesIDIdx).Values) inTab4.Variables(SpeciesPathIdx).Values num2cell(inTab4.Variables(SpeciesInitialValueIdx).Values) num2cell(inTab4.Variables(SpeciesScaleFactorIdx).Values) inTab4.Variables(SpeciesUnitIdx).Values num2cell(inTab4.Variables(SpeciesIsFormulaIdx).Values) inTab4.Variables(SpeciesFormulaIdx).Values inTab4.Variables(SpeciesDescriptionIdx).Values]

%Save new species inbitial values into the component
DCIMatlabR2017b6_1('SetInputTable',comp,4,inTab4);

%------------------------------------------------------------------
% Update output time schema
%------------------------------------------------------------------
inTab6.Variables(1).Values = [0 100 200]';
inTab6.Variables(2).Values = [10 110 220]';
inTab6.Variables(3).Values = {'min', 'min', 'min'}';
inTab6.Variables(4).Values = [11 11 5]';
inTab6.Variables(5).Values = {'Equidistant', 'Equidistant', 'Equidistant'}';

%View current values
[{inTab6.Variables(1:end).Name}; num2cell(inTab6.Variables(OutSchemaStartTimeIdx).Values) num2cell(inTab6.Variables(OutSchemaEndTimeIdx).Values) inTab6.Variables(OutSchemaUnitIdx).Values num2cell(inTab6.Variables(OutSchemaNoOfTimePointsIdx).Values) inTab6.Variables(OutSchemaDistributionIdx).Values]

%Save new species inbitial values into the component
DCIMatlabR2017b6_1('SetInputTable',comp,6,inTab6);

%------------------------------------------------------------------
%Perform one simulation Run
DCIMatlabR2017b6_1('ProcessData',comp);

%#######################################################################

%save changed simulation to XML
%DCIMatlabR2017b6_1('Invoke', comp, 'SaveSimulationToXml', '');

%------------------------------------------------------------------
% Get and show some results
%------------------------------------------------------------------

%Get 1st Output table (Simulation Times)
outTabTimes=DCIMatlabR2017b6_1('GetOutputTable',comp,1);

%Get 2nd Output table (Simulation values)
outTabValues=DCIMatlabR2017b6_1('GetOutputTable',comp,2);

%show metainfos for the 1st output
FirstOutVar = outTabValues.Variables(1);
{FirstOutVar.Attributes.Name; FirstOutVar.Attributes.Value}

%Get time column index
timeColIdx = str2num(FirstOutVar.Attributes(find(strcmp({FirstOutVar.Attributes.Name}, 'TimeColumnIndex'))).Value);

%show {time, value} pairs for the 1st output variable
timeColumn = outTabTimes.Variables(timeColIdx);
[{timeColumn.Name FirstOutVar.Name}; num2cell(timeColumn.Values) num2cell(FirstOutVar.Values)]

