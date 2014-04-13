#include "./OldL1MenuFile.h"

#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include "l1menu/TriggerMenu.h"
#include "l1menu/ITrigger.h"
#include "l1menu/ITriggerDescription.h"
#include "l1menu/ITriggerDescriptionWithErrors.h"
#include "l1menu/TriggerConstraint.h"
#include "l1menu/tools/miscellaneous.h"
#include "l1menu/tools/stringManipulation.h"
#include "./MenuRateImplementation.h"

l1menu::implementation::OldL1MenuFile::OldL1MenuFile( std::ostream& outputStream, const char delimeter ) : pOutputStream_(&outputStream), delimeter_(delimeter)
{
}

l1menu::implementation::OldL1MenuFile::OldL1MenuFile( const std::string& inputFilename, const char delimeter, bool write ) : pOutputStream_(nullptr), filename_(inputFilename), delimeter_(delimeter)
{
	// If the user wants to write to the file, it needs to be opened with write permissions
	if( write )	file_.open( inputFilename, std::ios_base::out | std::ios_base::trunc );
	else file_.open( inputFilename );

	if( file_.is_open() ) pOutputStream_=&file_;
	else throw std::runtime_error( "OldL1MenuFile::OldL1MenuFile( \""+inputFilename+"\", '"+delimeter+"' ) - Unable to open file" );
}

l1menu::implementation::OldL1MenuFile::~OldL1MenuFile()
{
	file_.close();
}

void l1menu::implementation::OldL1MenuFile::add( const l1menu::TriggerMenu& object )
{

}

void l1menu::implementation::OldL1MenuFile::add( const l1menu::IMenuRate& menuRates )
{
	if( pOutputStream_==nullptr ) throw std::runtime_error( "OldL1MenuFile add is trying to add an IMenuRate but the output stream pointer is null" );

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


	float totalNoOverlaps=0;
	float totalPure=0;
	for( const auto& pRate : triggerRates )
	{
		const auto& trigger=pRate->trigger();
		// Print the name
		(*pOutputStream_) << std::left << std::setw(23) << trigger.name();

		// Print the thresholds
		std::vector<std::string> thresholdNames=l1menu::tools::getThresholdNames( trigger );
		for( size_t thresholdNumber=0; thresholdNumber<4; ++thresholdNumber )
		{
			if( thresholdNames.size()>thresholdNumber ) (*pOutputStream_) << delimeter_ << std::setw(7) << trigger.parameter(thresholdNames[thresholdNumber]);
			else (*pOutputStream_) << delimeter_ << std::setw(7) << " ";
		}

		// Print the rates
		(*pOutputStream_) << delimeter_ << std::setw(15) << pRate->rate();
		(*pOutputStream_) << delimeter_ << std::setw(15) << pRate->rateError();
		(*pOutputStream_) << delimeter_ << std::setw(11) << pRate->pureRate();
		(*pOutputStream_) << delimeter_ << std::setw(11) << pRate->pureRateError();

		// Print the threshold errors it they're available
		for( size_t thresholdNumber=0; thresholdNumber<4; ++thresholdNumber )
		{
			if( thresholdNames.size()>thresholdNumber && trigger.parameterErrorsAreAvailable(thresholdNames[thresholdNumber]) )
			{
				(*pOutputStream_) << delimeter_ << std::setw(9) << trigger.parameterErrorLow(thresholdNames[thresholdNumber])
						<< delimeter_ << std::setw(9) << trigger.parameterErrorHigh(thresholdNames[thresholdNumber]);
			}
			else (*pOutputStream_) << delimeter_ << std::setw(9) << " " << delimeter_ << std::setw(9) << " ";
		}

		totalNoOverlaps+=pRate->rate();
		totalPure+=pRate->pureRate();

		(*pOutputStream_) << "\n";
	}

	(*pOutputStream_) << "---------------------------------------------------------------------------------------------------------------" << "\n"
			<< " Total L1 Rate (with overlaps)    = " << delimeter_ << std::setw(8) << menuRates.totalRate() << delimeter_ << " +/- " << delimeter_ << menuRates.totalRateError() << delimeter_ << " kHz" << "\n"
			<< " Total L1 Rate (without overlaps) = " << delimeter_ << std::setw(8) << totalNoOverlaps << delimeter_ << " kHz" << "\n"
			<< " Total L1 Rate (pure triggers)    = " << delimeter_ << std::setw(8) << totalPure << delimeter_ << " kHz" << std::endl;

}

std::vector< std::unique_ptr<l1menu::TriggerMenu> > l1menu::implementation::OldL1MenuFile::getMenus() const
{
	// Create a vector to hold the output. Old files can only hold one menu, but
	// the interface requires that a vector is returned.
	std::vector< std::unique_ptr<l1menu::TriggerMenu> > returnValue;
	// Also create a new menu. I'll only add this to returnValue if all of the
	// required information can be read from the file.
	std::unique_ptr<l1menu::TriggerMenu> pNewMenu( new l1menu::TriggerMenu );

	const size_t bufferSize=200;
	char buffer[bufferSize];

	// Can't use the file_ member because this is a const method, and reading from
	// it will change the internal buffers, position etcetera. So I'll open the file
	// again and use a local object.
	std::ifstream inputFile( filename_ );
	if( !inputFile.is_open() ) throw std::runtime_error( "OldL1MenuFile::getMenus() - unable to open file \""+filename_+"\"" );

	// This vector stores all of the trigger constraints for each trigger. I need to keep a note of this because the
	// old file format stores what the bandwidth for each trigger should be, but I want the fraction of the total
	// bandwidth. Since I don't know that until I add up the bandwidth for each trigger I'll store this and calculate
	// the fraction at the end.
	// First is whether the thresholds are locked, second is the requested rate.
	std::vector< std::pair<bool,float> > triggerConstraints;
	double totalBandwidth=0; // The running total

	while( inputFile.good() )
	{
		try
		{
			// Get one line at a time
			inputFile.getline( buffer, bufferSize );

			// split the line by whitespace into columns
			std::vector<std::string> tableColumns=l1menu::tools::splitByWhitespace( buffer );

			if( tableColumns.size()==1 && tableColumns[0].empty() ) continue; // Allow blank lines without giving a warning
			if( tableColumns.size()!=12 ) throw std::runtime_error( "The line does not have the correct number of columns" );

			float prescale=l1menu::tools::convertStringToFloat( tableColumns[2] );
			if( prescale!=0 )
			{
				std::string triggerName=tableColumns[0];

				try
				{
					//std::cout << "Added trigger \"" << tableColumns[0] << "\"" << std::endl;
					l1menu::ITrigger& newTrigger=pNewMenu->addTrigger( triggerName ); // Try and create a trigger with the name supplied

					//
					// Check what the constraints are and store them.
					//
					bool lockThresholds=l1menu::tools::convertStringToFloat( tableColumns[11] );
					float requestedRate=l1menu::tools::convertStringToFloat( tableColumns[9] );
					totalBandwidth+=requestedRate;
					triggerConstraints.push_back( std::make_pair(lockThresholds,requestedRate) );

					// Different triggers will have different numbers of thresholds, and even different names. E.g. Single triggers
					// will have "threshold1" whereas a cross trigger will have "leg1threshold1", "leg2threshold1" etcetera. This
					// utility function will get the threshold names in the correct order.
					const auto& thresholdNames=l1menu::tools::getThresholdNames(newTrigger);
					if( thresholdNames.size()>=1 ) newTrigger.parameter(thresholdNames[0])=l1menu::tools::convertStringToFloat( tableColumns[3] );
					if( thresholdNames.size()>=2 ) newTrigger.parameter(thresholdNames[1])=l1menu::tools::convertStringToFloat( tableColumns[4] );
					if( thresholdNames.size()>=3 ) newTrigger.parameter(thresholdNames[2])=l1menu::tools::convertStringToFloat( tableColumns[5] );
					if( thresholdNames.size()>=4 ) newTrigger.parameter(thresholdNames[3])=l1menu::tools::convertStringToFloat( tableColumns[6] );

					float etaOrRegionCut=l1menu::tools::convertStringToFloat( tableColumns[7] );
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
					try{ newTrigger.parameter("muonQuality")=l1menu::tools::convertStringToFloat( tableColumns[8] ); }
					catch( std::exception& error ) { } // Do nothing, just try and convert the other parameters

					try{ newTrigger.parameter("leg1muonQuality")=l1menu::tools::convertStringToFloat( tableColumns[8] ); }
					catch( std::exception& error ) { } // Do nothing, just try and convert the other parameters

					try{ newTrigger.parameter("leg2muonQuality")=l1menu::tools::convertStringToFloat( tableColumns[8] ); }
					catch( std::exception& error ) { } // Do nothing, just try and convert the other parameters

				} // end of try block
				catch( std::exception& error )
				{
					std::cerr << "Unable to add trigger \"" << tableColumns[0] << "\" because: " << error.what() << std::endl;
				}
			} // end of "if( prescale!=0 )"

		} // end of try block
		catch( std::runtime_error& exception )
		{
			std::cerr << "Some error occured while processing the line \"" << buffer << "\":" << exception.what() << std::endl;
		}
	}

	//
	// Now that I know what the total bandwidth is I can calculate what
	// the fraction of the total is and set the trigger constraints.
	//
	if( triggerConstraints.size()!=pNewMenu->numberOfTriggers() )
	{
		throw std::runtime_error("OldL1MenuFile::getMenus() - Something went wrong creating the menus. Constraints are not the same size as the triggers.");
	}

	for( size_t index=0; index<pNewMenu->numberOfTriggers(); ++index )
	{
		if( !triggerConstraints[index].first ) // If thresholds aren't locked (default value of TriggerConstraint is fixed thresholds)
		{
			l1menu::TriggerConstraint& newConstraint=pNewMenu->getTriggerConstraint(index);
			newConstraint.type( l1menu::TriggerConstraint::Type::FRACTION_OF_BANDWIDTH );
			newConstraint.value( triggerConstraints[index].second/totalBandwidth );
		}
	}

	returnValue.push_back( std::move( pNewMenu ) );
	return returnValue;
}

std::vector< std::unique_ptr<l1menu::IMenuRate> > l1menu::implementation::OldL1MenuFile::getRates() const
{
	throw std::logic_error( "OldL1MenuFile::getRates() not implemented yet - Loading results from the old format file is not implemented yet. Might never be." );
}
