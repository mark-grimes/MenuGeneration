#include "MultiTkJet.h"

#include <stdexcept>
#include <algorithm>
#include "../implementation/RegisterTriggerMacro.h"


namespace l1menu
{
	namespace triggers
	{

		/** @brief Essentially a copy of the MultiJet trigger, but overriding the name and forcing
		 * the numberOfJets parameter to always be four.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 04/Jun/2013
		 */
		class QuadTkJetVtx_v0 : public l1menu::triggers::MultiTkJet_v1
		{
		public:
			QuadTkJetVtx_v0();
			virtual const std::string name() const;
			virtual unsigned int version() const;

			// These implementations are just to remove the option of changing the
			// numberOfJets parameter.
			virtual const std::vector<std::string> parameterNames() const;
			virtual float& parameter( const std::string& parameterName );
			virtual const float& parameter( const std::string& parameterName ) const;
		}; // end of version 0 class

		/* The REGISTER_TRIGGER macro will make sure that the given trigger is registered in the
		 * l1menu::TriggerTable when the program starts. I also want to provide some suggested binning
		 * however. The REGISTER_TRIGGER_AND_CUSTOMISE macro does exactly the same but lets me pass
		 * a pointer to a function that will be called directly after the trigger has been registered
		 * at program startup. The function takes no parameters and returns void. In this case I'm
		 * giving it a lambda function.
		 */
		REGISTER_TRIGGER_AND_CUSTOMISE( QuadTkJetVtx_v0,
			[]() // Use a lambda function to customise rather than creating a named function that never gets used again.
			{
				l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();
				QuadTkJetVtx_v0 tempTriggerInstance;
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "threshold1", 100, 0, 90 );
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "threshold2", 100, 0, 70 );
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "threshold3", 100, 0, 50 );
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "threshold4", 100, 0, 40 );
			} // End of customisation lambda function
		) // End of REGISTER_TRIGGER_AND_CUSTOMISE macro call

	} // end of namespace triggers

} // end of namespace l1menu

l1menu::triggers::QuadTkJetVtx_v0::QuadTkJetVtx_v0()
{
	numberOfJets_=4;
}

const std::string l1menu::triggers::QuadTkJetVtx_v0::name() const
{
	return "L1_QuadTkJetVtx";
}

unsigned int l1menu::triggers::QuadTkJetVtx_v0::version() const
{
	return 0;
}

const std::vector<std::string> l1menu::triggers::QuadTkJetVtx_v0::parameterNames() const
{
	std::vector<std::string> returnValue=MultiTkJet::parameterNames();

	// Remove the numberOfJets parameter from the list
	const auto& iFindResult=std::find( returnValue.begin(), returnValue.end(), "numberOfJets" );
	returnValue.erase( iFindResult );

	return returnValue;
}

float& l1menu::triggers::QuadTkJetVtx_v0::parameter( const std::string& parameterName )
{
	if( parameterName!="numberOfJets" ) return MultiTkJet::parameter(parameterName);
	else throw std::logic_error( "Not a valid parameter name" );
}

const float& l1menu::triggers::QuadTkJetVtx_v0::parameter( const std::string& parameterName ) const
{
	if( parameterName!="numberOfJets" ) return MultiTkJet::parameter(parameterName);
	else throw std::logic_error( "Not a valid parameter name" );
}
