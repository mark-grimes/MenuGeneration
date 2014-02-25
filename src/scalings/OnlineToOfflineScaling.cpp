#include "l1menu/scalings/OnlineToOfflineScaling.h"

#include <stdexcept>
#include <iostream>
#include <map>
#include <fstream>
#include "l1menu/TriggerTable.h"
#include "l1menu/TriggerRatePlot.h"
#include "l1menu/MenuRatePlots.h"
#include "l1menu/IMenuRate.h"
#include "l1menu/ITrigger.h"
#include "l1menu/ITriggerDescriptionWithErrors.h"
#include "l1menu/tools/stringManipulation.h"
#include "l1menu/tools/miscellaneous.h"
#include "../implementation/MenuRateImplementation.h"
#include <TH1.h>

//
// Pimple class was implicitly declared, need to explicitly declare and define it
//
namespace l1menu
{
	namespace scalings
	{
		class OnlineToOfflineScalingPrivateMembers
		{
		public:
			// This is a map that uses the trigger name as a key. The value part is a vector
			// of the scalings for each of the thresholds, held as a pair with slope as "first"
			// and the offset as "second".
			std::map<std::string, std::vector<std::pair<float,float> > > triggerScalings_;
			void performScaling( TH1* pHistogramToScale, const std::string& triggerName );
			std::string detailedDescription_;
		};

	} // end of namespace scalings
} // end of namespace l1menu

l1menu::scalings::OnlineToOfflineScaling::OnlineToOfflineScaling( const std::string& offlineScalingFilename )
	: pImple( new OnlineToOfflineScalingPrivateMembers )
{
	pImple->detailedDescription_="Online to offline scaling filename: '"+offlineScalingFilename+"'";

	// Open the file that contains the scalings
	std::ifstream inputFile( offlineScalingFilename );
	if( !inputFile.is_open() ) throw std::runtime_error( "OnlineToOfflineScaling could not be constructed because the input file "+offlineScalingFilename+" could no be opened" );


	char buffer[200];
	while( inputFile.good() )
	{
		try
		{
			// Get one line at a time
			inputFile.getline( buffer, sizeof(buffer) );

			// split the line by whitespace into columns
			std::vector<std::string> tableColumns=l1menu::tools::splitByWhitespace( buffer );

			if( tableColumns.size()==1 && tableColumns[0].empty() ) continue; // Allow blank lines without giving a warning
			if( tableColumns.size()!=9 ) throw std::runtime_error( "OnlineToOfflineScaling could not be constructed because the input file does not have the correct number of columns" );

			// Read the trigger name and scalings from the file
			std::string triggerName=tableColumns[0];
			std::vector<std::pair<float,float> > scalings;

			using l1menu::tools::convertStringToFloat;

			scalings.push_back( std::make_pair( convertStringToFloat(tableColumns[1]), convertStringToFloat(tableColumns[2]) ) );
			scalings.push_back( std::make_pair( convertStringToFloat(tableColumns[3]), convertStringToFloat(tableColumns[4]) ) );
			scalings.push_back( std::make_pair( convertStringToFloat(tableColumns[5]), convertStringToFloat(tableColumns[6]) ) );
			scalings.push_back( std::make_pair( convertStringToFloat(tableColumns[7]), convertStringToFloat(tableColumns[8]) ) );

			// Then store them in the map
			pImple->triggerScalings_[triggerName]=scalings;

		} // end of try block
		catch( std::runtime_error& exception )
		{
			std::cerr << "Some error occured while processing the line \"" << buffer << "\":" << exception.what() << std::endl;
		}
	}
	inputFile.close();

}

l1menu::scalings::OnlineToOfflineScaling::~OnlineToOfflineScaling()
{
	// No operation
}

std::string l1menu::scalings::OnlineToOfflineScaling::briefDescription()
{
	return "Online thresholds scaled to offline thresholds.";
}

std::string l1menu::scalings::OnlineToOfflineScaling::detailedDescription()
{
	return "I'll write this later.";
}

std::unique_ptr<l1menu::TriggerRatePlot> l1menu::scalings::OnlineToOfflineScaling::scale( const l1menu::TriggerRatePlot& unscaledPlot )
{
	// Take a copy and work on that
	std::unique_ptr<l1menu::TriggerRatePlot> pReturnValue( new l1menu::TriggerRatePlot( unscaledPlot ) );

	TH1* pRawPlot=pReturnValue->getPlot();
	// First make sure it's held in memory only. The user can change that later if they want.
	pRawPlot->SetDirectory( nullptr );
	// Delegate to a private method to do the work
	pImple->performScaling( pRawPlot, pReturnValue->getTrigger().name() );

	return pReturnValue;
}

std::unique_ptr<l1menu::MenuRatePlots> l1menu::scalings::OnlineToOfflineScaling::scale( const l1menu::MenuRatePlots& unscaledPlots )
{
	// First take a copy of the input and modify that directly
	std::unique_ptr<l1menu::MenuRatePlots> pReturnValue( new l1menu::MenuRatePlots( unscaledPlots ) );

	// Loop over all of the plots and do the business
	for( auto& triggerRatePlot : pReturnValue->triggerRatePlots() )
	{
		TH1* pRawPlot=triggerRatePlot.getPlot();
		// First make sure all of them are only held in memory. The user can change that
		// later if they want.
		pRawPlot->SetDirectory( nullptr );
		// Delegate to a private method to do the work
		pImple->performScaling( pRawPlot, triggerRatePlot.getTrigger().name() );
	}

	return pReturnValue;
}

std::unique_ptr<l1menu::IMenuRate> l1menu::scalings::OnlineToOfflineScaling::scale( const l1menu::IMenuRate& unscaledMenuRate )
{
	// Create a new MenuRateImplementation. I need to access the extra setters that aren't in IMenuRate so
	// I'll keep hold of a raw pointer and operate on that.
	l1menu::implementation::MenuRateImplementation* pMenuRate=new l1menu::implementation::MenuRateImplementation;
	std::unique_ptr<l1menu::IMenuRate> pReturnValue( pMenuRate );

	// None of the rates change. What changes are the thresholds on the triggers, which are
	// modified so that they would give the same rate on the scaled rate plot as they do on
	// the unscaled plot. So I can copy over all of the rates directly.
	pMenuRate->setTotalFraction( unscaledMenuRate.totalFraction() );
	pMenuRate->setTotalFractionError( unscaledMenuRate.totalFractionError() );
	pMenuRate->setTotalRate( unscaledMenuRate.totalRate() );
	pMenuRate->setTotalRateError( unscaledMenuRate.totalRateError() );

	for( const auto& pUnscaledTriggerRate : unscaledMenuRate.triggerRates() )
	{
		const l1menu::ITriggerDescriptionWithErrors& trigger=pUnscaledTriggerRate->trigger();

		//
		// First I'll get a copy of the trigger whether it's a muon trigger or not. If it is a muon
		// trigger I'll overwrite the threshold information in the next "if" block. If it isn't, I'll
		// just leave it as it is.
		//
		std::unique_ptr<l1menu::ITrigger> pScaledTrigger=l1menu::TriggerTable::instance().copyTrigger( trigger );

		//
		// Modify the trigger thresholds as required.
		//
		const auto& iFindResult=pImple->triggerScalings_.find( trigger.name() );
		std::map< std::string, std::pair<float,float> > scaledThresholdErrors; // Store threshold errors here until I can set them
		if( iFindResult!=pImple->triggerScalings_.end() ) // If there is no scaling provided just pass it on unchanged
		{
			// This vector will have four entries, regardless of whether the trigger has four thresholds.
			// Normally the extra ones are just set to zero.
			const std::vector< std::pair<float,float> >& scalings=iFindResult->second;
			// Loop over the threshold names of the trigger.
			const std::vector<std::string> thresholdNames=l1menu::tools::getThresholdNames( *pScaledTrigger );
			for( size_t index=0; index<thresholdNames.size(); ++index )
			{
				float unscaledThreshold=pScaledTrigger->parameter( thresholdNames[index] );
				float slope=scalings[index].first;
				float offset=scalings[index].second;
				float scaledThreshold=unscaledThreshold*slope+offset;
				pScaledTrigger->parameter( thresholdNames[index] )=scaledThreshold;
				// Look to see if unscaled TriggerRate has any errors on the thresholds.
				// If it does, scale those too.
				if( trigger.parameterErrorsAreAvailable( thresholdNames[index] ) )
				{
					// I need to get the absolute values of these errors, not the difference to the threshold
					float unscaledLowEdge=unscaledThreshold-trigger.parameterErrorLow( thresholdNames[index] );
					float unscaledHighEdge=unscaledThreshold+trigger.parameterErrorHigh( thresholdNames[index] );
					// Then scale, and also convert back to the difference to the scaled threshold
					float scaledErrorLow=scaledThreshold-(unscaledLowEdge*slope+offset);
					float scaledErrorHigh=(unscaledHighEdge*slope+offset)-scaledThreshold;
					scaledThresholdErrors[thresholdNames[index]]=std::make_pair(scaledErrorLow,scaledErrorHigh);
				}
			}
		}


		// Now I can add the TriggerRate to the MenuRate. I can always copy the rates straight over because
		// it's the thresholds that are scaled.
		l1menu::implementation::TriggerRateImplementation scaledTriggerRate( *pScaledTrigger,
				pUnscaledTriggerRate->fraction(), pUnscaledTriggerRate->fractionError(),
				pUnscaledTriggerRate->rate(), pUnscaledTriggerRate->rateError(),
				pUnscaledTriggerRate->pureFraction(), pUnscaledTriggerRate->pureFractionError(),
				pUnscaledTriggerRate->pureRate(), pUnscaledTriggerRate->pureRateError() );
		// Now that I have created the TriggerRate I can set the errors on the thresholds that I
		// scaled earlier.
		for( const auto& nameErrorsPair : scaledThresholdErrors )
		{
			// TODO - Propagate errors for the scaling.
			//        This just scales the errors previously calculated.
			scaledTriggerRate.setParameterErrors( nameErrorsPair.first, nameErrorsPair.second.first, nameErrorsPair.second.second );
		}
		// Everything has been set so now I can add it to the MenuRate
		pMenuRate->addTriggerRate( std::move(scaledTriggerRate) );

	} // end of loop over unscaled TriggerRates

	return pReturnValue;
}

void l1menu::scalings::OnlineToOfflineScalingPrivateMembers::performScaling( TH1* pHistogramToScale, const std::string& triggerName )
{
	// If I have no scaling information for this trigger do nothing
	const auto& iFindResult=triggerScalings_.find( triggerName );
	if( iFindResult==triggerScalings_.end() ) return;


	// Currently short of time and just going to scale everything by the first threshold since that's
	// the only plot I want.
	// TODO - fix this
	const std::vector< std::pair<float,float> >& scalings=iFindResult->second;
	float slope=scalings[0].first;
	float offset=scalings[0].second;

	// Take a copy of the original before I change the binning
	std::unique_ptr<TH1> pClone( static_cast<TH1*>(pHistogramToScale->Clone()) );
	pClone->SetDirectory(nullptr);

	// The histogram is scaled by just changing the values along the x-axis because it's
	// a linear scaling. So I change the binning on the x-axis and then copy the contents
	// of the old bins back on top.
	int numberOfBins=pHistogramToScale->GetXaxis()->GetNbins();
	float lowEdge=pHistogramToScale->GetXaxis()->GetXmin();
	float highEdge=pHistogramToScale->GetXaxis()->GetXmax();
	pHistogramToScale->SetBins( numberOfBins, lowEdge*slope+offset, highEdge*slope+offset );

	// Bin contents will have been lost so copy it back over from the clone
	for( int bin=0; bin<=numberOfBins+1; ++bin ) // copy the underflow and overflow too (bin 0 and numberOfBins+1)
	{
		pHistogramToScale->SetBinContent( bin, pClone->GetBinContent(bin) );
		pHistogramToScale->SetBinError( bin, pClone->GetBinError(bin) );
	}
	pHistogramToScale->SetEntries( pClone->GetEntries() );

}
