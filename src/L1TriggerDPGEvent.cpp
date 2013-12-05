#include "l1menu/L1TriggerDPGEvent.h"

#include "l1menu/ITrigger.h"
#include "UserCode/L1TriggerUpgrade/interface/L1AnalysisDataFormat.h"
#include "UserCode/L1TriggerUpgrade/interface/L1AnalysisL1TrackDataFormat.h"

namespace l1menu
{
	class L1TriggerDPGEventPrivateMembers
	{
	public:
		L1TriggerDPGEventPrivateMembers( const l1menu::ISample* pParentSample ) : pParentSample_(pParentSample) {}
		L1Analysis::L1AnalysisDataFormat rawEvent;
		L1Analysis::L1AnalysisL1TrackDataFormat trackData;
		bool physicsBits[128];
		float weight;
		const l1menu::ISample* pParentSample_;
	};
}


l1menu::L1TriggerDPGEvent::L1TriggerDPGEvent( const l1menu::ISample& parentSample ) : pImple_( new L1TriggerDPGEventPrivateMembers(&parentSample) )
{
	pImple_->weight=1;
}

l1menu::L1TriggerDPGEvent::L1TriggerDPGEvent( const L1TriggerDPGEvent& otherEvent ) : pImple_( new L1TriggerDPGEventPrivateMembers(*otherEvent.pImple_) )
{
	// No operation besides the initialiser list
}

l1menu::L1TriggerDPGEvent::L1TriggerDPGEvent( L1TriggerDPGEvent&& otherEvent ) noexcept : pImple_( std::move(otherEvent.pImple_) )
{
	// No operation besides the initialiser list
}

l1menu::L1TriggerDPGEvent& l1menu::L1TriggerDPGEvent::operator=( const L1TriggerDPGEvent& otherEvent )
{
	*pImple_=*otherEvent.pImple_;
	return *this;
}

l1menu::L1TriggerDPGEvent& l1menu::L1TriggerDPGEvent::operator=( L1TriggerDPGEvent&& otherEvent ) noexcept
{
	pImple_=std::move(otherEvent.pImple_);
	return *this;
}

l1menu::L1TriggerDPGEvent::~L1TriggerDPGEvent()
{
	// No operation
}

L1Analysis::L1AnalysisDataFormat& l1menu::L1TriggerDPGEvent::rawEvent()
{
	return pImple_->rawEvent;
}

const L1Analysis::L1AnalysisDataFormat& l1menu::L1TriggerDPGEvent::rawEvent() const
{
	return pImple_->rawEvent;
}

bool* l1menu::L1TriggerDPGEvent::physicsBits()
{
	return pImple_->physicsBits;
}

const bool* l1menu::L1TriggerDPGEvent::physicsBits() const
{
	return pImple_->physicsBits;
}

L1Analysis::L1AnalysisL1TrackDataFormat& l1menu::L1TriggerDPGEvent::trackData()
{
	return pImple_->trackData;
}

const L1Analysis::L1AnalysisL1TrackDataFormat& l1menu::L1TriggerDPGEvent::trackData() const
{
	return pImple_->trackData;
}

void l1menu::L1TriggerDPGEvent::setWeight( float weight )
{
	pImple_->weight=weight;
}

bool l1menu::L1TriggerDPGEvent::passesTrigger( const l1menu::ITrigger& trigger ) const
{
	// This is an IEvent method, but ITrigger has a method that can
	// handle L1TriggerDPGEvent concrete objects, so call that
	// reflexively.
	return trigger.apply( *this );
}

float l1menu::L1TriggerDPGEvent::weight() const
{
	return pImple_->weight;
}

const l1menu::ISample& l1menu::L1TriggerDPGEvent::sample() const
{
	return *pImple_->pParentSample_;
}
