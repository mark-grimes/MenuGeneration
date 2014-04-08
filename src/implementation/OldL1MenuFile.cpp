#include "./OldL1MenuFile.h"

#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include "l1menu/TriggerMenu.h"
#include "l1menu/ITriggerDescription.h"
#include "l1menu/ITriggerDescriptionWithErrors.h"
#include "l1menu/tools/miscellaneous.h"
#include "./MenuRateImplementation.h"

l1menu::implementation::OldL1MenuFile::OldL1MenuFile( std::ostream& outputStream, const char delimeter ) : pOutputStream_(&outputStream), delimeter_(delimeter)
{
}

l1menu::implementation::OldL1MenuFile::OldL1MenuFile( const std::string& inputFilename, const char delimeter ) : pOutputStream_(nullptr), delimeter_(delimeter)
{
	file_.open( inputFilename );
	if( file_.is_open() ) pOutputStream_=&file_;
}

l1menu::implementation::OldL1MenuFile::~OldL1MenuFile()
{

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
	return std::vector< std::unique_ptr<l1menu::TriggerMenu> >();
}

std::vector< std::unique_ptr<l1menu::IMenuRate> > l1menu::implementation::OldL1MenuFile::getRates() const
{
	return std::vector< std::unique_ptr<l1menu::IMenuRate> >();
}
