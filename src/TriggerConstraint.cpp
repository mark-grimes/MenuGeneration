#include "l1menu/TriggerConstraint.h"

#include <stdexcept>

l1menu::TriggerConstraint::TriggerConstraint()
	: thresholdsLocked_(false), fractionOfTotalBandwidth_(-1)
{
	// No operation besides the initialiser list
}

bool l1menu::TriggerConstraint::thresholdsLocked() const
{
	return thresholdsLocked_;
}

void l1menu::TriggerConstraint::thresholdsLocked( bool thresholdsLocked )
{
	thresholdsLocked_=thresholdsLocked;
}

float l1menu::TriggerConstraint::fractionOfTotalBandwidth() const
{
	return fractionOfTotalBandwidth_;
}

void l1menu::TriggerConstraint::fractionOfTotalBandwidth( float fractionOfTotalBandwidth )
{
	if( fractionOfTotalBandwidth<0 || fractionOfTotalBandwidth>1 ) throw std::logic_error( "TriggerConstraint::fractionOfTotalBandwidth can only be set between zero and one" );
	fractionOfTotalBandwidth_=fractionOfTotalBandwidth;
}
