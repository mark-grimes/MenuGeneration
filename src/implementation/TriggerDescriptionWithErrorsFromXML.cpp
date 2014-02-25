#include "TriggerDescriptionWithErrorsFromXML.h"

#include <stdexcept>
#include "l1menu/tools/XMLElement.h"

l1menu::implementation::TriggerDescriptionWithErrorsFromXML::TriggerDescriptionWithErrorsFromXML( const l1menu::tools::XMLElement& xmlDescription )
{
	if( xmlDescription.name()!="Trigger" ) throw std::runtime_error( "Cannot create trigger from XML because the element provided is not named 'Trigger'" );

	std::vector<l1menu::tools::XMLElement> parameterElements=xmlDescription.getChildren("name");
	if( parameterElements.size()!=1 ) throw std::runtime_error( "Cannot create trigger from XML because the element doesn't have one and only one subelement called 'name'" );
	name_=parameterElements.front().getValue();

	parameterElements=xmlDescription.getChildren("version");
	if( parameterElements.size()!=1 ) throw std::runtime_error( "Cannot create trigger from XML because the element doesn't have one and only one subelement called 'version'" );
	version_=parameterElements.front().getIntValue();

	// Now loop over all of the parameters and set them
	parameterElements=xmlDescription.getChildren("parameter");
	for( const auto& parameterElement : parameterElements )
	{
		std::string parameterName=parameterElement.getAttribute("name");
		parameters_[parameterName]=parameterElement.getFloatValue();

		// See if there are errors stored.
		if( parameterElement.hasAttribute("errorHigh") || parameterElement.hasAttribute("errorLow") )
		{
			if( parameterElement.hasAttribute("errorHigh") ) parameterErrorsHigh_[parameterName]=parameterElement.getFloatAttribute("errorHigh");
			else parameterErrorsLow_[parameterName]=0;

			if( parameterElement.hasAttribute("errorLow") ) parameterErrorsLow_[parameterName]=parameterElement.getFloatAttribute("errorLow");
			else parameterErrorsLow_[parameterName]=0;
		}
	}

}

l1menu::implementation::TriggerDescriptionWithErrorsFromXML::TriggerDescriptionWithErrorsFromXML( const l1menu::ITriggerDescriptionWithErrors& otherDescription )
{
	name_=otherDescription.name();
	version_=otherDescription.version();

	for( const auto& parameterName : otherDescription.parameterNames() )
	{
		parameters_[parameterName]=otherDescription.parameter(parameterName);

		if( otherDescription.parameterErrorsAreAvailable(parameterName) )
		{
			parameterErrorsLow_[parameterName]=otherDescription.parameterErrorLow(parameterName);
			parameterErrorsHigh_[parameterName]=otherDescription.parameterErrorHigh(parameterName);
		}
	}
}

l1menu::implementation::TriggerDescriptionWithErrorsFromXML::TriggerDescriptionWithErrorsFromXML( const l1menu::ITriggerDescription& otherDescription )
{
	name_=otherDescription.name();
	version_=otherDescription.version();

	for( const auto& parameterName : otherDescription.parameterNames() )
	{
		parameters_[parameterName]=otherDescription.parameter(parameterName);
	}
}

const std::string l1menu::implementation::TriggerDescriptionWithErrorsFromXML::name() const
{
	return name_;
}

unsigned int l1menu::implementation::TriggerDescriptionWithErrorsFromXML::version() const
{
	return version_;
}

const std::vector<std::string> l1menu::implementation::TriggerDescriptionWithErrorsFromXML::parameterNames() const
{
	std::vector<std::string> returnValue;

	for( const auto& nameParameterPair : parameters_ ) returnValue.push_back( nameParameterPair.first );

	return returnValue;
}

const float& l1menu::implementation::TriggerDescriptionWithErrorsFromXML::parameter( const std::string& parameterName ) const
{
	const auto& iFindResult=parameters_.find( parameterName );
	if( iFindResult==parameters_.end() ) throw std::logic_error( "Not a valid parameter name" );
	else return iFindResult->second;
}

bool l1menu::implementation::TriggerDescriptionWithErrorsFromXML::parameterErrorsAreAvailable( const std::string& parameterName ) const
{
	// Assume that if there is a high error that there is also an entry in the low errors map.
	const auto& iFindResult=parameterErrorsHigh_.find( parameterName );
	if( iFindResult==parameterErrorsHigh_.end() ) return false;
	else return true;
}

const float& l1menu::implementation::TriggerDescriptionWithErrorsFromXML::parameterErrorLow( const std::string& parameterName ) const
{
	const auto& iFindResult=parameterErrorsLow_.find( parameterName );
	if( iFindResult==parameterErrorsLow_.end() ) throw std::runtime_error( "No error set for parameter "+parameterName);
	else return iFindResult->second;
}

const float& l1menu::implementation::TriggerDescriptionWithErrorsFromXML::parameterErrorHigh( const std::string& parameterName ) const
{
	const auto& iFindResult=parameterErrorsHigh_.find( parameterName );
	if( iFindResult==parameterErrorsHigh_.end() ) throw std::runtime_error( "No error set for parameter "+parameterName);
	else return iFindResult->second;
}
