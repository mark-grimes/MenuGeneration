#include "SingleTkMuEta.h"

#include <cmath>
#include <stdexcept>

#include "l1menu/L1TriggerDPGEvent.h"
#include "../implementation/RegisterTriggerMacro.h"
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
		REGISTER_TRIGGER_AND_CUSTOMISE( SingleTkMuEta_v0,
			[]() // Use a lambda function to customise rather than creating a named function that never gets used again.
			{
				l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();
				SingleTkMuEta_v0 tempTriggerInstance;
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "threshold1", 100, 0, 140 );
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

bool l1menu::triggers::SingleTkMuEta_v0::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw = PhysicsBits[0];   // ZeroBias
	if (! raw) return false;

	bool muon = false;

	int Nmu = analysisDataFormat.NTkmu;
	for (int imu=0; imu < Nmu; imu++) {
		int bx = analysisDataFormat.BxTkmu.at(imu);
		// This next comment line is copied from the original SingleIsoMuEta. It's commented out
		// in that trigger which leaves SingleTkMuEta and SingleIsoMuEta the same. I've set up
		// SingleIsoMuEta essentially as an alias for this trigger, but I'll leave this comment
		// in for reference in case I ever have to add the functionality back. MG 05/Jun/2013.
		//if (bx != 0 || !analysisDataFormat.Isomu.at(imu)) continue;
		if (bx != 0) continue;
		float pt = analysisDataFormat.PtTkmu.at(imu);
		int qual = analysisDataFormat.QualTkmu.at(imu);
		if ( qual < muonQuality_) continue;
		float eta = analysisDataFormat.EtaTkmu.at(imu);

		if (std::fabs(eta) > etaCut_) continue;
		if (pt >= threshold1_) muon = true;
	}

	bool ok = muon;
	return ok;
}

bool l1menu::triggers::SingleTkMuEta_v0::thresholdsAreCorrelated() const
{
	return false;
}

unsigned int l1menu::triggers::SingleTkMuEta_v0::version() const
{
	return 0;
}

l1menu::triggers::SingleTkMuEta::SingleTkMuEta()
	: threshold1_(20), muonQuality_(4), etaCut_(2.1)
{
	// No operation other than the initialiser list
}

const std::string l1menu::triggers::SingleTkMuEta::name() const
{
	return "L1_SingleTkMu";
}

const std::vector<std::string> l1menu::triggers::SingleTkMuEta::parameterNames() const
{
	std::vector<std::string> returnValue;
	returnValue.push_back("threshold1");
	returnValue.push_back("muonQuality");
	returnValue.push_back("etaCut");
	return returnValue;
}

float& l1menu::triggers::SingleTkMuEta::parameter( const std::string& parameterName )
{
	if( parameterName=="threshold1" ) return threshold1_;
	else if( parameterName=="muonQuality" ) return muonQuality_;
	else if( parameterName=="etaCut" ) return etaCut_;
	else throw std::logic_error( "Not a valid parameter name" );
}

const float& l1menu::triggers::SingleTkMuEta::parameter( const std::string& parameterName ) const
{
	if( parameterName=="threshold1" ) return threshold1_;
	else if( parameterName=="muonQuality" ) return muonQuality_;
	else if( parameterName=="etaCut" ) return etaCut_;
	else throw std::logic_error( "Not a valid parameter name" );
}
