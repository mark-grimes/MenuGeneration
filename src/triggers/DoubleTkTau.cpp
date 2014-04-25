#include "../implementation/RegisterTriggerMacro.h"
#include "l1menu/L1TriggerDPGEvent.h"

#include <stdexcept>
#include "UserCode/L1TriggerUpgrade/interface/L1AnalysisDataFormat.h"

#include "l1menu/ITrigger.h"

#include <string>
#include <vector>
#include <cmath>

namespace l1menu
{
	namespace triggers
	{
		/** @brief Base class for all versions of the DoubleTkTau trigger.
		 *
		 * Note that this class is abstract because it doesn't implement the "version"
		 * and "apply" methods. That's left up to the implementations of the different
		 * versions.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 02/Jun/2013
		 */
		class DoubleTkTau : public l1menu::ITrigger
		{
		public:
			DoubleTkTau();

			virtual const std::string name() const;
			virtual const std::vector<std::string> parameterNames() const;
			virtual float& parameter( const std::string& parameterName );
			virtual const float& parameter( const std::string& parameterName ) const;
		protected:
			float leg1threshold1_;
			float leg2threshold1_;
			float regionCut_;
			float zVtxCut_;
		}; // end of the DoubleTkTau base class

		/** @brief First version of the DoubleTkTau trigger.
		 *
		 * @author probably Brian Winer
		 * @date sometime
		 */
		class DoubleTkTau_v0 : public DoubleTkTau
		{
		public:
			virtual unsigned int version() const;
			virtual bool apply( const l1menu::L1TriggerDPGEvent& event ) const;
			virtual bool thresholdsAreCorrelated() const;
		}; // end of version 0 class
		/** @brief First version of the DoubleTkTau trigger.
		 *         --> Add zvtx cut
		 * @author probably Brian Winer
		 * @date sometime
		 */
		class DoubleTkTau_v1 : public DoubleTkTau
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
		REGISTER_TRIGGER_AND_CUSTOMISE( DoubleTkTau_v1,
			[]() // Use a lambda function to customise rather than creating a named function that never gets used again.
			{
				l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();
				DoubleTkTau_v1 tempTriggerInstance;
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "leg1threshold1", 100, 0, 100 );
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "leg2threshold1", 100, 0, 100 );
			} // End of customisation lambda function
		) // End of REGISTER_TRIGGER_AND_CUSTOMISE macro call
		REGISTER_TRIGGER( DoubleTkTau_v0 )


	} // end of namespace triggers

} // end of namespace l1menu


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//---------------  Definitions below         ---------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


bool l1menu::triggers::DoubleTkTau_v0::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw = PhysicsBits[0];   // ZeroBias
	if (! raw) return false;

        bool ok = false;
	int n1=0;
	int n2=0;
	int Ntau = analysisDataFormat.NTktau;
	for (int ue=0; ue < Ntau; ue++) {
		int bx = analysisDataFormat.BxTktau[ue];
		if (bx != 0) continue;
		float eta = analysisDataFormat.EtaTktau[ue];
		if (eta < regionCut_ || eta > 21.-regionCut_) continue;  // eta = 5 - 16
		float rank = analysisDataFormat.EtTktau[ue];    // the rank of the electron
		float pt = rank ;
		if (pt >= leg1threshold1_) n1++;
		if (pt >= leg2threshold1_) n2++;
	}  // end loop over EM objects

        ok = ( n1 >= 1 && n2 >= 2);
	
	return ok;
}

bool l1menu::triggers::DoubleTkTau_v0::thresholdsAreCorrelated() const
{
	return false;
}

unsigned int l1menu::triggers::DoubleTkTau_v0::version() const
{
	return 0;
}

bool l1menu::triggers::DoubleTkTau_v1::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw = PhysicsBits[0];   // ZeroBias
	if (! raw) return false;

        bool ok = false;

	int Ntau = analysisDataFormat.NTktau;
	for (int ue=0; ue < Ntau; ue++) {
		int bx = analysisDataFormat.BxTktau[ue];
		if (bx != 0) continue;
		float eta = analysisDataFormat.EtaTktau[ue];
		if (eta < regionCut_ || eta > 21.-regionCut_) continue;  // eta = 5 - 16
		float rank = analysisDataFormat.EtTktau[ue];    // the rank of the electron
		float pt = rank ;
		if (pt >= leg1threshold1_) {
		
		      float tauZvtx = analysisDataFormat.zVtxTktau[ue];
		      for(int ue2=0; ue2< Ntau; ue2++) {
		          if( (ue2 != ue) &&
			     fabs(tauZvtx - analysisDataFormat.zVtxTktau[ue2]) < zVtxCut_)  {
			      if (analysisDataFormat.BxTktau[ue2]!= 0) continue;
			      float eta2 = analysisDataFormat.EtaTktau[ue2];
			      if (eta2 < regionCut_ || eta2 > 21.-regionCut_) continue;  // eta = 5 - 16
			      float pt2 = analysisDataFormat.EtTktau[ue2];     
		              if (pt2 >= leg2threshold1_ ) ok=true;
			   }//end if vtx compatability  
		      }	//end loop over second object
		}  //end pt threshold

	}  // end loop over EM objects

	
	return ok;
}

bool l1menu::triggers::DoubleTkTau_v1::thresholdsAreCorrelated() const
{
	return false;
}

unsigned int l1menu::triggers::DoubleTkTau_v1::version() const
{
	return 1;
}



l1menu::triggers::DoubleTkTau::DoubleTkTau()
	: leg1threshold1_(20), leg2threshold1_(20), regionCut_(4.5), zVtxCut_(99999.)
{
	// No operation other than the initialiser list
}

const std::string l1menu::triggers::DoubleTkTau::name() const
{
	return "L1_DoubleTkTau";
}

const std::vector<std::string> l1menu::triggers::DoubleTkTau::parameterNames() const
{
	std::vector<std::string> returnValue;
	returnValue.push_back("leg1threshold1");
	returnValue.push_back("leg2threshold1");
	returnValue.push_back("regionCut");
	returnValue.push_back("zVtxCut");	
	return returnValue;
}

float& l1menu::triggers::DoubleTkTau::parameter( const std::string& parameterName )
{
	if( parameterName=="leg1threshold1" ) return leg1threshold1_;
	else if( parameterName=="leg2threshold1" ) return leg2threshold1_;
	else if( parameterName=="regionCut" ) return regionCut_;
	else if( parameterName=="zVtxCut" ) return zVtxCut_;	
	else throw std::logic_error( "Not a valid parameter name" );
}

const float& l1menu::triggers::DoubleTkTau::parameter( const std::string& parameterName ) const
{
	if( parameterName=="leg1threshold1" ) return leg1threshold1_;
	else if( parameterName=="leg2threshold1" ) return leg2threshold1_;
	else if( parameterName=="regionCut" ) return regionCut_;
	else if( parameterName=="zVtxCut" ) return zVtxCut_;	
	else throw std::logic_error( "Not a valid parameter name" );
}
