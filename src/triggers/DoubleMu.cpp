#include "DoubleMu.h"

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
		REGISTER_TRIGGER_AND_CUSTOMISE( DoubleMu_v0,
			[]() // Use a lambda function to customise rather than creating a named function that never gets used again.
			{
				l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();
				DoubleMu_v0 tempTriggerInstance;
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "threshold1", 100, 0, 100 );
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "threshold2", 100, 0, 100 );
			} // End of customisation lambda function
		) // End of REGISTER_TRIGGER_AND_CUSTOMISE macro call


	} // end of namespace triggers

} // end of namespace l1menu


bool l1menu::triggers::DoubleMu_v0::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw=PhysicsBits[0]; // ZeroBias
	if( !raw ) return false;

	int n1=0;
	int n2=0;
	int Nmu=analysisDataFormat.Nmu;
	for( int imu=0; imu<Nmu; imu++ )
	{
		int bx=analysisDataFormat.Bxmu.at( imu );
		if( bx!=0 ) continue;
		float pt=analysisDataFormat.Ptmu.at( imu );
		//float eta=analysisDataFormat.Etamu.at( imu ); // Commented out to stop unused variable compile warning
		int qual=analysisDataFormat.Qualmu.at( imu );
		if( qual<muonQuality_ ) continue;

		if( pt>=threshold1_ ) n1++;
		if( pt>=threshold2_ ) n2++;
	}

	bool ok=( n1>=1 && n2>=2 );
	return ok;
}

bool l1menu::triggers::DoubleMu_v0::thresholdsAreCorrelated() const
{
	return true;
}

unsigned int l1menu::triggers::DoubleMu_v0::version() const
{
	return 0;
}

l1menu::triggers::DoubleMu::DoubleMu()
	: threshold1_(20), threshold2_(20), muonQuality_(4)
{
	// No operation other than the initialiser list
}

const std::string l1menu::triggers::DoubleMu::name() const
{
	return "L1_DoubleMu";
}

const std::vector<std::string> l1menu::triggers::DoubleMu::parameterNames() const
{
	std::vector<std::string> returnValue;
	returnValue.push_back("threshold1");
	returnValue.push_back("threshold2");
	returnValue.push_back("muonQuality");
	return returnValue;
}

float& l1menu::triggers::DoubleMu::parameter( const std::string& parameterName )
{
	if( parameterName=="threshold1" ) return threshold1_;
	else if( parameterName=="threshold2" ) return threshold2_;
	else if( parameterName=="muonQuality" ) return muonQuality_;
	else throw std::logic_error( "Not a valid parameter name" );
}

const float& l1menu::triggers::DoubleMu::parameter( const std::string& parameterName ) const
{
	if( parameterName=="threshold1" ) return threshold1_;
	else if( parameterName=="threshold2" ) return threshold2_;
	else if( parameterName=="muonQuality" ) return muonQuality_;
	else throw std::logic_error( "Not a valid parameter name" );
}
