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
		/** @brief Base class for all versions of the DoubleTkEle trigger.
		 *
		 * Note that this class is abstract because it doesn't implement the "version"
		 * and "apply" methods. That's left up to the implementations of the different
		 * versions.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 02/Jun/2013
		 */
		class DoubleTkEle : public l1menu::ITrigger
		{
		public:
			DoubleTkEle();

			virtual const std::string name() const;
			virtual const std::vector<std::string> parameterNames() const;
			virtual float& parameter( const std::string& parameterName );
			virtual const float& parameter( const std::string& parameterName ) const;
		protected:
			float leg1threshold1_;
			float leg2threshold1_;
			float regionCut_;
		}; // end of the DoubleTkEle base class

		/** @brief First version of the DoubleTkEle trigger.
		 *
		 * @author probably Brian Winer
		 * @date sometime
		 */
		class DoubleTkEle_v0 : public DoubleTkEle
		{
		public:
			virtual unsigned int version() const;
			virtual bool apply( const l1menu::L1TriggerDPGEvent& event ) const;
			virtual bool thresholdsAreCorrelated() const;
		}; // end of version 0 class

		/** @brief Second version of the DoubleTkEle trigger.
		 *             --> Used TkEle Collection with lower Pt cut
		 * @author probably Brian Winer
		 * @date sometime
		 */
		class DoubleTkEle_v1 : public DoubleTkEle
		{
		public:
			virtual unsigned int version() const;
			virtual bool apply( const l1menu::L1TriggerDPGEvent& event ) const;
			virtual bool thresholdsAreCorrelated() const;
		}; // end of version 1 class


		/* The REGISTER_TRIGGER macro will make sure that the given trigger is registered in the
		 * l1menu::TriggerTable when the program starts. I also want to provide some suggested binning
		 * however. The REGISTER_TRIGGER_AND_CUSTOMISE macro does exactly the same but lets me pass
		 * a pointer to a function that will be called directly after the trigger has been registered
		 * at program startup. The function takes no parameters and returns void. In this case I'm
		 * giving it a lambda function.
		 */
		REGISTER_TRIGGER_AND_CUSTOMISE( DoubleTkEle_v1,
			[]() // Use a lambda function to customise rather than creating a named function that never gets used again.
			{
				l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();
				DoubleTkEle_v1 tempTriggerInstance;
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "leg1threshold1", 100, 0, 100 );
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "leg2threshold1", 100, 0, 100 );
			} // End of customisation lambda function
		) // End of REGISTER_TRIGGER_AND_CUSTOMISE macro call
		REGISTER_TRIGGER( DoubleTkEle_v0 )


	} // end of namespace triggers

} // end of namespace l1menu


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//---------------  Definitions below         ---------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


bool l1menu::triggers::DoubleTkEle_v0::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw = PhysicsBits[0];   // ZeroBias
	if (! raw) return false;

	int n1=0;
	int n2=0;
	int Nele = analysisDataFormat.NTkele;
	for (int ue=0; ue < Nele; ue++) {
		int bx = analysisDataFormat.BxTkel[ue];
		if (bx != 0) continue;
		float eta = analysisDataFormat.EtaTkel[ue];
		if (eta < regionCut_ || eta > 21.-regionCut_) continue;  // eta = 5 - 16
		float rank = analysisDataFormat.EtTkel[ue];    // the rank of the electron
		float pt = rank ;
		if (pt >= leg1threshold1_) n1++;
		if (pt >= leg2threshold1_) n2++;
	}  // end loop over EM objects

	bool ok = ( n1 >= 1 && n2 >= 2) ;
	//if(ok) printf("Found doubleEG event Run %i Event %i \n",event_->run,event_->event);
	return ok;
}

bool l1menu::triggers::DoubleTkEle_v0::thresholdsAreCorrelated() const
{
	return false;
}

unsigned int l1menu::triggers::DoubleTkEle_v0::version() const
{
	return 0;
}

bool l1menu::triggers::DoubleTkEle_v1::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw = PhysicsBits[0];   // ZeroBias
	if (! raw) return false;

	int n1=0;
	int n2=0;
	int Nele = analysisDataFormat.NTkele2;
	for (int ue=0; ue < Nele; ue++) {
		int bx = analysisDataFormat.BxTkel2[ue];
		if (bx != 0) continue;
		float eta = analysisDataFormat.EtaTkel2[ue];
		if (eta < regionCut_ || eta > 21.-regionCut_) continue;  // eta = 5 - 16
		float rank = analysisDataFormat.EtTkel2[ue];    // the rank of the electron
		float pt = rank ;
		if (pt >= leg1threshold1_) n1++;
		if (pt >= leg2threshold1_) n2++;
	}  // end loop over EM objects

	bool ok = ( n1 >= 1 && n2 >= 2) ;
	//if(ok) printf("Found doubleEG event Run %i Event %i \n",event_->run,event_->event);
	return ok;
}

bool l1menu::triggers::DoubleTkEle_v1::thresholdsAreCorrelated() const
{
	return false;
}

unsigned int l1menu::triggers::DoubleTkEle_v1::version() const
{
	return 1;
}


l1menu::triggers::DoubleTkEle::DoubleTkEle()
	: leg1threshold1_(20), leg2threshold1_(20), regionCut_(4.5)
{
	// No operation other than the initialiser list
}

const std::string l1menu::triggers::DoubleTkEle::name() const
{
	return "L1_DoubleTkEle";
}

const std::vector<std::string> l1menu::triggers::DoubleTkEle::parameterNames() const
{
	std::vector<std::string> returnValue;
	returnValue.push_back("leg1threshold1");
	returnValue.push_back("leg2threshold1");
	returnValue.push_back("regionCut");
	return returnValue;
}

float& l1menu::triggers::DoubleTkEle::parameter( const std::string& parameterName )
{
	if( parameterName=="leg1threshold1" ) return leg1threshold1_;
	else if( parameterName=="leg2threshold1" ) return leg2threshold1_;
	else if( parameterName=="regionCut" ) return regionCut_;
	else throw std::logic_error( "Not a valid parameter name" );
}

const float& l1menu::triggers::DoubleTkEle::parameter( const std::string& parameterName ) const
{
	if( parameterName=="leg1threshold1" ) return leg1threshold1_;
	else if( parameterName=="leg2threshold1" ) return leg2threshold1_;
	else if( parameterName=="regionCut" ) return regionCut_;
	else throw std::logic_error( "Not a valid parameter name" );
}
