#include "TriggerRowEntry.h"
#include "TriggerRowEntry.moc"

#include <stdexcept>
#include <sstream>

#include "l1menu/ITrigger.h"
#include "l1menu/ITriggerDescriptionWithErrors.h"
#include "l1menu/ITriggerRate.h"
#include "l1menu/TriggerTable.h"
#include "l1menu/tools/miscellaneous.h"

l1menu::guiclasses::TriggerRowEntry::TriggerRowEntry( const l1menu::ITrigger& trigger )
	: pTrigger_( l1menu::TriggerTable::instance().copyTrigger(trigger) ),
	  widgetsShowOnlineThresholds_(true)
{
	pNameLabel_.reset( new QLabel( (trigger.name()+" v"+std::to_string(trigger.version())).c_str() ) );

	QDoubleSpinBox* pPrimaryThreshold=nullptr;
	for( const auto& thresholdName : l1menu::tools::getThresholdNames(trigger) )
	{
		std::unique_ptr<QDoubleSpinBox> pButton( new QDoubleSpinBox );
		pButton->setMinimum(0);
		pButton->setMaximum(1000);
		pButton->setValue( trigger.parameter(thresholdName) );
		pButton->setToolTip( thresholdName.c_str() );
		connect( pButton.get(), SIGNAL(valueChanged(double)), this, SLOT(thresholdWidgetChanged(double)) );

		// Default the online to offline scaling to slope=1, offset=0
		thresholdOnlineToOfflineScaling_.push_back( std::make_pair(1,0) );

		if( pPrimaryThreshold==nullptr )
		{
			pPrimaryThreshold=pButton.get();
			freeThresholds_.push_back( std::move(pButton) );
		}
		else
		{
			if( trigger.thresholdsAreCorrelated() )
			{
				pButton->setDisabled(true);
				double scaledValue=pButton->value()/pPrimaryThreshold->value();
				scaledThresholds_.push_back( std::make_pair(std::move(pButton),scaledValue) );
			}
			else freeThresholds_.push_back( std::move(pButton) );
		}
	}

	pRateLabel_.reset( new QLabel("?") );
	pRateErrorLabel_.reset( new QLabel("?") );
	pPureRateLabel_.reset( new QLabel("?") );
	pPureRateErrorLabel_.reset( new QLabel("?") );

	pRateLabel_->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
	pRateErrorLabel_->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
	pPureRateLabel_->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
	pPureRateErrorLabel_->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
}

size_t l1menu::guiclasses::TriggerRowEntry::numberOfFreeThresholds() const
{
	return freeThresholds_.size();
}

size_t l1menu::guiclasses::TriggerRowEntry::numberOfThresholds() const
{
	return freeThresholds_.size()+scaledThresholds_.size();
}

float l1menu::guiclasses::TriggerRowEntry::onlineThreshold( size_t thresholdIndex ) const
{
	throw std::runtime_error( "not implemented yet" );
//	if( thresholdIndex>=thresholdNames_.size() ) throw std::runtime_error( std::string("Invalid threshold index '")+std::to_string(thresholdIndex)+"' - "+__FILE__+" ("+std::to_string(__LINE__)+")" );
//	return pTrigger_->parameter( thresholdNames_[thresholdIndex] );
}

float l1menu::guiclasses::TriggerRowEntry::offlineThreshold( size_t thresholdIndex ) const
{
	throw std::runtime_error( "not implemented yet" );
//	if( thresholdIndex>=thresholdNames_.size() ) throw std::runtime_error( std::string("Invalid threshold index '")+std::to_string(thresholdIndex)+"' - "+__FILE__+" ("+std::to_string(__LINE__)+")" );
//	return scalingSlope_*pTrigger_->parameter( thresholdNames_[thresholdIndex] )+scalingOffset_;
}

void l1menu::guiclasses::TriggerRowEntry::setFromOnlineThreshold( size_t thresholdIndex, float threshold )
{
	throw std::runtime_error( "not implemented yet" );
//	if( thresholdIndex>=thresholdNames_.size() ) throw std::runtime_error( std::string("Invalid threshold index '")+std::to_string(thresholdIndex)+"' - "+__FILE__+" ("+std::to_string(__LINE__)+")" );
//	pTrigger_->parameter( thresholdNames_[thresholdIndex] )=threshold;
}

void l1menu::guiclasses::TriggerRowEntry::setFromOfflineThreshold( size_t thresholdIndex, float threshold )
{
	throw std::runtime_error( "not implemented yet" );
//	if( thresholdIndex>=thresholdNames_.size() ) throw std::runtime_error( std::string("Invalid threshold index '")+std::to_string(thresholdIndex)+"' - "+__FILE__+" ("+std::to_string(__LINE__)+")" );
//	pTrigger_->parameter( thresholdNames_[thresholdIndex] )=scalingSlope_*threshold+scalingSlope_;
}

const l1menu::ITriggerDescription& l1menu::guiclasses::TriggerRowEntry::getTrigger() const
{
	return *pTrigger_;
}

QWidget* l1menu::guiclasses::TriggerRowEntry::nameWidget()
{
	return pNameLabel_.get();
}

std::vector<QWidget*> l1menu::guiclasses::TriggerRowEntry::thresholdWidgets()
{
	std::vector<QWidget*> returnValue;

	for( const auto& pWidget : freeThresholds_ ) returnValue.push_back(pWidget.get());
	for( const auto& widgetScalePair : scaledThresholds_ ) returnValue.push_back(widgetScalePair.first.get());

	return returnValue;
}

QWidget* l1menu::guiclasses::TriggerRowEntry::rateLabelWidget()
{
	return pRateLabel_.get();
}

QWidget* l1menu::guiclasses::TriggerRowEntry::rateErrorLabelWidget()
{
	return pRateErrorLabel_.get();
}

QWidget* l1menu::guiclasses::TriggerRowEntry::pureRateLabelWidget()
{
	return pPureRateLabel_.get();
}

QWidget* l1menu::guiclasses::TriggerRowEntry::pureRateErrorLabelWidget()
{
	return pPureRateErrorLabel_.get();
}

void l1menu::guiclasses::TriggerRowEntry::enable()
{
	// Never enable the scaled thresholds, because I don't want the user to be able to
	// modify the values directly.
	for( const auto& pWidget : freeThresholds_ ) pWidget->setDisabled(false);
	pNameLabel_->setDisabled(false);
	pRateLabel_->setDisabled(false);
	pPureRateLabel_->setDisabled(false);
}

void l1menu::guiclasses::TriggerRowEntry::disable()
{
	// scaled thresholds are already disabled
	for( const auto& pWidget : freeThresholds_ ) pWidget->setDisabled(true);
	pNameLabel_->setDisabled(true);
	pRateLabel_->setDisabled(true);
	pPureRateLabel_->setDisabled(true);
}

void l1menu::guiclasses::TriggerRowEntry::setRate( const l1menu::ITriggerRate* pRate )
{
	// First perform a check to make sure this rate matches this trigger
	if( pTrigger_->name()!=pRate->trigger().name() ) throw std::runtime_error("TriggerWidget::setRate was called with an incompatible trigger");
	if( pTrigger_->version()!=pRate->trigger().version() ) throw std::runtime_error("TriggerWidget::setRate was called with an incompatible trigger version");

	std::stringstream widgetText;
	double roundedValue;

	roundedValue=static_cast<double>( static_cast<int>(pRate->rate()*100+0.5) )/100.0;
	widgetText << roundedValue;
	pRateLabel_->setText( widgetText.str().c_str() );

	widgetText.str("");
	roundedValue=static_cast<double>( static_cast<int>(pRate->rateError()*100+0.5) )/100.0;
	widgetText << roundedValue;
	pRateErrorLabel_->setText( widgetText.str().c_str() );

	widgetText.str("");
	roundedValue=static_cast<double>( static_cast<int>(pRate->pureRate()*100+0.5) )/100.0;
	widgetText << roundedValue;
	pPureRateLabel_->setText( widgetText.str().c_str() );

	widgetText.str("");
	roundedValue=static_cast<double>( static_cast<int>(pRate->pureRateError()*100+0.5) )/100.0;
	widgetText << roundedValue;
	pPureRateErrorLabel_->setText( widgetText.str().c_str() );
}

void l1menu::guiclasses::TriggerRowEntry::setEnabled( int state )
{
	if( state==Qt::Checked ) enable();
	else if( state==Qt::Unchecked ) disable();
}

void l1menu::guiclasses::TriggerRowEntry::thresholdWidgetChanged( double value )
{
	// The "value" parameter isn't actually used, because I can't tell which threshold
	// it's for. I just loop over all of them and read the values from the boxes.

	//
	// Update the trigger. This call loops over the widgets and reads their values.
	//
	setTriggerFromWidgets();

	// I also need to change the values shown in the widgets for the scaled thresholds.
	// It sounds a bit bizarre straight after setting the trigger, but the easiest way
	// of doing this is to call setWidgetsFromTrigger(). The call to
	// setTriggerFromWidgets() updates just the free thresholds from the values in the
	// widgets, then scales the scaled values from the free thresholds in the trigger
	// (i.e. the scaled threshold widgets are never queried). Since I need to take
	// account of the possible online to offline scaling it's easier to use the code
	// already in the setWidgetsFromTrigger method. The "true" parameter in this call
	// tells the method not to bother touching the free thresholds.
	setWidgetsFromTrigger( true );
}

void l1menu::guiclasses::TriggerRowEntry::setTriggerFromWidgets()
{
	// Loop over the free thresholds and propagate the values in the boxes
	// to the trigger.
	std::string primaryThresholdName;
	for( size_t index=0; index<freeThresholds_.size(); ++index )
	{
		auto& pWidget=freeThresholds_[index];
		auto& slopeOffsetPair=thresholdOnlineToOfflineScaling_[index];
		double newValue;
		// Convert offline to online threshold if necessary
		if( widgetsShowOnlineThresholds_ ) newValue=pWidget->value();
		else newValue=(pWidget->value()-slopeOffsetPair.second)/slopeOffsetPair.first;
		// Parameter name is in the tool tip, but need to convert QString to C
		// string with toLocal8Bit().data() methods.
		std::string thresholdName=pWidget->toolTip().toLocal8Bit().data();
		if( primaryThresholdName.empty() ) primaryThresholdName=thresholdName;
		pTrigger_->parameter( thresholdName )=newValue;
	}

	// If there are scaled thresholds, then I need to modify those too.
	// Take the primary threshold from the trigger since that will be the online
	// threshold, which is what I want to scale from (widget could be online or
	// offline).
	double primaryThresholdValue=pTrigger_->parameter( primaryThresholdName );
	for( auto& widgetScalePair : scaledThresholds_ )
	{
		double newValue=primaryThresholdValue*widgetScalePair.second;
		// Parameter name is in the tool tip, but need to convert QString to C
		// string with toLocal8Bit().data() methods.
		std::string thresholdName=widgetScalePair.first->toolTip().toLocal8Bit().data();
		pTrigger_->parameter( thresholdName )=newValue;
	}

}

void l1menu::guiclasses::TriggerRowEntry::setWidgetsFromTrigger( bool onlySetScaledThresholds )
{

	if( !onlySetScaledThresholds )
	{
		for( size_t index=0; index<freeThresholds_.size(); ++index )
		{
			auto& pWidget=freeThresholds_[index];
			auto& slopeOffsetPair=thresholdOnlineToOfflineScaling_[index];

			// Parameter name is in the tool tip, but need to convert QString to C
			// string with toLocal8Bit().data() methods.
			std::string thresholdName=pWidget->toolTip().toLocal8Bit().data();
			double onlineThreshold=pTrigger_->parameter( thresholdName );

			float newValue;
			if( widgetsShowOnlineThresholds_ ) newValue=onlineThreshold;
			else newValue=onlineThreshold*slopeOffsetPair.first+slopeOffsetPair.first;

			pWidget->setValue( newValue );
		}
	}


	for( size_t index=0; index<scaledThresholds_.size(); ++index )
	{
		auto& pWidget=scaledThresholds_[index].first;
		auto& slopeOffsetPair=thresholdOnlineToOfflineScaling_[index+freeThresholds_.size()];

		// Parameter name is in the tool tip, but need to convert QString to C
		// string with toLocal8Bit().data() methods.
		std::string thresholdName=pWidget->toolTip().toLocal8Bit().data();
		double onlineThreshold=pTrigger_->parameter( thresholdName );

		double newValue;
		if( widgetsShowOnlineThresholds_ ) newValue=onlineThreshold;
		else newValue=onlineThreshold*slopeOffsetPair.first+slopeOffsetPair.first;
		pWidget->setValue(newValue);
	}

}
