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
		/** @brief Base class for all versions of the TkMu_Mu trigger.
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
		class TkMu_Mu : public l1menu::ITrigger
		{
		public:
			TkMu_Mu();

			virtual const std::string name() const;
			virtual const std::vector<std::string> parameterNames() const;
			virtual float& parameter( const std::string& parameterName );
			virtual const float& parameter( const std::string& parameterName ) const;
		protected:
			float leg1threshold1_;
			float leg2threshold1_;
			float muonQuality_;
		}; // end of the TkMu_Mu base class


		/** @brief First version of the TkMu_Mu trigger.
		 *
		 * @author probably Brian Winer
		 * @date sometime
		 */
		class TkMu_Mu_v0 : public TkMu_Mu
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
		REGISTER_TRIGGER_AND_CUSTOMISE( TkMu_Mu_v0,
			[]() // Use a lambda function to customise rather than creating a named function that never gets used again.
			{
				l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();
				TkMu_Mu_v0 tempTriggerInstance;
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

bool l1menu::triggers::TkMu_Mu_v0::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw=PhysicsBits[0]; // ZeroBias
	if( !raw ) return false;

	bool ok = false;

	int NTkmu=analysisDataFormat.NTkmu;
	int Nmu=analysisDataFormat.Nmu;
	
	for( int imu=0; imu<NTkmu; imu++ )
	{
		int bx=analysisDataFormat.BxTkmu.at( imu );
		if( bx!=0 ) continue;
		float pt=analysisDataFormat.PtTkmu.at( imu );
		//float eta=analysisDataFormat.Etamu.at( imu ); // Commented out to stop unused variable compile warning
		int qual=analysisDataFormat.QualTkmu.at( imu );
		if( qual<muonQuality_ ) continue;

		if( pt>=leg1threshold1_ ) {
		  

			for (int uj=0; uj < Nmu; uj++) {
				int bxj = analysisDataFormat.Bxmu[uj];
				if (bxj != 0) continue;

				float pt2=analysisDataFormat.Ptmu.at( uj );
				//float eta=analysisDataFormat.Etamu.at( uj ); // Commented out to stop unused variable compile warning
				int qual=analysisDataFormat.Qualmu.at( uj );
				if( qual<muonQuality_ ) continue;

                                //remove overlap with simple delta R for now.
                                float delEta = fabs(analysisDataFormat.Etamu[uj]-analysisDataFormat.EtaTkmu[imu]);
				float delPhi = fabs(analysisDataFormat.Phimu[uj]-analysisDataFormat.PhiTkmu[imu]);
				if(delPhi>TMath::Pi()) delPhi = TMath::TwoPi() - delPhi;
				float delR = sqrt(delEta*delEta + delPhi*delPhi);

				if (pt2 >= leg2threshold1_ && delR>0.5) ok = true;
			}
                } 

		
	}

	return ok;
}



bool l1menu::triggers::TkMu_Mu_v0::thresholdsAreCorrelated() const
{
	return true;
}

unsigned int l1menu::triggers::TkMu_Mu_v0::version() const
{
	return 0;
}

l1menu::triggers::TkMu_Mu::TkMu_Mu()
	: leg1threshold1_(20), leg2threshold1_(20), muonQuality_(4) 
{
	// No operation other than the initialiser list
}

const std::string l1menu::triggers::TkMu_Mu::name() const
{
	return "L1_TkMu_Mu";
}

const std::vector<std::string> l1menu::triggers::TkMu_Mu::parameterNames() const
{
	std::vector<std::string> returnValue;
	returnValue.push_back("leg1threshold1");
	returnValue.push_back("leg2threshold1");
	returnValue.push_back("muonQuality");
	return returnValue;
}

float& l1menu::triggers::TkMu_Mu::parameter( const std::string& parameterName )
{
	if( parameterName=="leg1threshold1" ) return leg1threshold1_;
	else if( parameterName=="leg2threshold1" ) return leg2threshold1_;
	else if( parameterName=="muonQuality" ) return muonQuality_;
	else throw std::logic_error( "Not a valid parameter name" );
}

const float& l1menu::triggers::TkMu_Mu::parameter( const std::string& parameterName ) const
{
	if( parameterName=="leg1threshold1" ) return leg1threshold1_;
	else if( parameterName=="leg2threshold1" ) return leg2threshold1_;
	else if( parameterName=="muonQuality" ) return muonQuality_;
	else throw std::logic_error( "Not a valid parameter name" );
}
