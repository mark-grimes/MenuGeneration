#include "../implementation/RegisterTriggerMacro.h"
#include "l1menu/L1TriggerDPGEvent.h"
#include <TMath.h>

#include <stdexcept>
#include "UserCode/L1TriggerUpgrade/interface/L1AnalysisDataFormat.h"

#include "l1menu/ITrigger.h"

#include <string>
#include <vector>

namespace l1menu
{
	namespace triggers
	{
		/** @brief Base class for all versions of the TkEle_Tau trigger.
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
		class TkEle_Tau : public l1menu::ITrigger
		{
		public:
			TkEle_Tau();

			virtual const std::string name() const;
			virtual const std::vector<std::string> parameterNames() const;
			virtual float& parameter( const std::string& parameterName );
			virtual const float& parameter( const std::string& parameterName ) const;
		protected:
			float leg1threshold1_;
			float leg2threshold1_;
			float leg1regionCut_;
			float leg2regionCut_;
		}; // end of the TkEle_Tau base class


		/** @brief First version of the TkEle_Tau trigger.
		 *
		 * @author probably Brian Winer
		 * @date sometime
		 */
		class TkEle_Tau_v0 : public TkEle_Tau
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
		REGISTER_TRIGGER_AND_CUSTOMISE( TkEle_Tau_v0,
			[]() // Use a lambda function to customise rather than creating a named function that never gets used again.
			{
				l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();
				TkEle_Tau_v0 tempTriggerInstance;
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

bool l1menu::triggers::TkEle_Tau_v0::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw=PhysicsBits[0]; // ZeroBias
	if( !raw ) return false;

	bool ok = false;

	int NTkele=analysisDataFormat.NTkele;
	int Nj=analysisDataFormat.Njet;
	
	for( int ie=0; ie<NTkele; ie++ )
	{
		bool leg1=false;
		bool leg2=false;

		int bx=analysisDataFormat.BxTkel.at( ie );
		if( bx!=0 ) continue;
		float pt = analysisDataFormat.EtTkel.at( ie );
		float eta = analysisDataFormat.EtaTkel[ie];
		if (eta < leg1regionCut_ || eta > 21.-leg1regionCut_) continue;  // eta = 5 - 16  // eta = 5 - 16

		if( pt>=leg1threshold1_ ) {
		  
		       leg1=true;

			for (int ut=0; ut < Nj; ut++) {
				int bx = analysisDataFormat.Bxjet[ut];
				if (bx != 0) continue;
				bool isTauJet = analysisDataFormat.Taujet[ut];
				if (! isTauJet) continue;
				float pt2 = analysisDataFormat.Etjet[ut];    // the rank of the electron
				float eta2 = analysisDataFormat.Etajet[ut];
				if (eta2 < leg2regionCut_ || eta2 > 21.-leg2regionCut_) continue;  // eta = 5 - 16  // eta = 5 - 16

                                //remove overlap with simple delta R for now.
                                float delEta = fabs(analysisDataFormat.Etajet[ut]-analysisDataFormat.EtaTkel[ie]);
				float delPhi = fabs(analysisDataFormat.Phijet[ut]-analysisDataFormat.PhiTkel[ie]);
				if(delPhi>TMath::Pi()) delPhi = TMath::TwoPi() - delPhi;
				float delR = sqrt(delEta*delEta + delPhi*delPhi);

				if (pt2 >= leg2threshold1_ && delR>0.5) leg2 = true;
			}
                } 
                ok=( leg1 & leg2 );
		
	}

	
	return ok;
}



bool l1menu::triggers::TkEle_Tau_v0::thresholdsAreCorrelated() const
{
	return true;
}

unsigned int l1menu::triggers::TkEle_Tau_v0::version() const
{
	return 0;
}

l1menu::triggers::TkEle_Tau::TkEle_Tau()
	: leg1threshold1_(20), leg2threshold1_(20), leg1regionCut_(4.5), leg2regionCut_(4.5) 
{
	// No operation other than the initialiser list
}

const std::string l1menu::triggers::TkEle_Tau::name() const
{
	return "L1_TkEle_Tau";
}

const std::vector<std::string> l1menu::triggers::TkEle_Tau::parameterNames() const
{
	std::vector<std::string> returnValue;
	returnValue.push_back("leg1threshold1");
	returnValue.push_back("leg2threshold1");
	returnValue.push_back("leg1regionCut");
	returnValue.push_back("leg2regionCut");
	return returnValue;
}

float& l1menu::triggers::TkEle_Tau::parameter( const std::string& parameterName )
{
	if( parameterName=="leg1threshold1" ) return leg1threshold1_;
	else if( parameterName=="leg2threshold1" ) return leg2threshold1_;
	else if( parameterName=="leg1regionCut" ) return leg1regionCut_;
	else if( parameterName=="leg2regionCut" ) return leg2regionCut_;		
	else throw std::logic_error( "Not a valid parameter name" );
}

const float& l1menu::triggers::TkEle_Tau::parameter( const std::string& parameterName ) const
{
	if( parameterName=="leg1threshold1" ) return leg1threshold1_;
	else if( parameterName=="leg2threshold1" ) return leg2threshold1_;
	else if( parameterName=="leg1regionCut" ) return leg1regionCut_;
	else if( parameterName=="leg2regionCut" ) return leg2regionCut_;		
	else throw std::logic_error( "Not a valid parameter name" );
}
