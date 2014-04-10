#include "l1menu/MenuFitter.h"

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include "l1menu/ICachedTrigger.h"
#include "l1menu/ISample.h"
#include "l1menu/IEvent.h"
#include "l1menu/IMenuRate.h"
#include "l1menu/ITriggerRate.h"
#include "l1menu/TriggerMenu.h"
#include "l1menu/ITrigger.h"
#include "l1menu/TriggerRatePlot.h"
#include "l1menu/TriggerConstraint.h"
#include "l1menu/MenuRatePlots.h"
#include "l1menu/tools/miscellaneous.h"
#include "l1menu/tools/fileIO.h"
#include "l1menu/tools/stringManipulation.h"

namespace // Use the unnamed namespace for things only used here
{
	/** @brief Structure to collate a few things about triggers that can be scaled
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 08/Jul/2013
	 */
	struct TriggerScalingDetails
	{
		size_t triggerNumber; ///< The number of the trigger in the trigger table
		float bandwidthFraction; ///< The fraction of the total bandwidth requested for this trigger
		float currentBandwidth;
		l1menu::TriggerRatePlot ratePlot; ///< The rate plot for this trigger
		std::string mainThreshold;
		std::vector< std::pair<std::string,float> > thresholdScalings; ///< The constant to scale each threshold compared to the main threshold
	};
} // end of the unnamed namespace


//
// Need to declare the pimple. So far it's just been forward declared.
//
namespace l1menu
{
	/** @brief Private members for the MenuFitter wrapped up in a compiler firewall.
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 08/Jul/2013
	 */
	class MenuFitterPrivateMembers
	{
	public:
		MenuFitterPrivateMembers( const l1menu::ISample& newSample, const l1menu::TriggerMenu& newMenu, const l1menu::MenuRatePlots* pRatePlots )
			: sample(newSample), menu(newMenu)
		{
			// If a l1menu::MenuRatePlots has been provided then I need to take a copy.
			if( pRatePlots!=nullptr ) pMenuRatePlots.reset( new l1menu::MenuRatePlots(*pRatePlots) );
			else
			{
				pMenuRatePlots.reset( new MenuRatePlots(menu) );
				pMenuRatePlots->addSample(sample);
			}

			//
			// Run over the new menu and have a look at the TriggerConstraints. If any
			// are for fraction of total bandwidth create a ::TriggerScalingDetails object
			// for it.
			//
			for( size_t index=0; index<menu.numberOfTriggers(); ++index )
			{
				l1menu::TriggerConstraint& constraint=menu.getTriggerConstraint( index );
				if( constraint.type()==l1menu::TriggerConstraint::Type::FRACTION_OF_BANDWIDTH )
				{
					addBandwidthConstraint( index, constraint.value() );
				}
			}
		}
		const l1menu::ISample& sample;
		std::unique_ptr<l1menu::MenuRatePlots> pMenuRatePlots;
		l1menu::TriggerMenu menu;
		std::vector< ::TriggerScalingDetails > scalableTriggers;
		std::vector<std::pair<size_t,float> > bandwidthFractions;
		void addBandwidthConstraint( size_t triggerNumber, float fractionOfTotalBandwidth );
		std::stringstream debugLog;
	};

}

l1menu::MenuFitter::MenuFitter( const l1menu::ISample& sample, const l1menu::TriggerMenu& menu )
	: pImple_( new MenuFitterPrivateMembers(sample,menu,NULL) )
{
	// No operation
}

l1menu::MenuFitter::MenuFitter( const l1menu::ISample& sample, const l1menu::TriggerMenu& menu, const l1menu::MenuRatePlots& menuRatePlots )
	: pImple_( new MenuFitterPrivateMembers(sample,menu,&menuRatePlots) )
{
	// No operation
}

l1menu::MenuFitter::~MenuFitter()
{
	// No operation
}

const l1menu::TriggerMenu& l1menu::MenuFitter::menu() const
{
	return pImple_->menu;
}

const l1menu::TriggerRatePlot& l1menu::MenuFitter::triggerRatePlot( size_t triggerNumber ) const
{
	for( const auto& triggerScalingDetails : pImple_->scalableTriggers )
	{
		if( triggerScalingDetails.triggerNumber==triggerNumber ) return triggerScalingDetails.ratePlot;
	}

	// If control got this far then there is no plot
	throw std::runtime_error( "l1menu::MenuFitter::triggerRatePlot was asked for a plot for a trigger that doesn't have a plot" );
}

const l1menu::MenuRatePlots& l1menu::MenuFitter::menuRatePlots() const
{
	return *pImple_->pMenuRatePlots;
}

std::shared_ptr<const l1menu::IMenuRate> l1menu::MenuFitter::fit( float totalRate, float tolerance )
{
	// Clear the log from whatever might be there from previous fits
	pImple_->debugLog.str("");

	//
	// First set all the thresholds to be the ratio of the total bandwidth that
	// the user specified. With correlations this will probably mean the total
	// will come out less than the requested rate, but it's a good first step.
	// Loop over all of the triggers whose thresholds are not fixed.
	//
	for( auto& triggerScalingDetails : pImple_->scalableTriggers )
	{
		l1menu::ITrigger& trigger=pImple_->menu.getTrigger( triggerScalingDetails.triggerNumber );

//		pImple_->debugLog << "Trying to find a threshold for " << trigger.name()
//				<< " to try and get a rate of " << totalRate*triggerScalingDetails.bandwidthFraction
//				<< ". Plot title is " << triggerScalingDetails.ratePlot.getPlot()->GetTitle() << std::endl;

		float& mainThreshold=trigger.parameter( triggerScalingDetails.mainThreshold );
		// Figure out what threshold should give the target rate for this particular trigger.
		// Note this is a reference so this command changes the trigger.
		triggerScalingDetails.currentBandwidth=totalRate*triggerScalingDetails.bandwidthFraction;
		mainThreshold=triggerScalingDetails.ratePlot.findThreshold( triggerScalingDetails.currentBandwidth );
		// Then scale all of the others off this
		for( const auto& nameScalePair : triggerScalingDetails.thresholdScalings )
		{
			trigger.parameter( nameScalePair.first )=mainThreshold*nameScalePair.second;
		}

		pImple_->debugLog << "Initially setting threshold for " << std::setw(20) << trigger.name() << " to " << std::setw(10) << mainThreshold << " to try and get a rate of " << totalRate*triggerScalingDetails.bandwidthFraction << std::endl;
	}

	// Then work out what the total rate is
	std::shared_ptr<const l1menu::IMenuRate> pMenuRate;
	pMenuRate=pImple_->sample.rate( pImple_->menu, menuRatePlots() );

	l1menu::tools::dumpTriggerRates( pImple_->debugLog, *pMenuRate );

	size_t iterationNumber=0;
	while ( std::fabs(pMenuRate->totalRate()-totalRate)>tolerance )
	{
		if( iterationNumber>10 ) throw std::runtime_error( "Too many iterations" );
		++iterationNumber;

		float scaleAllBandwidthsBy=totalRate/pMenuRate->totalRate();
		pImple_->debugLog << "\n" << "New loop. Last iteration had a rate of " << pMenuRate->totalRate() << ". Scaling all bandwidths by " << scaleAllBandwidthsBy << " to try and get " << totalRate << std::endl;

		for( auto& triggerScalingDetails : pImple_->scalableTriggers )
		{
			const size_t& triggerNumber=triggerScalingDetails.triggerNumber;
			l1menu::ITrigger& trigger=pImple_->menu.getTrigger( triggerNumber );
			const l1menu::ITriggerRate* pTriggerRate=pMenuRate->triggerRates()[triggerNumber];

			float& mainThreshold=trigger.parameter( triggerScalingDetails.mainThreshold );
			// Figure out what threshold should give the target rate for this particular trigger.
			triggerScalingDetails.currentBandwidth*=scaleAllBandwidthsBy;
			mainThreshold=triggerScalingDetails.ratePlot.findThreshold( triggerScalingDetails.currentBandwidth );

			// Then scale all of the others off this
			for( const auto& nameScalePair : triggerScalingDetails.thresholdScalings )
			{
				trigger.parameter( nameScalePair.first )=mainThreshold*nameScalePair.second;
			}
			pImple_->debugLog << "Changing threshold for " << std::setw(20) << trigger.name() << " to " << std::setw(10) << mainThreshold << " to try and change the rate from " << std::setw(10) << pTriggerRate->rate() << " to " << pTriggerRate->rate()*scaleAllBandwidthsBy << std::endl;

		} // end of loop over triggers I'm allowed to change thresholds for

		pMenuRate=pImple_->sample.rate( pImple_->menu, menuRatePlots() );
		l1menu::tools::dumpTriggerRates( pImple_->debugLog, *pMenuRate );
	}

	return pMenuRate;
}

const std::string l1menu::MenuFitter::debugLog()
{
	return pImple_->debugLog.str();
}

void l1menu::MenuFitter::addTrigger( const l1menu::ITrigger& trigger, const TriggerConstraint& constraint )
{
	size_t triggerNumber=pImple_->menu.numberOfTriggers(); // This will be the number of the next trigger added
	pImple_->menu.addTrigger( trigger );
	pImple_->menu.getTriggerConstraint( triggerNumber )=constraint;
	if( constraint.type()==l1menu::TriggerConstraint::Type::FRACTION_OF_BANDWIDTH )
	{
		pImple_->addBandwidthConstraint( triggerNumber, constraint.value() );
	}
}

void l1menu::MenuFitterPrivateMembers::addBandwidthConstraint( size_t triggerNumber, float fractionOfTotalBandwidth )
{
	const l1menu::ITrigger& newTrigger=menu.getTrigger( triggerNumber );

	const std::vector<std::string> thresholdNames=l1menu::tools::getThresholdNames(newTrigger);
	const std::string& mainThreshold=thresholdNames.front();

	// Record the scaling between the main threshold and all of the others, so that
	// when they get increased/decreased it's all done proportionally.
	std::vector< std::pair<std::string,float> > thresholdScalings;
	const float mainThresholdValue=newTrigger.parameter(mainThreshold);
	for( const auto& thresholdName : thresholdNames )
	{
		if( thresholdName==mainThreshold ) continue;
		thresholdScalings.push_back( std::make_pair( thresholdName, newTrigger.parameter(thresholdName)/mainThresholdValue ) );
	}

	//
	// I need a rate plot for this trigger. First check to see if there is one that matches
	// in the rate plots that might have been given in the constructor.
	//
	const l1menu::TriggerRatePlot* pPreviouslyCreatedRatePlot=nullptr;
	if( pMenuRatePlots!=nullptr )
	{
		for( const auto& triggerRatePlot : pMenuRatePlots->triggerRatePlots() )
		{
			// See if the plot was made with a trigger equivalent to this one
			if( triggerRatePlot.triggerMatches(newTrigger) )
			{
				pPreviouslyCreatedRatePlot=&triggerRatePlot;
				break;
			}
		}
	}

	if( pPreviouslyCreatedRatePlot!=nullptr )
	{
		//std::cout << "Found previously created plot for " << newTrigger.name() << ". Title is " << pPreviouslyCreatedRatePlot->getPlot()->GetTitle() << std::endl;
		//
		// Bundle all of this information in the helper structure I wrote in
		// the unnamed namespace.
		//
		scalableTriggers.push_back( ::TriggerScalingDetails{triggerNumber,fractionOfTotalBandwidth,0,*pPreviouslyCreatedRatePlot,mainThreshold,std::move(thresholdScalings)} );
	}
	else
	{
		//std::cout << "Need to create plot for " << newTrigger.name() << std::endl;
		//
		// Either no rate plots were supplied or a suitable one wasn't found, so I need to
		// create a rate plot for this trigger.
		//
		unsigned int numberOfBins=100;
		float lowerEdge=0;
		float upperEdge=100;
		try
		{
			l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();
			numberOfBins=triggerTable.getSuggestedNumberOfBins( newTrigger.name(), mainThreshold );
			lowerEdge=triggerTable.getSuggestedLowerEdge( newTrigger.name(), mainThreshold );
			upperEdge=triggerTable.getSuggestedUpperEdge( newTrigger.name(), mainThreshold );
		}
		catch( std::exception& error) { /* Do nothing. If no binning suggestions have been set for this trigger use the defaults I set above. */ }

		l1menu::TriggerRatePlot ratePlot(newTrigger,newTrigger.name()+"_v_allThresholdsScaled",numberOfBins,lowerEdge,upperEdge,mainThreshold,thresholdNames);
		ratePlot.addSample( sample );

		//
		// Bundle all of this information in the helper structure I wrote in
		// the unnamed namespace.
		//
		scalableTriggers.push_back( ::TriggerScalingDetails{triggerNumber,fractionOfTotalBandwidth,0,std::move(ratePlot),mainThreshold,std::move(thresholdScalings)} );
	} // end of else block where pPreviouslyCreatedRatePlot is null

}
