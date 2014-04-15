#include "SingleTkEleEta.h"

#include <stdexcept>
#include "../implementation/RegisterTriggerMacro.h"
#include "l1menu/L1TriggerDPGEvent.h"
#include "UserCode/L1TriggerUpgrade/interface/L1AnalysisDataFormat.h"


namespace l1menu
{
	namespace triggers
	{

		/* The REGISTER_TRIGGER macro will make sure that the given trigger is registered in the
		 * l1menu::TriggerTable when the program starts. I also want to provide some suggested binning
		 * however. The REGISTER_TRIGGER_AND_CUSTOMISE macro does exactly the same but lets me pass
		 * a pointer to a function that will be called directly after the trigger has been registered
		 * at program startup. The function takes no parameters and returns void. In this case I'm
		 * giving it a lambda function.
		 */
		REGISTER_TRIGGER_AND_CUSTOMISE( SingleTkEleEta_v1,
			[]() // Use a lambda function to customise rather than creating a named function that never gets used again.
			{
				l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();
				SingleTkEleEta_v1 tempTriggerInstance;
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "threshold1", 100, 0, 100 );
			} // End of customisation lambda function
		) // End of REGISTER_TRIGGER_AND_CUSTOMISE macro call
		REGISTER_TRIGGER( SingleTkEleEta_v0 )


	} // end of namespace triggers

} // end of namespace l1menu


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//---------------  Definitions below         ---------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


bool l1menu::triggers::SingleTkEleEta_v0::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw = PhysicsBits[0];   // ZeroBias
	if (! raw) return false;

	bool ok=false;
	int NTkele = analysisDataFormat.NTkele;
	for (int ue=0; ue < NTkele; ue++) {
		int bx = analysisDataFormat.BxTkel[ue];
		if (bx != 0) continue;
		float eta = analysisDataFormat.EtaTkel[ue];
		if (eta < regionCut_ || eta > 21.-regionCut_) continue;  // eta = 5 - 16
		float rank = analysisDataFormat.EtTkel[ue];    // the rank of the electron
		float pt = rank ;
		if (pt >= threshold1_) ok = true;
	}  // end loop over EM objects

	return ok;
}

bool l1menu::triggers::SingleTkEleEta_v0::thresholdsAreCorrelated() const
{
	return false;
}

unsigned int l1menu::triggers::SingleTkEleEta_v0::version() const
{
	return 0;
}

bool l1menu::triggers::SingleTkEleEta_v1::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw = PhysicsBits[0];   // ZeroBias
	if (! raw) return false;

	bool ok=false;
	int NTkele = analysisDataFormat.NTkele2;
	for (int ue=0; ue < NTkele; ue++) {
		int bx = analysisDataFormat.BxTkel2[ue];
		if (bx != 0) continue;
		float eta = analysisDataFormat.EtaTkel2[ue];
		if (eta < regionCut_ || eta > 21.-regionCut_) continue;  // eta = 5 - 16
		float rank = analysisDataFormat.EtTkel2[ue];    // the rank of the electron
		float pt = rank ;
		if (pt >= threshold1_) ok = true;
	}  // end loop over EM objects

	return ok;
}

bool l1menu::triggers::SingleTkEleEta_v1::thresholdsAreCorrelated() const
{
	return false;
}

unsigned int l1menu::triggers::SingleTkEleEta_v1::version() const
{
	return 1;
}

l1menu::triggers::SingleTkEleEta::SingleTkEleEta()
	: threshold1_(20), regionCut_(4.5)
{
	// No operation other than the initialiser list
}

const std::string l1menu::triggers::SingleTkEleEta::name() const
{
	return "L1_SingleTkEle";
}

const std::vector<std::string> l1menu::triggers::SingleTkEleEta::parameterNames() const
{
	std::vector<std::string> returnValue;
	returnValue.push_back("threshold1");
	returnValue.push_back("regionCut");
	return returnValue;
}

float& l1menu::triggers::SingleTkEleEta::parameter( const std::string& parameterName )
{
	if( parameterName=="threshold1" ) return threshold1_;
	else if( parameterName=="regionCut" ) return regionCut_;
	else throw std::logic_error( "Not a valid parameter name" );
}

const float& l1menu::triggers::SingleTkEleEta::parameter( const std::string& parameterName ) const
{
	if( parameterName=="threshold1" ) return threshold1_;
	else if( parameterName=="regionCut" ) return regionCut_;
	else throw std::logic_error( "Not a valid parameter name" );
}
