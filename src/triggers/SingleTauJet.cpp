#include "SingleTauJet.h"

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
		REGISTER_TRIGGER_AND_CUSTOMISE( SingleTauJet_v0,
			[]() // Use a lambda function to customise rather than creating a named function that never gets used again.
			{
				l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();
				SingleTauJet_v0 tempTriggerInstance;
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "threshold1", 200, 0, 200 );
			} // End of customisation lambda function
		) // End of REGISTER_TRIGGER_AND_CUSTOMISE macro call


	} // end of namespace triggers

} // end of namespace l1menu


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//---------------  Definitions below         ---------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


bool l1menu::triggers::SingleTauJet_v0::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw = PhysicsBits[0];  // ZeroBias
	if (! raw) return false;

	int n1=0;
	int Nj = analysisDataFormat.Njet ;
	for (int ue=0; ue < Nj; ue++) {
		int bx = analysisDataFormat.Bxjet[ue];
		if (bx != 0) continue;
		bool isTauJet = analysisDataFormat.Taujet[ue];
		if (! isTauJet) continue;
		float rank = analysisDataFormat.Etjet[ue];    // the rank of the electron
		float pt = rank; //CorrectedL1JetPtByGCTregions(analysisDataFormat.Etajet[ue],rank*4.,theL1JetCorrection);
		float eta = analysisDataFormat.Etajet[ue];
		if (eta < regionCut_ || eta > 21.-regionCut_) continue;  // eta = 5 - 16  // eta = 5 - 16
		if (pt >= threshold1_) n1++;
	}  // end loop over jets

	bool ok = ( n1 >=1 );
	return ok;
}

bool l1menu::triggers::SingleTauJet_v0::thresholdsAreCorrelated() const
{
	return false;
}

unsigned int l1menu::triggers::SingleTauJet_v0::version() const
{
	return 0;
}

l1menu::triggers::SingleTauJet::SingleTauJet()
	: threshold1_(20), regionCut_(4.5)
{
	// No operation other than the initialiser list
}

const std::string l1menu::triggers::SingleTauJet::name() const
{
	return "L1_SingleTau";
}

const std::vector<std::string> l1menu::triggers::SingleTauJet::parameterNames() const
{
	std::vector<std::string> returnValue;
	returnValue.push_back("threshold1");
	returnValue.push_back("regionCut");
	return returnValue;
}

float& l1menu::triggers::SingleTauJet::parameter( const std::string& parameterName )
{
	if( parameterName=="threshold1" ) return threshold1_;
	else if( parameterName=="regionCut" ) return regionCut_;
	else throw std::logic_error( "Not a valid parameter name" );
}

const float& l1menu::triggers::SingleTauJet::parameter( const std::string& parameterName ) const
{
	if( parameterName=="threshold1" ) return threshold1_;
	else if( parameterName=="regionCut" ) return regionCut_;
	else throw std::logic_error( "Not a valid parameter name" );
}
