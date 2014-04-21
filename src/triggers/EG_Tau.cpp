#include "../implementation/RegisterTriggerMacro.h"
#include "l1menu/L1TriggerDPGEvent.h"

#include <stdexcept>
#include "UserCode/L1TriggerUpgrade/interface/L1AnalysisDataFormat.h"

#include "l1menu/ITrigger.h"

#include <string>
#include <vector>

namespace l1menu
{
	namespace triggers
	{
		/** @brief Base class for all versions of the EG_Tau trigger.
		 *
		 * Note that this class is abstract because it doesn't implement the "version"
		 * and "apply" methods. That's left up to the implementations of the different
		 * versions.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 02/Jun/2013
		 */
		class EG_Tau : public l1menu::ITrigger
		{
		public:
			EG_Tau();

			virtual const std::string name() const;
			virtual const std::vector<std::string> parameterNames() const;
			virtual float& parameter( const std::string& parameterName );
			virtual const float& parameter( const std::string& parameterName ) const;
		protected:
			float leg1threshold1_;
			float leg2threshold1_;
			float leg1regionCut_;
			float leg2regionCut_;
		}; // end of the EG_Tau base class

		/** @brief First version of the EG_Tau trigger.
		 *
		 * @author probably Brian Winer
		 * @date sometime
		 */
		class EG_Tau_v0 : public EG_Tau
		{
		public:
			virtual unsigned int version() const;
			virtual bool apply( const l1menu::L1TriggerDPGEvent& event ) const;
			virtual bool thresholdsAreCorrelated() const;
		}; // end of version 0 class


		/* The REGISTER_TRIGGER macro will make sure that the given trigger is registered in the
		 * l1menu::TriggerTable when the program starts. I also want to provide some suggested binning
		 * however. The REGISTER_TRIGGER_AND_CUSTOMISE macro does exactly the same but lets me pass
		 * a pointer to a function that will be called directly after the trigger has been registered
		 * at program startup. The function takes no parameters and returns void. In this case I'm
		 * giving it a lambda function.
		 */
		REGISTER_TRIGGER_AND_CUSTOMISE( EG_Tau_v0,
			[]() // Use a lambda function to customise rather than creating a named function that never gets used again.
			{
				l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();
				EG_Tau_v0 tempTriggerInstance;
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "leg1threshold1", 100, 0, 100 );
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "leg2threshold1", 100, 0, 100 );
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


bool l1menu::triggers::EG_Tau_v0::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw = PhysicsBits[0];    // ZeroBias
	if (! raw) return false;


	bool ok =false;


	int Nele = analysisDataFormat.Nele;
	for (int ue=0; ue < Nele; ue++) {
		int bx = analysisDataFormat.Bxel[ue];
		if (bx != 0 ) continue;
		float eta = analysisDataFormat.Etael[ue];
		if (eta < leg1regionCut_ || eta > 21.-leg1regionCut_) continue;  // eta = 5 - 16
		float rank = analysisDataFormat.Etel[ue];    // the rank of the electron
		float pt = rank ;
		if (pt >= leg1threshold1_) {


			// Now look for a tau that is not the same as this eg
			int Nj = analysisDataFormat.Njet ;
			for (int uj=0; uj < Nj; uj++) {
				bx = analysisDataFormat.Bxjet[uj];
				if (bx != 0) continue;
				bool isTauJet = analysisDataFormat.Taujet[uj];
				if (!isTauJet) continue;
				float tauEta = analysisDataFormat.Etajet[uj];
				if (tauEta < leg2regionCut_ || tauEta > 21.-leg2regionCut_) continue;  // tauEta = 5 - 16
				if (analysisDataFormat.Etajet[uj] == analysisDataFormat.Etael[ue] && analysisDataFormat.Phijet[uj] == analysisDataFormat.Phiel[ue] ) continue;
				float rankt = analysisDataFormat.Etjet[uj];
				float ptt = rankt; //CorrectedL1JetPtByGCTregions(analysisDataFormat.Etajet[uj],rank*4.,theL1JetCorrection);
				if (ptt >= leg2threshold1_) ok = true;
			}

		}
	}  // end loop over EM objects

	return ok;
}

bool l1menu::triggers::EG_Tau_v0::thresholdsAreCorrelated() const
{
	return true;
}

unsigned int l1menu::triggers::EG_Tau_v0::version() const
{
	return 0;
}

l1menu::triggers::EG_Tau::EG_Tau()
	: leg1threshold1_(20), leg2threshold1_(20), leg1regionCut_(4.5), leg2regionCut_(4.5)
{
	// No operation other than the initialiser list
}

const std::string l1menu::triggers::EG_Tau::name() const
{
	return "L1_EG_Tau";
}

const std::vector<std::string> l1menu::triggers::EG_Tau::parameterNames() const
{
	std::vector<std::string> returnValue;
	returnValue.push_back("leg1threshold1");
	returnValue.push_back("leg1regionCut");
	returnValue.push_back("leg2threshold1");
	returnValue.push_back("leg2regionCut");
	return returnValue;
}

float& l1menu::triggers::EG_Tau::parameter( const std::string& parameterName )
{
	if( parameterName=="leg1threshold1" ) return leg1threshold1_;
	else if( parameterName=="leg2threshold1" ) return leg2threshold1_;
	else if( parameterName=="leg1regionCut" ) return leg1regionCut_;
	else if( parameterName=="leg2regionCut" ) return leg2regionCut_;
	else throw std::logic_error( "Not a valid parameter name (\""+parameterName+"\")" );
}

const float& l1menu::triggers::EG_Tau::parameter( const std::string& parameterName ) const
{
	if( parameterName=="leg1threshold1" ) return leg1threshold1_;
	else if( parameterName=="leg2threshold1" ) return leg2threshold1_;
	else if( parameterName=="leg1regionCut" ) return leg1regionCut_;
	else if( parameterName=="leg2regionCut" ) return leg2regionCut_;
	else throw std::logic_error( "Not a valid parameter name (\""+parameterName+"\")" );
}
