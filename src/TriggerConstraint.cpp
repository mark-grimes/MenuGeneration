#include "l1menu/TriggerConstraint.h"

#include <stdexcept>
#include <iostream>

l1menu::TriggerConstraint::TriggerConstraint()
	: type_(Type::FIXED_THRESHOLDS), value_(-1)
{
	// No operation besides the initialiser list
}

l1menu::TriggerConstraint::Type l1menu::TriggerConstraint::type() const
{
	return type_;
}

void l1menu::TriggerConstraint::type( Type newType )
{
	type_=newType;
}

float l1menu::TriggerConstraint::value() const
{
	return value_;
}

void l1menu::TriggerConstraint::value( float newValue )
{
	value_=newValue;
}

std::ostream& operator<<( std::ostream& stream, const l1menu::TriggerConstraint::Type& object )
{
	if( object==l1menu::TriggerConstraint::Type::FIXED_THRESHOLDS ) stream << "FIXED_THRESHOLDS";
	else if( object==l1menu::TriggerConstraint::Type::FIXED_RATE ) stream << "FIXED_RATE";
	else if( object==l1menu::TriggerConstraint::Type::FRACTION_OF_BANDWIDTH ) stream << "FRACTION_OF_BANDWIDTH";
	else stream << "[stream output not updated for this value]";
	return stream;
}
