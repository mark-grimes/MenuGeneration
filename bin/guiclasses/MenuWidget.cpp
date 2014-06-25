#include "MenuWidget.h"
#include "MenuWidget.moc"

#include <sstream>

#include "l1menu/ITrigger.h"
#include "l1menu/ISample.h"
#include "l1menu/IMenuRate.h"
#include "TriggerRowEntry.h"

l1menu::guiclasses::MenuWidget::MenuWidget()
	: pLayout_( new QGridLayout ),
	  pTotalRateWidget_( new QLabel )
{
	this->setLayout( pLayout_.get() );
}

void l1menu::guiclasses::MenuWidget::addTrigger( const l1menu::ITrigger& trigger )
{
	std::unique_ptr<l1menu::guiclasses::TriggerRowEntry> pNewTableRow( new l1menu::guiclasses::TriggerRowEntry(trigger) );

	// Add a checkbox for each trigger to allow it to be enabled or disabled
	std::unique_ptr<QCheckBox> pCheckBox( new QCheckBox );
	pCheckBox->setTristate( false );
	pCheckBox->setCheckState( Qt::Checked );
	connect( pCheckBox.get(), SIGNAL(stateChanged(int)), pNewTableRow.get(), SLOT(setEnabled(int)) );

	int currentRow=pLayout_->rowCount();
	int currentColumn=0;

	pLayout_->addWidget( pCheckBox.get(), currentRow, currentColumn++ );
	pLayout_->addWidget( pNewTableRow->nameWidget(), currentRow, currentColumn++ );

	auto thresholdWidgets=pNewTableRow->thresholdWidgets();
	for( auto& pThresholdWidget : thresholdWidgets ) pLayout_->addWidget( pThresholdWidget, currentRow, currentColumn++ );
	currentColumn+=4-thresholdWidgets.size(); // Make sure 4 columns are reserved for the thresholds

	pLayout_->setColumnMinimumWidth( currentColumn++, 10 ); // Add a spacer column between the rate labels
	pLayout_->addWidget( pNewTableRow->rateLabelWidget(), currentRow, currentColumn++ );
	pLayout_->addWidget( new QLabel("+/-"), currentRow, currentColumn++ );
	pLayout_->addWidget( pNewTableRow->rateErrorLabelWidget(), currentRow, currentColumn++ );
	pLayout_->addWidget( new QLabel("kHz"), currentRow, currentColumn++ );

	pLayout_->setColumnMinimumWidth( currentColumn++, 10 ); // Add a spacer column between the rate labels
	pLayout_->addWidget( pNewTableRow->pureRateLabelWidget(), currentRow, currentColumn++ );
	pLayout_->addWidget( new QLabel("+/-"), currentRow, currentColumn++ );
	pLayout_->addWidget( pNewTableRow->pureRateErrorLabelWidget(), currentRow, currentColumn++ );
	pLayout_->addWidget( new QLabel("kHz"), currentRow, currentColumn++ );

	checkboxes_.push_back( std::move(pCheckBox) );
	tableRow_.push_back( std::move(pNewTableRow) );
}

l1menu::TriggerMenu l1menu::guiclasses::MenuWidget::currentMenu()
{
	l1menu::TriggerMenu returnValue;
	// Loop over all of the rows in the table, and for any that are not
	// disabled add them to the menu.
	for( size_t index=0; index<tableRow_.size(); ++index )
	{
		if( checkboxes_[index]->checkState()==Qt::Checked ) returnValue.addTrigger( tableRow_[index]->getTrigger() );
	}

	return returnValue;
}

void l1menu::guiclasses::MenuWidget::calculateRates( const l1menu::ISample& sample )
{
	l1menu::TriggerMenu menu=currentMenu();
	std::shared_ptr<const l1menu::IMenuRate> pMenuRate=sample.rate(menu);

	double roundedRate=static_cast<double>( static_cast<int>(pMenuRate->totalRate()*100+0.5) )/100.0;
	double roundedRateError=static_cast<double>( static_cast<int>(pMenuRate->totalRateError()*100+0.5) )/100.0;
	std::stringstream rateText;
	rateText << "Total rate= " << roundedRate << " +/- " << roundedRateError << " kHz";
	pTotalRateWidget_->setText( rateText.str().c_str() );

	// The trigger rates should be in the same order as the active trigger widgets
	std::vector<const l1menu::ITriggerRate*>::const_iterator iTriggerRate=pMenuRate->triggerRates().begin();
	for( size_t index=0; index<tableRow_.size(); ++index )
	{
		if( checkboxes_[index]->checkState()==Qt::Checked )
		{
			tableRow_[index]->setRate( *iTriggerRate );
			++iTriggerRate;
			if( iTriggerRate==pMenuRate->triggerRates().end() ) break;
		}
	}

}

QWidget* l1menu::guiclasses::MenuWidget::getRateLabel()
{
	return pTotalRateWidget_.get();
}
