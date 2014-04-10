#include "l1menu/tools/fileIO.h"

#include <sstream>
#include <exception>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "l1menu/ITrigger.h"
#include "l1menu/L1TriggerDPGEvent.h"
#include "l1menu/TriggerTable.h"
#include "l1menu/TriggerMenu.h"
#include "l1menu/IMenuRate.h"
#include "l1menu/ITriggerRate.h"
#include "l1menu/FullSample.h"
#include "l1menu/ReducedSample.h"
#include "l1menu/IL1MenuFile.h"
#include "l1menu/tools/XMLFile.h"
#include "l1menu/tools/miscellaneous.h"
#include "l1menu/tools/XMLElement.h"
#include "../implementation/MenuRateImplementation.h"

namespace // Unnamed namespace for things only used in this file
{
	/** @brief Moved out some code from the publicly available functions to here for convenience.
	 *
	 * I was getting ambiguous call compiler errors because nullptr was being converted to the
	 * FileFormat enum. I thought c++11 would stop that, but I couldn't sort it out.
	 *
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 14/Oct/2013
	 */
	void dumpTriggerRatesInOldFormat( std::ostream& output, const l1menu::IMenuRate& menuRates, const l1menu::IMenuRate* pOfflineThresholds, const char delimeter )
	{
		// I want to print the triggers in the same order as the old code does to make results
		// easier to compare between the old and new code. Otherwise the new code will print
		// the results alphabetically. I need to hard code that order with this vector.
		std::vector<std::string> triggerNames;
		triggerNames.push_back("L1_SingleEG");
		triggerNames.push_back("L1_SingleIsoEG");
		triggerNames.push_back("L1_SingleMu");
		triggerNames.push_back("L1_SingleIsoMu");
		triggerNames.push_back("L1_SingleTau");
		triggerNames.push_back("L1_SingleIsoTau");
		triggerNames.push_back("L1_DoubleEG");
		triggerNames.push_back("L1_isoEG_EG");
		triggerNames.push_back("L1_DoubleIsoEG");
		triggerNames.push_back("L1_DoubleMu");
		triggerNames.push_back("L1_isoMu_Mu");
		triggerNames.push_back("L1_DoubleIsoMu");
		triggerNames.push_back("L1_DoubleTau");
		triggerNames.push_back("L1_isoTau_Tau");
		triggerNames.push_back("L1_DoubleIsoTau");
		triggerNames.push_back("L1_EG_Mu");
		triggerNames.push_back("L1_isoEG_Mu");
		triggerNames.push_back("L1_isoEG_isoMu");
		triggerNames.push_back("L1_Mu_EG");
		triggerNames.push_back("L1_isoMu_EG");
		triggerNames.push_back("L1_isoMu_isoEG");
		triggerNames.push_back("L1_EG_Tau");
		triggerNames.push_back("L1_isoEG_Tau");
		triggerNames.push_back("L1_isoEG_isoTau");
		triggerNames.push_back("L1_Mu_Tau");
		triggerNames.push_back("L1_isoMu_Tau");
		triggerNames.push_back("L1_isoMu_isoTau");
		triggerNames.push_back("L1_SingleJet");
		triggerNames.push_back("L1_SingleJetC");
		triggerNames.push_back("L1_DoubleJet");
		triggerNames.push_back("L1_QuadJetC");
		triggerNames.push_back("L1_SixJet");
		triggerNames.push_back("L1_SingleEG_CJet");
		triggerNames.push_back("L1_SingleIsoEG_CJet");
		triggerNames.push_back("L1_SingleMu_CJet");
		triggerNames.push_back("L1_SingleIsoMu_CJet");
		triggerNames.push_back("L1_SingleTau_TwoFJet");
		triggerNames.push_back("L1_DoubleFwdJet");
		triggerNames.push_back("L1_SingleEG_ETM");
		triggerNames.push_back("L1_SingleIsoEG_ETM");
		triggerNames.push_back("L1_SingleMu_ETM");
		triggerNames.push_back("L1_SingleIsoMu_ETM");
		triggerNames.push_back("L1_SingleTau_ETM");
		triggerNames.push_back("L1_SingleIsoTau_ETM");
		triggerNames.push_back("L1_SingleEG_HTM");
		triggerNames.push_back("L1_SingleIsoEG_HTM");
		triggerNames.push_back("L1_SingleMu_HTM");
		triggerNames.push_back("L1_SingleIsoMu_HTM");
		triggerNames.push_back("L1_SingleTau_HTM");
		triggerNames.push_back("L1_SingleIsoTau_HTM");
		triggerNames.push_back("L1_HTM");
		triggerNames.push_back("L1_ETM");
		triggerNames.push_back("L1_HTT");

		// Take a copy of the results so that I can resort them.
		auto triggerRates=menuRates.triggerRates();
		//
		// Use a lambda function to sort the ITriggerRates into the same
		// order as the standard list above.
		//
		std::sort( triggerRates.begin(), triggerRates.end(), [&](const l1menu::ITriggerRate* pFirst,const l1menu::ITriggerRate* pSecond)->bool
			{
				auto iFirstPosition=std::find( triggerNames.begin(), triggerNames.end(), pFirst->trigger().name() );
				auto iSecondPosition=std::find( triggerNames.begin(), triggerNames.end(), pSecond->trigger().name() );
				// If both these trigger names aren't in the standard list, sort
				// them by their name which I guess means alphabetically.
				if( iFirstPosition==triggerNames.end() && iSecondPosition==triggerNames.end() ) return pFirst->trigger().name()<pSecond->trigger().name();
				// If only one of them is in the list then that one needs to be first.
				else if( iFirstPosition==triggerNames.end() ) return false;
				else if( iSecondPosition==triggerNames.end() ) return true;
				// If they're both in the standard list sort by their position in the list
				else return iFirstPosition<iSecondPosition;
			} );

		// Create strings of the offline thresholds if they exist. Put them in a map with the trigger name as a key
		// so that I can easily match them up with the online thresholds.
		std::map<std::string,std::string> offlineThresholdStrings;
		if( pOfflineThresholds!=nullptr )
		{
			std::stringstream offlineOutput;
			for( const auto& pOfflineRate : pOfflineThresholds->triggerRates() )
			{
				offlineOutput.str("");

				const auto& trigger=pOfflineRate->trigger();

				// Print the thresholds
				std::vector<std::string> thresholdNames=l1menu::tools::getThresholdNames( trigger );
				for( size_t thresholdNumber=0; thresholdNumber<4; ++thresholdNumber )
				{
					if( thresholdNames.size()>thresholdNumber ) offlineOutput << delimeter << std::setw(7) << trigger.parameter(thresholdNames[thresholdNumber]);
					else offlineOutput << delimeter << std::setw(7) << " ";
				}

				offlineThresholdStrings[trigger.name()]=offlineOutput.str();
			}
		}

		float totalNoOverlaps=0;
		float totalPure=0;
		for( const auto& pRate : triggerRates )
		{
			const auto& trigger=pRate->trigger();
			//output << "Trigger " << pRate->trigger().name() << " has fraction " << pRate->fraction() << " and rate " << pRate->rate() << "kHz, pure " << pRate->pureRate() << "kHz" << std::endl;
			// Print the name
			output << std::left << std::setw(23) << trigger.name();

			// Print the thresholds
			std::vector<std::string> thresholdNames=l1menu::tools::getThresholdNames( trigger );
			for( size_t thresholdNumber=0; thresholdNumber<4; ++thresholdNumber )
			{
				if( thresholdNames.size()>thresholdNumber ) output << delimeter << std::setw(7) << trigger.parameter(thresholdNames[thresholdNumber]);
				else output << delimeter << std::setw(7) << " ";
			}

			// If there are offline thresholds to print, print those as well.
			const auto iFindResult=offlineThresholdStrings.find( trigger.name() );
			if( iFindResult!=offlineThresholdStrings.end() )
			{
				output << iFindResult->second;
			}

			// Print the rates
			output << delimeter << std::setw(15) << pRate->rate();
			output << delimeter << std::setw(15) << pRate->rateError();
			output << delimeter << std::setw(11) << pRate->pureRate();
			output << delimeter << std::setw(11) << pRate->pureRateError();

			totalNoOverlaps+=pRate->rate();
			totalPure+=pRate->pureRate();

			output << "\n";
		}

		output << "---------------------------------------------------------------------------------------------------------------" << "\n"
				<< " Total L1 Rate (with overlaps)    = " << delimeter << std::setw(8) << menuRates.totalRate() << delimeter << " +/- " << delimeter << menuRates.totalRateError() << delimeter << " kHz" << "\n"
				<< " Total L1 Rate (without overlaps) = " << delimeter << std::setw(8) << totalNoOverlaps << delimeter << " kHz" << "\n"
				<< " Total L1 Rate (pure triggers)    = " << delimeter << std::setw(8) << totalPure << delimeter << " kHz" << std::endl;

	} // end of function dumpTriggerRatesInOldFormat
}


void l1menu::tools::dumpTriggerRates( std::ostream& output, const l1menu::IMenuRate& menuRates, const l1menu::IMenuRate& offlineThresholds, l1menu::tools::FileFormat format )
{
	//return dumpTriggerRates( output, menuRates, &offlineThresholds );
	if( format==l1menu::tools::FileFormat::XMLFORMAT )
	{
		l1menu::tools::XMLFile outputFile;
		l1menu::tools::XMLElement rootElement=outputFile.rootElement();

		l1menu::tools::XMLElement onlineThresholdElement=l1menu::tools::convertToXML( menuRates, rootElement );
		l1menu::tools::XMLElement offlineThresholdElement=l1menu::tools::convertToXML( offlineThresholds, rootElement );

		onlineThresholdElement.setAttribute( "description", "online thresholds" );
		offlineThresholdElement.setAttribute( "description", "offline thresholds" );

		outputFile.outputToStream( output );

	}
	else if( format==l1menu::tools::FileFormat::OLDFORMAT || format==l1menu::tools::FileFormat::CSVFORMAT )
	{
		char delimeter;
		if( format==l1menu::tools::FileFormat::CSVFORMAT ) delimeter=',';
		else delimeter=' ';

		// Delegate to the function in the unnamed namespace
		::dumpTriggerRatesInOldFormat( output, menuRates, &offlineThresholds, delimeter );

	} // end of "if format is OLDFORMAT or CSVFORMAT"
}

void l1menu::tools::dumpTriggerRates( std::ostream& output, const l1menu::IMenuRate& menuRates, l1menu::tools::FileFormat format )
{
	//return dumpTriggerRates( output, menuRates, nullptr, format );
	if( format==l1menu::tools::FileFormat::XMLFORMAT )
	{
		l1menu::tools::XMLFile outputFile;
		l1menu::tools::XMLElement rootElement=outputFile.rootElement();
		l1menu::tools::convertToXML( menuRates, rootElement );
		outputFile.outputToStream( output );

	}
	else if( format==l1menu::tools::FileFormat::OLDFORMAT || format==l1menu::tools::FileFormat::CSVFORMAT )
	{
		char delimeter;
		if( format==l1menu::tools::FileFormat::CSVFORMAT ) delimeter=',';
		else delimeter=' ';

		// Delegate to the function in the unnamed namespace
		::dumpTriggerRatesInOldFormat( output, menuRates, nullptr, delimeter );

	} // end of "if format is OLDFORMAT or CSVFORMAT"
	else // all known formats covered above, but in case a new one is introduced
	{
		throw std::logic_error( "dumpTriggerRates - file format requested has not been implemented yet" );
	}

}

void l1menu::tools::dumpTriggerMenu( std::ostream& output, const l1menu::TriggerMenu& menu, l1menu::tools::FileFormat format )
{
	if( format==l1menu::tools::FileFormat::OLDFORMAT )
	{
		for( size_t index=0; index< menu.numberOfTriggers(); ++index )
		{
			const l1menu::ITrigger& trigger=menu.getTrigger(index);
			output << std::setw(21) << trigger.name()
					<< " 00 " // previous code output the trigger bit number. For now I'll output an arbitrary "00"
					<< std::setw(5) << " 1 "; // haven't implemented prescales yet, so output "1"
			std::vector<std::string> thresholdNames=l1menu::tools::getThresholdNames(trigger);
			for( const auto& name : thresholdNames ) output << std::setw(5) << trigger.parameter(name) << " ";
			// If there is fewer than 4 thresholds, then I need to output arbitrary null values
			for( size_t thresholdNumber=thresholdNames.size(); thresholdNumber<4; ++thresholdNumber ) output << std::setw(5) << -1 << " ";

			// I now need to output the eta cut, which can be stored in a number of ways.
			float etaCut;
			try{ etaCut=trigger.parameter("etaCut"); }
			catch(...)
			{
				try{ etaCut=trigger.parameter("regionCut"); }
				catch(...)
				{
					try{ etaCut=trigger.parameter("leg1etaCut"); }
					catch(...)
					{
						try{ etaCut=trigger.parameter("leg1regionCut"); }
						catch(...)
						{
							// Print out a default of -1 if no eta cut can be found
							etaCut=-1;
						}
					}
				}
			}
			output << std::setw(5) << etaCut << " ";

			// Also need muon quality, if it exists
			float muonQuality;
			try{ muonQuality=trigger.parameter("muonQuality"); }
			catch(...)
			{
				try{ muonQuality=trigger.parameter("leg1muonQuality"); }
				catch(...)
				{
					try{ muonQuality=trigger.parameter("leg2muonQuality"); }
					catch(...)
					{
						// Print out a default of -1 if no quality can be found
						muonQuality=-1;
					}
				}
			}
			output << std::setw(5) << muonQuality << " ";

			output << "\n";
		}
	} // if file format is OLDFORMAT
	else if( format==l1menu::tools::FileFormat::XMLFORMAT )
	{
		l1menu::tools::XMLFile outputFile;
		l1menu::tools::XMLElement rootElement=outputFile.rootElement();
		l1menu::tools::convertToXML( menu, rootElement );
		outputFile.outputToStream( output );
	}
	else
	{
		throw std::logic_error( "dumpTriggerMenu - file format requested has not been implemented yet" );
	}
}

std::unique_ptr<l1menu::ISample> l1menu::tools::loadSample( const std::string& filename )
{
	// Open the file, read enough of the start to determine what kind of file
	// it is, then close it.
	std::ifstream inputFile( filename, std::ios_base::binary );
	if( !inputFile.is_open() ) throw std::runtime_error( "The file does not exist or could not be opened" );

	// Look at the first few characters and see if they match some of the file formats
	const size_t bufferSize=20;
	char buffer[bufferSize];
	inputFile.get( buffer, bufferSize );
	inputFile.close();

	if( std::string(buffer)=="l1menuReducedSample" ) return std::unique_ptr<l1menu::ISample>( new l1menu::ReducedSample(filename) );
	else
	{
		// If it's not a ReducedSample then the only other ISample implementation at the
		// moment is a FullSample.
		std::unique_ptr<l1menu::FullSample> pReturnValue( new l1menu::FullSample );

		if( std::string(buffer).substr(0,4)=="root" )
		{
			// File is a root file, so assume it is one of the L1 DPG ntuples and try and load it
			// into the FullSample.
			pReturnValue->loadFile( filename );
			return std::unique_ptr<l1menu::ISample>( pReturnValue.release() );
		}
		else
		{
			// Assume the file is a list of filenames of L1 DPG ntuples.
			// TODO Do some checking to see if the characters I've read so far are valid filepath characters.
			pReturnValue->loadFilesFromList( filename );
			return std::unique_ptr<l1menu::ISample>( pReturnValue.release() );
		}
	}
}

std::unique_ptr<l1menu::TriggerMenu> l1menu::tools::loadMenu( const std::string& filename )
{
	//
	// This function was written before IL1MenuFile was created, but it's still
	// useful as a shorthand.
	//
	std::unique_ptr<l1menu::IL1MenuFile> pFile=l1menu::IL1MenuFile::getInputFile( filename );
	std::vector< std::unique_ptr<l1menu::TriggerMenu> > menusFromFile=pFile->getMenus();
	if( menusFromFile.empty() ) throw std::runtime_error( "l1menu::tools::loadMenu(\""+filename+"\") - Unable to load the menu" );
	return std::move( menusFromFile.front() );
}

std::unique_ptr<l1menu::IMenuRate> l1menu::tools::loadRate( const std::string& filename )
{
	l1menu::tools::XMLFile inputFile( filename );
	l1menu::tools::XMLElement rootElement=inputFile.rootElement();

	std::vector<l1menu::tools::XMLElement> childElements=rootElement.getChildren("MenuRate");
	if( childElements.empty() ) throw std::runtime_error( "l1menu::tools::loadRate - file does not contain a \"MenuRate\" child element." );
	if( childElements.size()>1 ) std::cout << "l1menu::tools::loadRate - N.B. The file has more than one \"MenuRate\" child element, only the first will be used." << std::endl;

	std::unique_ptr<l1menu::IMenuRate> pReturnValue( new l1menu::implementation::MenuRateImplementation( childElements.front() ) );
	return pReturnValue;
}

l1menu::tools::XMLElement l1menu::tools::convertToXML( const l1menu::TriggerMenu& object, l1menu::tools::XMLElement& parent )
{
	l1menu::tools::XMLElement thisElement=parent.createChild( "TriggerMenu" );

	for( size_t index=0; index<object.numberOfTriggers(); ++index )
	{
		l1menu::tools::convertToXML( object.getTrigger(index), thisElement );
	}

	return thisElement;
}

l1menu::tools::XMLElement l1menu::tools::convertToXML( const l1menu::ITriggerRate& object, l1menu::tools::XMLElement& parent )
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

	l1menu::tools::convertToXML( object.trigger(), thisElement );

	return thisElement;
}

l1menu::tools::XMLElement l1menu::tools::convertToXML( const l1menu::ITriggerDescription& object, l1menu::tools::XMLElement& parent )
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

l1menu::tools::XMLElement l1menu::tools::convertToXML( const l1menu::IMenuRate& object, l1menu::tools::XMLElement& parent )
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
		l1menu::tools::convertToXML( *pTriggerRate, thisElement );
	}

	return thisElement;
}
