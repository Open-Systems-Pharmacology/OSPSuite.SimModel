#ifndef _SpeciesProperties_H_
#define _SpeciesProperties_H_

#include "SimModelManaged/AdjustableEntityProperties.h"
#include "SimModel/SpeciesInfo.h"

namespace SimModelNET
{
	//public interface
	public interface class ISpeciesProperties : public IAdjustableEntityProperties
	{
		property double ScaleFactor
		{
			double get();
			void set(double scaleFactor);
		}
	};

	ref class SpeciesProperties : 
		public AdjustableEntityProperties,
		public ISpeciesProperties
	{
	private:
		double _scaleFactor;
	internal:
		SpeciesProperties(SimModelNative::SpeciesInfo & speciesInfo, const std::string & objectPathDelimiter);

	public:
		property double ScaleFactor
		{
			virtual double get();
			virtual void set(double scaleFactor);
		}
	};
}

#endif //_ParameterProperties_H_

