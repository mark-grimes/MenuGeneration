#include "AnalyseReducedSampleWidget.h"
#include "AnalyseReducedSampleWidget.moc"

#include "l1menu/ReducedSample.h"
#include "l1menu/TriggerMenu.h"
#include "l1menu/IMenuRate.h"
#include "l1menu/ITriggerRate.h"
#include "l1menu/TriggerConstraint.h"
#include "l1menu/IL1MenuFile.h"

l1menu::guiclasses::AnalyseReducedSampleWidget::AnalyseReducedSampleWidget( l1menu::ReducedSample& sample )
	: sample_(sample),
	  menu_(sample.getTriggerMenu())
{
	for( size_t index=0; index<menu_.numberOfTriggers(); ++index )
	{
		menuWidget_.addTrigger( menu_.getTrigger(index) );
	}

	// Use smart pointers all the way through so that if something goes wrong
	// the memory will be cleaned up. As soon as the object is added to another
	// QT object (e.g. addWidget() etcetera) the parent owns the memory, so
	// I can release the smart pointer.
	std::unique_ptr<QScrollArea> pScrollArea( new QScrollArea );
	pScrollArea->setWidget(&menuWidget_);
	pScrollArea->setWidgetResizable( true );

	std::unique_ptr<QVBoxLayout> pMainLayout( new QVBoxLayout );
	pMainLayout->addWidget( pScrollArea.release() );

	pCollisionRate_.reset( new QDoubleSpinBox() );
	pCollisionRate_->setMinimum(0);
	pCollisionRate_->setMaximum(100000);
	{ // Block to limit the scope of temporary variables
		const float scaleToKiloHz=1.0/1000.0;
		const float orbitsPerSecond=11246;
		//const float numberOfBunches=1380; // for 50 ns bunch spacing
		const float numberOfBunches=2760; // for 25 ns bunch spacing
		pCollisionRate_->setValue( orbitsPerSecond*numberOfBunches*scaleToKiloHz );
	}
	pCalculateRateButton_.reset( new QPushButton( "&Calculate rates" ) );
	QObject::connect( pCalculateRateButton_.get(), SIGNAL( clicked() ), this, SLOT( calculateRates() ) );

	std::unique_ptr<QHBoxLayout> pCollisionRateLineLayout( new QHBoxLayout );
	pCollisionRateLineLayout->addWidget( new QLabel("Collision rate (kHz)=") );
	pCollisionRateLineLayout->addWidget( pCollisionRate_.get() );
	pCollisionRateLineLayout->addWidget( pCalculateRateButton_.get() );
	std::unique_ptr<QWidget> pCollisionRateLine( new QWidget );
	pCollisionRateLine->setLayout( pCollisionRateLineLayout.release() );
	pMainLayout->addWidget( pCollisionRateLine.release() );

	std::unique_ptr<QPushButton> pSaveMenuButton( new QPushButton( "&Save menu" ) );
	QObject::connect( pSaveMenuButton.get(), SIGNAL( clicked() ), this, SLOT( saveMenu() ) );
	std::unique_ptr<QHBoxLayout> pTotalAndSaveButtonLineLayout( new QHBoxLayout );
	pTotalAndSaveButtonLineLayout->addWidget( menuWidget_.getRateLabel() );
	pTotalAndSaveButtonLineLayout->addWidget( pSaveMenuButton.release() );
	std::unique_ptr<QWidget> pTotalAndSaveButtonLine( new QWidget );
	pTotalAndSaveButtonLine->setLayout( pTotalAndSaveButtonLineLayout.release() );
	pMainLayout->addWidget( pTotalAndSaveButtonLine.release() );

	this->setLayout( pMainLayout.release() );

}

void l1menu::guiclasses::AnalyseReducedSampleWidget::calculateRates()
{
	// Disable the button while doing the calculation, so that the user can't
	// click on it multiple times.
	pCalculateRateButton_->setDisabled(true);

	sample_.setEventRate( pCollisionRate_->value() );
	menuWidget_.calculateRates( sample_ );

	// Re-enable the button
	pCalculateRateButton_->setDisabled(false);

}

void l1menu::guiclasses::AnalyseReducedSampleWidget::saveMenu()
{
	QString filename=QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("XML Files (*.xml)"));

	if( filename!="" )
	{
		std::unique_ptr<l1menu::IL1MenuFile> pOutputL1MenuFile=l1menu::IL1MenuFile::getOutputFile( l1menu::IL1MenuFile::FileFormat::XML, filename.toLocal8Bit().data() );

		// Create a menu by copying just the triggers that are active
		l1menu::TriggerMenu menuToSave=menuWidget_.currentMenu();

		// I also want to set the fraction of bandwidth constraint, which is only
		// used if the menu is scaled for a particular bandwidth. To work out what
		// this is I'll fit the menu again. I could read the rates from the text
		// boxes but the user could have edited the menu since the last fit.
		sample_.setEventRate( pCollisionRate_->value() );
		std::shared_ptr<const l1menu::IMenuRate> pMenuRate=sample_.rate(menuToSave);

		for( size_t index=0; index<menuToSave.numberOfTriggers(); ++index )
		{
			float fraction=pMenuRate->triggerRates()[index]->rate()/pMenuRate->totalRate();
			l1menu::TriggerConstraint& newConstraint=menuToSave.getTriggerConstraint(index);
			newConstraint.type( l1menu::TriggerConstraint::Type::FRACTION_OF_BANDWIDTH );
			newConstraint.value( fraction );
		}

		pOutputL1MenuFile->add( menuToSave );
	}

}
