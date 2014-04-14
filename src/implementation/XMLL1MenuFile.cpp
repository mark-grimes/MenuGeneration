#include "./XMLL1MenuFile.h"

#include <fstream>
#include <stdexcept>
#include "l1menu/TriggerMenu.h"
#include "l1menu/ITrigger.h"
#include "l1menu/TriggerConstraint.h"
#include "./MenuRateImplementation.h"

l1menu::implementation::XMLL1MenuFile::XMLL1MenuFile( std::ostream& outputStream ) : pOutputStream_(&outputStream)
{
}

l1menu::implementation::XMLL1MenuFile::XMLL1MenuFile( const std::string& filename, bool outputOnly ) : pOutputStream_(nullptr)
{
	if( outputOnly ) filenameForOutput_=filename;
	else outputFile_.parseFromFile(filename);
}

l1menu::implementation::XMLL1MenuFile::~XMLL1MenuFile()
{
	std::ofstream outputFile;
	if( !filenameForOutput_.empty() )
	{
		outputFile.open( filenameForOutput_ );
		if( outputFile.is_open() ) pOutputStream_=&outputFile;
	}

	if( pOutputStream_!=nullptr ) outputFile_.outputToStream( *pOutputStream_ );
}

void l1menu::implementation::XMLL1MenuFile::add( const l1menu::TriggerMenu& menu )
{
	l1menu::tools::XMLElement rootElement=outputFile_.rootElement();
	convertToXML( menu, rootElement );
}

void l1menu::implementation::XMLL1MenuFile::add( const l1menu::IMenuRate& menuRate )
{
	l1menu::tools::XMLElement rootElement=outputFile_.rootElement();
	convertToXML( menuRate, rootElement );
}

std::vector< std::unique_ptr<l1menu::TriggerMenu> > l1menu::implementation::XMLL1MenuFile::getMenus() const
{
	std::vector<l1menu::tools::XMLElement> childElements=outputFile_.rootElement().getChildren("TriggerMenu");

	std::vector< std::unique_ptr<l1menu::TriggerMenu> > returnValue;
	for( const auto& menuElement : childElements )
	{
		std::unique_ptr<l1menu::TriggerMenu> pNewMenu( new l1menu::TriggerMenu );

		std::vector<l1menu::tools::XMLElement> triggerElements=menuElement.getChildren("Trigger");
		for( const auto& triggerElement : triggerElements )
		{
			std::vector<l1menu::tools::XMLElement> parameterElements=triggerElement.getChildren("name");
			if( parameterElements.size()!=1 ) throw std::runtime_error( "Trigger doesn't have one and only one subelement called 'name'" );
			std::string triggerName=parameterElements.front().getValue();

			parameterElements=triggerElement.getChildren("version");
			if( parameterElements.size()!=1 ) throw std::runtime_error( "Trigger doesn't have one and only one subelement called 'version'" );
			size_t version=parameterElements.front().getIntValue();

			l1menu::ITrigger& newTrigger=pNewMenu->addTrigger( triggerName, version );
			// Now loop over all of the parameters and set them
			parameterElements=triggerElement.getChildren("parameter");
			for( const auto& parameterElement : parameterElements )
			{
				std::string parameterName=parameterElement.getAttribute("name");
				float parameterValue=parameterElement.getFloatValue();
				newTrigger.parameter(parameterName)=parameterValue;
			}

			//
			// If the menu has any information about constraints when
			// scaling, include those as well.
			//
			if( triggerElement.hasAttribute("fractionOfTotalBandwidth") )
			{
				float fraction=triggerElement.getFloatAttribute("fractionOfTotalBandwidth");
				l1menu::TriggerConstraint& newConstraint=pNewMenu->getTriggerConstraint(pNewMenu->numberOfTriggers()-1);
				newConstraint.type( l1menu::TriggerConstraint::Type::FRACTION_OF_BANDWIDTH );
				newConstraint.value( fraction );
			}
			else if( triggerElement.hasAttribute("fixedRate") )
			{
				float rate=triggerElement.getFloatAttribute("fixedRate");
				l1menu::TriggerConstraint& newConstraint=pNewMenu->getTriggerConstraint(pNewMenu->numberOfTriggers()-1);
				newConstraint.type( l1menu::TriggerConstraint::Type::FIXED_RATE );
				newConstraint.value( rate );
			}
		}

		returnValue.push_back( std::move(pNewMenu) );
	}

	return returnValue;
}

std::vector< std::unique_ptr<l1menu::IMenuRate> > l1menu::implementation::XMLL1MenuFile::getRates() const
{
	std::vector<l1menu::tools::XMLElement> childElements=outputFile_.rootElement().getChildren("MenuRate");

	std::vector< std::unique_ptr<l1menu::IMenuRate> > returnValue;
	for( const auto& element : childElements )
	{
		std::unique_ptr<l1menu::IMenuRate> pNewRate( new l1menu::implementation::MenuRateImplementation( element ) );
		returnValue.push_back( std::move(pNewRate) );
	}

	return returnValue;
}

l1menu::tools::XMLElement l1menu::implementation::XMLL1MenuFile::convertToXML( const l1menu::TriggerMenu& object, l1menu::tools::XMLElement& parent )
{
	l1menu::tools::XMLElement thisElement=parent.createChild( "TriggerMenu" );

	for( size_t index=0; index<object.numberOfTriggers(); ++index )
	{
		l1menu::tools::XMLElement newTriggerElement=convertToXML( object.getTrigger(index), thisElement );

		// If there are constraints on the trigger for fitting to a bandwidth, output those
		// as well. The type FIXED_THRESHOLDS is equivalent to no constraints (i.e. just use
		// whatever thresholds are set), so don't need to add anything for that.
		l1menu::TriggerConstraint constraint=object.getTriggerConstraint( index );
		if( constraint.type()==l1menu::TriggerConstraint::Type::FIXED_THRESHOLDS ) continue;
		else if( constraint.type()==l1menu::TriggerConstraint::Type::FIXED_RATE ) newTriggerElement.setAttribute( "fixedRate", constraint.value() );
		else if( constraint.type()==l1menu::TriggerConstraint::Type::FRACTION_OF_BANDWIDTH ) newTriggerElement.setAttribute( "fractionOfTotalBandwidth", constraint.value() );
		else throw std::runtime_error( "XMLL1MenuFile::convertToXML( const l1menu::TriggerMenu& object, ... ) - a trigger has an unknown constraint type" );
	}

	return thisElement;
}

l1menu::tools::XMLElement l1menu::implementation::XMLL1MenuFile::convertToXML( const l1menu::IMenuRate& object, l1menu::tools::XMLElement& parent )
{
	l1menu::tools::XMLElement thisElement=parent.createChild( "MenuRate" );
	thisElement.setAttribute( "formatVersion", 0 );

	thisElement.createChild( "totalFraction" ).setValue( object.totalFraction() );
	thisElement.createChild( "totalFractionError" ).setValue( object.totalFractionError() );
	thisElement.createChild( "totalRate" ).setValue( object.totalRate() );
	thisElement.createChild( "totalRateError" ).setValue( object.totalRateError() );

	// Loop over all of the trigger rates and add those to the file
	for( const auto& pTriggerRate : object.triggerRates() )
	{
		convertToXML( *pTriggerRate, thisElement );
	}

	return thisElement;
}

l1menu::tools::XMLElement l1menu::implementation::XMLL1MenuFile::convertToXML( const l1menu::ITriggerRate& object, l1menu::tools::XMLElement& parent )
{
	l1menu::tools::XMLElement thisElement=parent.createChild( "TriggerRate" );
	thisElement.setAttribute( "formatVersion", 0 );

	thisElement.createChild( "fraction" ).setValue( object.fraction() );
	thisElement.createChild( "fractionError" ).setValue( object.fractionError() );
	thisElement.createChild( "rate" ).setValue( object.rate() );
	thisElement.createChild( "rateError" ).setValue( object.rateError() );
	thisElement.createChild( "pureFraction" ).setValue( object.pureFraction() );
	thisElement.createChild( "pureFractionError" ).setValue( object.pureFractionError() );
	thisElement.createChild( "pureRate" ).setValue( object.pureRate() );
	thisElement.createChild( "pureRateError" ).setValue( object.pureRateError() );

	convertToXML( object.trigger(), thisElement );

	return thisElement;
}

l1menu::tools::XMLElement l1menu::implementation::XMLL1MenuFile::convertToXML( const l1menu::ITriggerDescription& object, l1menu::tools::XMLElement& parent )
{
	l1menu::tools::XMLElement thisElement=parent.createChild( "Trigger" );
	thisElement.setAttribute( "formatVersion", 0 );

	thisElement.createChild( "name" ).setValue( object.name() );
	// Need a cast because the compiler doesn't like going from unsigned int to int
	thisElement.createChild( "version" ).setValue( static_cast<int>( object.version() ) );

	for( const auto& parameterName : object.parameterNames() )
	{
		l1menu::tools::XMLElement parameterElement=thisElement.createChild( "parameter" );
		parameterElement.setAttribute( "name", parameterName );
		parameterElement.setValue( object.parameter( parameterName ) );
	}

	return thisElement;
}

l1menu::tools::XMLElement l1menu::implementation::XMLL1MenuFile::convertToXML( const l1menu::ITriggerDescriptionWithErrors& object, l1menu::tools::XMLElement& parent )
{
	// Use the method for the trigger description without errors for most of
	// the hard work, then add the errors afterwards.
	l1menu::tools::XMLElement thisElement=convertToXML( static_cast<const l1menu::ITriggerDescription&>(object), parent );

	//
	// Loop over all of the parameter elements just created, and if there
	// are errors available add those as attributes.
	//
	for( auto& childElement : thisElement.getChildren("parameter") )
	{
		std::string parameterName=childElement.getAttribute("name");
		if( object.parameterErrorsAreAvailable(parameterName) )
		{
			childElement.setAttribute( "errorHigh", object.parameterErrorHigh(parameterName) );
			childElement.setAttribute( "errorLow", object.parameterErrorLow(parameterName) );
		}
	}

	return thisElement;
}
