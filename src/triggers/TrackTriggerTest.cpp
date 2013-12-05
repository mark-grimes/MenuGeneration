#include "../implementation/RegisterTriggerMacro.h"
#include "l1menu/L1TriggerDPGEvent.h"

#include <stdexcept>
#include "UserCode/L1TriggerUpgrade/interface/L1AnalysisDataFormat.h"
#include "UserCode/L1TriggerUpgrade/interface/L1AnalysisL1TrackDataFormat.h"

#include "l1menu/ITrigger.h"

#include <string>
#include <vector>
#include <cmath>
#include <iostream>

namespace l1menu
{
	namespace triggers
	{
		/** @brief Base class for all versions of this testing class.
		 *
		 * This is just a proof of concept to make sure I can get data about the L1
		 * tracks all the way from the EDM files into this menu generation framework.
		 *
		 * Note that this class is abstract because it doesn't implement the "version"
		 * and "apply" methods. That's left up to the implementations of the different
		 * versions. The idea is that each time the code is changed, a new derived class
		 * will be created with the new "apply" method, and the "version" method returning
		 * a constant that has increased by one. That way it's easy to keep track of which
		 * results came from which algorithm.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 05/Dec/2013
		 */
		class TrackTriggerTest : public l1menu::ITrigger
		{
		public:
			TrackTriggerTest();

			virtual const std::string name() const;
			virtual const std::vector<std::string> parameterNames() const;
			virtual float& parameter( const std::string& parameterName );
			virtual const float& parameter( const std::string& parameterName ) const;
		protected:
			float threshold1_;
			float etaCut_;
		}; // end of the TrackTriggerTest base class

		/** @brief First version of the TrackTriggerTest trigger.
		 *
		 * @author Mark Grimes
		 * @date 05/Dec/2013
		 */
		class TrackTriggerTest_v0 : public TrackTriggerTest
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
		REGISTER_TRIGGER_AND_CUSTOMISE( TrackTriggerTest_v0,
			[]() // Use a lambda function to customise rather than creating a named function that never gets used again.
			{
				l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();
				TrackTriggerTest_v0 tempTriggerInstance;
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "threshold1", 100, 0, 100 );
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


bool l1menu::triggers::TrackTriggerTest_v0::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisL1TrackDataFormat& trackDataFormat=event.trackData();
	// Comment these two out so as to avoid unused variable compiler errors
	//const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	//const bool* PhysicsBits=event.physicsBits();

	for( unsigned int index=0; index<trackDataFormat.nTrackEG; ++index )
	{
		if( trackDataFormat.trackEG_bunchCrossing[index] != 0 ) continue;
		if( std::fabs(trackDataFormat.trackEG_eta[index]) > etaCut_ ) continue;

		if( trackDataFormat.trackEG_eT[index] >= threshold1_ ) return true;
	}

	// If control got this far then none of the objects fit the requirements, and
	// the trigger has failed.
	return false;
}

bool l1menu::triggers::TrackTriggerTest_v0::thresholdsAreCorrelated() const
{
	return false;
}

unsigned int l1menu::triggers::TrackTriggerTest_v0::version() const
{
	return 0;
}

l1menu::triggers::TrackTriggerTest::TrackTriggerTest()
	: threshold1_(50), etaCut_(3)
{
	// No operation other than the initialiser list
}

const std::string l1menu::triggers::TrackTriggerTest::name() const
{
	return "L1_TrackTriggerTest";
}

const std::vector<std::string> l1menu::triggers::TrackTriggerTest::parameterNames() const
{
	std::vector<std::string> returnValue;
	returnValue.push_back("threshold1");
	returnValue.push_back("etaCut");
	return returnValue;
}

float& l1menu::triggers::TrackTriggerTest::parameter( const std::string& parameterName )
{
	if( parameterName=="threshold1" ) return threshold1_;
	else if( parameterName=="etaCut" ) return etaCut_;
	else throw std::logic_error( "Not a valid parameter name" );
}

const float& l1menu::triggers::TrackTriggerTest::parameter( const std::string& parameterName ) const
{
	if( parameterName=="threshold1" ) return threshold1_;
	else if( parameterName=="etaCut" ) return etaCut_;
	else throw std::logic_error( "Not a valid parameter name" );
}
