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
		/** @brief Base class for all versions of the EG_JetCentral trigger.
		 *
		 * Note that this class is abstract because it doesn't implement the "version"
		 * and "apply" methods. That's left up to the implementations of the different
		 * versions.
		 *
		 * It would have been nicer to implement this as a derived class of CrossTrigger,
		 * but there is a check on whether the electron and jet have the same value for
		 * eta and phi, and so the two triggers are not independent.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 02/Jun/2013
		 */
		class EG_JetCentral : public l1menu::ITrigger
		{
		public:
			EG_JetCentral();

			virtual const std::string name() const;
			virtual const std::vector<std::string> parameterNames() const;
			virtual float& parameter( const std::string& parameterName );
			virtual const float& parameter( const std::string& parameterName ) const;
		protected:
			float leg1threshold1_;
			float leg2threshold1_;
			float leg1regionCut_;
			float leg2regionCut_;
		}; // end of the EG_JetCentral base class

		/** @brief Second version of the EG_JetCentral trigger.
		 *
		 * An error was spotted in the logic that checks if the jet is the same object as the electron.
		 * The old logic required that both the eta and phi were different, whereas only one needs to
		 * be different for them to be different objects.
		 *
		 * @author as for v0, but logic error spotted by Brian Winer's unnamed student.
		 * @date 09/Sep/2013
		 */
		class EG_JetCentral_v1 : public EG_JetCentral
		{
		public:
			virtual unsigned int version() const;
			virtual bool apply( const l1menu::L1TriggerDPGEvent& event ) const;
			virtual bool thresholdsAreCorrelated() const;
		}; // end of version 0 class

		/** @brief First version of the EG_JetCentral trigger.
		 *
		 * @author probably Brian Winer
		 * @date sometime
		 */
		class EG_JetCentral_v0 : public EG_JetCentral
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
		REGISTER_TRIGGER_AND_CUSTOMISE( EG_JetCentral_v1,
			[]() // Use a lambda function to customise rather than creating a named function that never gets used again.
			{
				l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();
				EG_JetCentral_v1 tempTriggerInstance;
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "leg1threshold1", 100, 0, 100 );
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "leg2threshold1", 100, 0, 100 );
			} // End of customisation lambda function
		) // End of REGISTER_TRIGGER_AND_CUSTOMISE macro call

		// Don't need to register suggested binning for this version, because the binning above will be used for all versions.
		REGISTER_TRIGGER( EG_JetCentral_v0 )


	} // end of namespace triggers

} // end of namespace l1menu


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//---------------  Definitions below         ---------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

bool l1menu::triggers::EG_JetCentral_v1::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw = PhysicsBits[0];   // ZeroBias
	if (! raw) return false;

	bool jet=false;
	bool eg = false;
	bool ok = false;

	int Nj = analysisDataFormat.Njet ;

	int Nele = analysisDataFormat.Nele;
	for (int ue=0; ue < Nele; ue++) {
		int bx = analysisDataFormat.Bxel[ue];
		if (bx != 0 ) continue;
		float eta = analysisDataFormat.Etael[ue];
		if (eta < leg1regionCut_ || eta > 21.-leg1regionCut_) continue;  // eta = 5 - 16
		float rank = analysisDataFormat.Etel[ue];    // the rank of the electron
		float pt = rank ;
		if (pt >= leg1threshold1_){

			eg = true;

			for (int uj=0; uj < Nj; uj++) {
				int bxj = analysisDataFormat.Bxjet[uj];
				if (bxj != 0) continue;
				bool isFwdJet = analysisDataFormat.Fwdjet[uj];
				if (isFwdJet) continue;
				bool isTauJet = analysisDataFormat.Taujet[uj];
				if (isTauJet) continue;
				float rankj = analysisDataFormat.Etjet[uj];
				float ptj = rankj; //CorrectedL1JetPtByGCTregions(analysisDataFormat.Etajet[uj],rank*4.,theL1JetCorrection);

				if (analysisDataFormat.Etajet[uj] < leg2regionCut_ || analysisDataFormat.Etajet[uj] > 21.-leg2regionCut_) continue;
				if (ptj >= leg2threshold1_ &&
					!(analysisDataFormat.Etajet[uj]==analysisDataFormat.Etael[ue] &&
					  analysisDataFormat.Phijet[uj]==analysisDataFormat.Phiel[ue]) ) jet = true;
			}

			ok = eg && jet;
		} // if good EG
	}  // end loop over EM objects

	return ok;
}

bool l1menu::triggers::EG_JetCentral_v1::thresholdsAreCorrelated() const
{
	return true;
}

unsigned int l1menu::triggers::EG_JetCentral_v1::version() const
{
	return 1;
}

bool l1menu::triggers::EG_JetCentral_v0::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw = PhysicsBits[0];   // ZeroBias
	if (! raw) return false;

	bool ok = false;

	int Nj = analysisDataFormat.Njet ;

	int Nele = analysisDataFormat.Nele;
	for (int ue=0; ue < Nele; ue++) {
		int bx = analysisDataFormat.Bxel[ue];
		if (bx != 0 ) continue;
		float eta = analysisDataFormat.Etael[ue];
		if (eta < leg1regionCut_ || eta > 21.-leg1regionCut_) continue;  // eta = 5 - 16
		float rank = analysisDataFormat.Etel[ue];    // the rank of the electron
		float pt = rank ;
		if (pt >= leg1threshold1_){


			for (int uj=0; uj < Nj; uj++) {
				int bxj = analysisDataFormat.Bxjet[uj];
				if (bxj != 0) continue;
				bool isFwdJet = analysisDataFormat.Fwdjet[uj];
				if (isFwdJet) continue;
				bool isTauJet = analysisDataFormat.Taujet[uj];
				if (isTauJet) continue;
				float rankj = analysisDataFormat.Etjet[uj];
				float ptj = rankj; //CorrectedL1JetPtByGCTregions(analysisDataFormat.Etajet[uj],rank*4.,theL1JetCorrection);

				if (analysisDataFormat.Etajet[uj] < leg2regionCut_ || analysisDataFormat.Etajet[uj] > 21.-leg2regionCut_) continue;
				if (ptj >= leg2threshold1_ &&
					(analysisDataFormat.Etajet[uj]!=analysisDataFormat.Etael[ue]) &&
					(analysisDataFormat.Phijet[uj]!=analysisDataFormat.Phiel[ue]) ) ok = true;
			}

		
		} // if good EG
	}  // end loop over EM objects

	return ok;
}

bool l1menu::triggers::EG_JetCentral_v0::thresholdsAreCorrelated() const
{
	return true;
}

unsigned int l1menu::triggers::EG_JetCentral_v0::version() const
{
	return 0;
}

l1menu::triggers::EG_JetCentral::EG_JetCentral()
	: leg1threshold1_(20), leg2threshold1_(20), leg1regionCut_(4.5), leg2regionCut_(4.5)
{
	// No operation other than the initialiser list
}

const std::string l1menu::triggers::EG_JetCentral::name() const
{
	return "L1_SingleEG_CJet";
}

const std::vector<std::string> l1menu::triggers::EG_JetCentral::parameterNames() const
{
	std::vector<std::string> returnValue;
	returnValue.push_back("leg1threshold1");
	returnValue.push_back("leg1regionCut");
	returnValue.push_back("leg2threshold1");
	returnValue.push_back("leg2regionCut");
	return returnValue;
}

float& l1menu::triggers::EG_JetCentral::parameter( const std::string& parameterName )
{
	if( parameterName=="leg1threshold1" ) return leg1threshold1_;
	else if( parameterName=="leg1regionCut" ) return leg1regionCut_;
	else if( parameterName=="leg2threshold1" ) return leg2threshold1_;
	else if( parameterName=="leg2regionCut" ) return leg2regionCut_;
	else throw std::logic_error( "Not a valid parameter name" );
}

const float& l1menu::triggers::EG_JetCentral::parameter( const std::string& parameterName ) const
{
	if( parameterName=="leg1threshold1" ) return leg1threshold1_;
	else if( parameterName=="leg1regionCut" ) return leg1regionCut_;
	else if( parameterName=="leg2threshold1" ) return leg2threshold1_;
	else if( parameterName=="leg2regionCut" ) return leg2regionCut_;
	else throw std::logic_error( "Not a valid parameter name" );
}
