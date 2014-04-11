#include "l1menu/TriggerMenu.h"

#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include "l1menu/ITrigger.h"
#include "l1menu/tools/miscellaneous.h"
#include "l1menu/tools/fileIO.h"
#include "l1menu/tools/stringManipulation.h"
#include "l1menu/tools/XMLFile.h"
#include "l1menu/tools/XMLElement.h"

l1menu::TriggerMenu::TriggerMenu() : triggerTable_( l1menu::TriggerTable::instance() )
{
	// No operation besides the initialiser list
}

l1menu::TriggerMenu::~TriggerMenu()
{
	// No operation since I switched from raw pointers to unique_ptr.
}

l1menu::TriggerMenu::TriggerMenu( const TriggerMenu& otherTriggerMenu )
	: triggerTable_(otherTriggerMenu.triggerTable_)
{
	for( std::vector< std::unique_ptr<l1menu::ITrigger> >::const_iterator iTrigger=otherTriggerMenu.triggers_.begin(); iTrigger!=otherTriggerMenu.triggers_.end(); ++iTrigger )
	{
		l1menu::ITrigger& sourceTrigger=**iTrigger;

		triggers_.push_back( std::move(triggerTable_.copyTrigger(sourceTrigger)) );
	}

	// Make sure triggerResults_ is always the same size as triggers_
	triggerResults_.resize( triggers_.size() );
}

l1menu::TriggerMenu::TriggerMenu( TriggerMenu&& otherTriggerMenu ) noexcept
	: triggerTable_(otherTriggerMenu.triggerTable_), triggers_( std::move(otherTriggerMenu.triggers_) ),
	  triggerResults_( std::move(otherTriggerMenu.triggerResults_) )
{
	// No operation besides the initialiser list
}

l1menu::TriggerMenu& l1menu::TriggerMenu::operator=( const l1menu::TriggerMenu& otherTriggerMenu )
{
	//
	// First clean up whatever triggers I had before
	//
	triggers_.clear();

	//
	// Now copy the triggers from the other menu
	//
	for( std::vector< std::unique_ptr<l1menu::ITrigger> >::const_iterator iTrigger=otherTriggerMenu.triggers_.begin(); iTrigger!=otherTriggerMenu.triggers_.end(); ++iTrigger )
	{
		l1menu::ITrigger& sourceTrigger=**iTrigger;

		triggers_.push_back( std::move(triggerTable_.copyTrigger(sourceTrigger)) );
	}

	// Make sure triggerResults_ is always the same size as triggers_
	triggerResults_.resize( triggers_.size() );

	return *this;
}

l1menu::TriggerMenu& l1menu::TriggerMenu::operator=( l1menu::TriggerMenu&& otherTriggerMenu ) noexcept
{
	triggers_=std::move( otherTriggerMenu.triggers_ );
	triggerResults_=std::move(otherTriggerMenu.triggerResults_);

	return *this;
}

l1menu::ITrigger& l1menu::TriggerMenu::addTrigger( const std::string& triggerName )
{
	std::unique_ptr<l1menu::ITrigger> pNewTrigger=triggerTable_.getTrigger( triggerName );
	if( pNewTrigger.get()==NULL ) throw std::range_error( "Trigger requested that does not exist" );

	triggers_.push_back( std::move(pNewTrigger) );

	// Make sure triggerResults_ is always the same size as triggers_
	triggerResults_.resize( triggers_.size() );
	return *triggers_.back();
}

l1menu::ITrigger& l1menu::TriggerMenu::addTrigger( const std::string& triggerName, unsigned int version )
{
	std::unique_ptr<l1menu::ITrigger> pNewTrigger=triggerTable_.getTrigger( triggerName, version );
	if( pNewTrigger.get()==NULL ) throw std::range_error( "Trigger requested that does not exist" );

	triggers_.push_back( std::move(pNewTrigger) );

	// Make sure triggerResults_ is always the same size as triggers_
	triggerResults_.resize( triggers_.size() );
	return *triggers_.back();
}

l1menu::ITrigger& l1menu::TriggerMenu::addTrigger( const l1menu::ITrigger& triggerToCopy )
{
	std::unique_ptr<l1menu::ITrigger> pNewTrigger=triggerTable_.copyTrigger( triggerToCopy );
	if( pNewTrigger.get()==NULL ) throw std::range_error( "Trigger requested that does not exist" );

	triggers_.push_back( std::move(pNewTrigger) );

	// Make sure triggerResults_ is always the same size as triggers_
	triggerResults_.resize( triggers_.size() );
	return *triggers_.back();
}

size_t l1menu::TriggerMenu::numberOfTriggers() const
{
	return triggers_.size();
}

l1menu::ITrigger& l1menu::TriggerMenu::getTrigger( size_t position )
{
	if( position>triggers_.size() ) throw std::range_error( "Trigger requested that does not exist in the menu" );

	return *triggers_[position];
}

const l1menu::ITrigger& l1menu::TriggerMenu::getTrigger( size_t position ) const
{
	if( position>triggers_.size() ) throw std::range_error( "Trigger requested that does not exist in the menu" );

	return *triggers_[position];
}

std::unique_ptr<l1menu::ITrigger> l1menu::TriggerMenu::getTriggerCopy( size_t position ) const
{
	if( position>triggers_.size() ) throw std::range_error( "Trigger requested that does not exist in the menu" );

	return triggerTable_.copyTrigger(*triggers_[position]);
}

bool l1menu::TriggerMenu::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	bool atLeastOneTriggerHasFired=false;

	for( size_t triggerNumber=0; triggerNumber<triggers_.size(); ++triggerNumber )
	{
		bool result=triggers_[triggerNumber]->apply(event);
//		triggerResults_[triggerNumber]=result;
		if( result ) atLeastOneTriggerHasFired=true;
	}

	return atLeastOneTriggerHasFired;
}

void l1menu::TriggerMenu::loadMenuFromFile( const std::string& filename )
{
	// First try and open the file as an XML file. If that fails then assume
	// the file is in the old format and do that.
	bool fileIsXML=false;
	try
	{
		l1menu::tools::XMLFile inputFile( filename );
		// If an exception is thrown above, I want to open the file in the old format.
		// If an exception is thrown after this line then I want to propagate it upwards
		// so I'll set this flag to tell the catch block to rethrow.
		fileIsXML=true;

		l1menu::tools::XMLElement rootElement=inputFile.rootElement();
		restoreFromXML( rootElement );
	}
	catch( std::runtime_error& exception )
	{
		// If the file was successfully opened as XML but some error occurred, then I want
		// that exception to propagate backwards.
		if( fileIsXML ) throw;


		std::ifstream file( filename.c_str() );
		if( !file.is_open() ) throw std::runtime_error( "Unable to open file "+filename );

		loadMenuInOldFormat( file );
	}
}

void l1menu::TriggerMenu::saveMenuToFile( const std::string& filename ) const
{
	l1menu::tools::XMLFile outputFile;
	l1menu::tools::XMLElement rootElement=outputFile.rootElement();
	saveToXML( rootElement );

	std::ofstream outputStream( filename );
	if( !outputStream.is_open() ) throw std::runtime_error( "Unable to open file "+filename+" for output" );
	outputFile.outputToStream( outputStream );
}

void l1menu::TriggerMenu::saveToXML( l1menu::tools::XMLElement& parentElement ) const
{
	l1menu::tools::XMLElement thisElement=parentElement.createChild( "TriggerMenu" );

	for( const auto& pTrigger : triggers_ )
	{
		l1menu::tools::convertToXML( *pTrigger, thisElement );
	}
}

void l1menu::TriggerMenu::restoreFromXML( const l1menu::tools::XMLElement& parentElement )
{
	// Rather than read into the member, I'll use a temporary store so that this
	// method is atomic. I.e. if something goes wrong the instance won't be in a
	// half altered state. Once I know everything was successful, then I'll write
	// over the previous information.
	std::vector< std::unique_ptr<l1menu::ITrigger> > newTriggers;

	// See what children the parent element has
	std::vector<l1menu::tools::XMLElement> childElements=parentElement.getChildren("TriggerMenu");

	if( childElements.empty() ) throw std::runtime_error( "l1menu::TriggerMenu::restoreFromXML - element passed does not have a \"TriggerMenu\" child element." );
	if( childElements.size()>1 ) std::cout << "l1menu::TriggerMenu::restoreFromXML - N.B. The element passed has more than one \"TriggerMenu\" child element, only the first will be used." << std::endl;

	const auto& thisElement=childElements.front();
	// Loop over all of the child elements that have the name "Trigger"
	std::vector<l1menu::tools::XMLElement> triggerElements=thisElement.getChildren("Trigger");
	for( const auto& triggerElement : triggerElements )
	{
		std::vector<l1menu::tools::XMLElement> parameterElements=triggerElement.getChildren("name");
		if( parameterElements.size()!=1 ) throw std::runtime_error( "Trigger doesn't have one and only one subelement called 'name'" );
		std::string triggerName=parameterElements.front().getValue();

		parameterElements=triggerElement.getChildren("version");
		if( parameterElements.size()!=1 ) throw std::runtime_error( "Trigger doesn't have one and only one subelement called 'version'" );
		size_t version=parameterElements.front().getIntValue();

		std::unique_ptr<l1menu::ITrigger> pNewTrigger=triggerTable_.getTrigger( triggerName, version );
		if( pNewTrigger==nullptr ) throw std::runtime_error( "l1menu::TriggerMenu::restoreFromXML - the file lists trigger \""+triggerName+"\" with version "+triggerElement.getAttribute("version")+" that is not registered in the TriggerTable." );

		// Now loop over all of the parameters and set them
		parameterElements=triggerElement.getChildren("parameter");
		for( const auto& parameterElement : parameterElements )
		{
			std::string parameterName=parameterElement.getAttribute("name");
			float parameterValue=parameterElement.getFloatValue();
			pNewTrigger->parameter(parameterName)=parameterValue;
		}
		newTriggers.push_back( std::move(pNewTrigger) );
	}

	// If we get to this point and no exceptions have been thrown, then everything
	// worked and I can write over the previous information.
	triggers_=std::move( newTriggers );
}

void l1menu::TriggerMenu::loadMenuInOldFormat( std::ifstream& file )
{
	const size_t bufferSize=200;
	char buffer[bufferSize];

	while( file.good() )
	{
		try
		{
			// Get one line at a time
			file.getline( buffer, bufferSize );

			// split the line by whitespace into columns
			std::vector<std::string> tableColumns=l1menu::tools::splitByWhitespace( buffer );

			if( tableColumns.size()==1 && tableColumns[0].empty() ) continue; // Allow blank lines without giving a warning
			if( tableColumns.size()!=12 ) throw std::runtime_error( "The line does not have the correct number of columns" );

			addTriggerFromOldFormat( tableColumns );

		} // end of try block
		catch( std::runtime_error& exception )
		{
			std::cerr << "Some error occured while processing the line \"" << buffer << "\":" << exception.what() << std::endl;
		}
	}
}

bool l1menu::TriggerMenu::addTriggerFromOldFormat( const std::vector<std::string>& columns )
{
	bool successful=false;

	if( columns.size()<9 ) throw std::runtime_error( "There are not enough columns" );

	float prescale=l1menu::tools::convertStringToFloat( columns[2] );
	if( prescale!=0 )
	{
		std::string triggerName=columns[0];

		try
		{
			//std::cout << "Added trigger \"" << columns[0] << "\"" << std::endl;
			l1menu::ITrigger& newTrigger=addTrigger( triggerName ); // Try and create a trigger with the name supplied
			successful=true;

			// Different triggers will have different numbers of thresholds, and even different names. E.g. Single triggers
			// will have "threshold1" whereas a cross trigger will have "leg1threshold1", "leg2threshold1" etcetera. This
			// utility function will get the threshold names in the correct order.
			const auto& thresholdNames=l1menu::tools::getThresholdNames(newTrigger);
			if( thresholdNames.size()>=1 ) newTrigger.parameter(thresholdNames[0])=l1menu::tools::convertStringToFloat( columns[3] );
			if( thresholdNames.size()>=2 ) newTrigger.parameter(thresholdNames[1])=l1menu::tools::convertStringToFloat( columns[4] );
			if( thresholdNames.size()>=3 ) newTrigger.parameter(thresholdNames[2])=l1menu::tools::convertStringToFloat( columns[5] );
			if( thresholdNames.size()>=4 ) newTrigger.parameter(thresholdNames[3])=l1menu::tools::convertStringToFloat( columns[6] );

			float etaOrRegionCut=l1menu::tools::convertStringToFloat( columns[7] );
			// For most triggers, I can just try and set both the etaCut and regionCut parameters
			// with this value. If it doesn't have either of the parameters just catch the exception
			// and nothing will happen. Some cross triggers however have both, and need to set them
			// both from this value which requires a conversion. Most cross triggers expect this
			// value to be the regionCut, except for L1_SingleMu_CJet which expects it as the etaCut.
			if( triggerName=="L1_SingleMu_CJet" )
			{
				newTrigger.parameter("leg1etaCut")=etaOrRegionCut;
				newTrigger.parameter("leg2regionCut")=l1menu::tools::convertEtaCutToRegionCut( etaOrRegionCut );
			}
			else if( triggerName=="L1_TkMu_TkJet" )
			{
				newTrigger.parameter("leg1etaCut")=etaOrRegionCut;
				newTrigger.parameter("leg2regionCut")=l1menu::tools::convertEtaCutToRegionCut( etaOrRegionCut );
			}
			else if( triggerName=="L1_isoMu_EG" )
			{
				newTrigger.parameter("leg1etaCut")=l1menu::tools::convertRegionCutToEtaCut( etaOrRegionCut );
				newTrigger.parameter("leg2regionCut")=etaOrRegionCut;
			}
			else if( triggerName=="L1_isoEG_Mu" )
			{
				newTrigger.parameter("leg1regionCut")=etaOrRegionCut;
				newTrigger.parameter("leg2etaCut")=l1menu::tools::convertRegionCutToEtaCut( etaOrRegionCut );
			}
			else if( triggerName=="L1_TkEle_Mu" )
			{
				newTrigger.parameter("leg1regionCut")=etaOrRegionCut;
				newTrigger.parameter("leg2etaCut")=l1menu::tools::convertRegionCutToEtaCut( etaOrRegionCut );
			}
			else if( triggerName=="L1_TkTau_Mu" )
			{
				newTrigger.parameter("leg1regionCut")=etaOrRegionCut;
				newTrigger.parameter("leg2etaCut")=l1menu::tools::convertRegionCutToEtaCut( etaOrRegionCut );
			}

			else if( triggerName=="L1_isoMu_Tau" )
			{
				newTrigger.parameter("leg1etaCut")=l1menu::tools::convertRegionCutToEtaCut( etaOrRegionCut );
				newTrigger.parameter("leg2regionCut")=etaOrRegionCut;
			}
			else
			{
				// Any remaining triggers should only have one of these parameters and won't
				// need conversion. I'll just try and set them both, not a problem if one fails.
				// The cross triggers will have e.g. "leg1" prefixed to the parameter name so I'll
				// also try for those.
				try{ newTrigger.parameter("etaCut")=etaOrRegionCut; }
				catch( std::exception& error ) { } // Do nothing, just try and convert the other parameters

				try{ newTrigger.parameter("regionCut")=etaOrRegionCut; }
				catch( std::exception& error ) { } // Do nothing, just try and convert the other parameters

				try{ newTrigger.parameter("leg1etaCut")=etaOrRegionCut; }
				catch( std::exception& error ) { } // Do nothing, just try and convert the other parameters

				try{ newTrigger.parameter("leg1regionCut")=etaOrRegionCut; }
				catch( std::exception& error ) { } // Do nothing, just try and convert the other parameters

				try{ newTrigger.parameter("leg2etaCut")=etaOrRegionCut; }
				catch( std::exception& error ) { } // Do nothing, just try and convert the other parameters

				try{ newTrigger.parameter("leg2regionCut")=etaOrRegionCut; }
				catch( std::exception& error ) { } // Do nothing, just try and convert the other parameters
			}

			// The trigger may or may not have a muon quality cut. I also don't know if its name
			// is prefixed with e.g. "leg1". I'll try setting all combinations, but wrap individually
			// in a try block so that it doesn't matter if it fails.
			try{ newTrigger.parameter("muonQuality")=l1menu::tools::convertStringToFloat( columns[8] ); }
			catch( std::exception& error ) { } // Do nothing, just try and convert the other parameters

			try{ newTrigger.parameter("leg1muonQuality")=l1menu::tools::convertStringToFloat( columns[8] ); }
			catch( std::exception& error ) { } // Do nothing, just try and convert the other parameters

			try{ newTrigger.parameter("leg2muonQuality")=l1menu::tools::convertStringToFloat( columns[8] ); }
			catch( std::exception& error ) { } // Do nothing, just try and convert the other parameters

		} // end of try block
		catch( std::exception& error )
		{
			std::cerr << "Unable to add trigger \"" << columns[0] << "\" because: " << error.what() << std::endl;
		}
	} // end of "if( prescale!=0 )"

	return successful;
}
