#include <cppunit/extensions/HelperMacros.h>

//
// Forward declarations
//
namespace l1menu
{
	class TriggerMenu;
}

/** @brief A cppunit TestFixture to test loading and saving menus and results to a file.
 *
 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
 * @date 08/Apr/2014
 */
class TriggerMenuUnitTestSuite : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(TriggerMenuUnitTestSuite);
	CPPUNIT_TEST(testFormatsAreEqual);
	CPPUNIT_TEST(testFormatsGiveSameTriggerConstraints);
	CPPUNIT_TEST(testFormatsGiveSameResult);
	CPPUNIT_TEST_SUITE_END();

protected:

public:
	void setUp();

protected:
	void testFormatsAreEqual();
	void testFormatsGiveSameTriggerConstraints();
	void testFormatsGiveSameResult();

	// These are set in the setUp() method
	std::unique_ptr<l1menu::TriggerMenu> pMenuFromXMLFormat_;
	std::unique_ptr<l1menu::TriggerMenu> pMenuFromOldFormat_;
};





#include <cppunit/config/SourcePrefix.h>
#include <stdexcept>

#include "TestParameters.h"
#include "l1menu/IL1MenuFile.h"
#include "l1menu/TriggerMenu.h"
#include "l1menu/ITrigger.h"
#include "l1menu/TriggerConstraint.h"
#include "l1menu/ICachedTrigger.h"
#include "l1menu/ISample.h"
#include "l1menu/IEvent.h"
#include "l1menu/tools/fileIO.h"

CPPUNIT_TEST_SUITE_REGISTRATION(TriggerMenuUnitTestSuite);

void TriggerMenuUnitTestSuite::setUp()
{
	std::string inputXMLFilename=TestParameters<std::string>::instance().getParameter( "TEST_XMLMENU_FILENAME" );
	std::string inputOLDFilename=TestParameters<std::string>::instance().getParameter( "TEST_MENU_FILENAME" );
	std::unique_ptr<l1menu::IL1MenuFile> pXmlFile;
	std::unique_ptr<l1menu::IL1MenuFile> pOldFile;
	CPPUNIT_ASSERT_NO_THROW( pXmlFile=l1menu::IL1MenuFile::getInputFile( inputXMLFilename ) );
	CPPUNIT_ASSERT_NO_THROW( pOldFile=l1menu::IL1MenuFile::getInputFile( inputOLDFilename ) );

	std::vector< std::unique_ptr<l1menu::TriggerMenu> > menusFromXML=pXmlFile->getMenus();
	std::vector< std::unique_ptr<l1menu::TriggerMenu> > menusFromOLD=pOldFile->getMenus();

	CPPUNIT_ASSERT_EQUAL( menusFromXML.size(), menusFromOLD.size() );
	CPPUNIT_ASSERT( menusFromXML.size()>0 );

	pMenuFromXMLFormat_=std::move( menusFromXML.front() );
	pMenuFromOldFormat_=std::move( menusFromOLD.front() );

	CPPUNIT_ASSERT( pMenuFromXMLFormat_!=nullptr );
	CPPUNIT_ASSERT( pMenuFromOldFormat_!=nullptr );
}

void TriggerMenuUnitTestSuite::testFormatsAreEqual()
{
	//
	// Now that I have two menus (one from each of the file formats) I can run some checks to
	// make sure they both have the same values.
	//
	CPPUNIT_ASSERT_EQUAL( pMenuFromXMLFormat_->numberOfTriggers(), pMenuFromOldFormat_->numberOfTriggers() );

	// Loop over all of the triggers and check each one
	for( size_t triggerIndex=0; triggerIndex<pMenuFromXMLFormat_->numberOfTriggers(); ++triggerIndex )
	{
		l1menu::ITrigger& xmlTrigger=pMenuFromXMLFormat_->getTrigger( triggerIndex );
		l1menu::ITrigger& oldTrigger=pMenuFromOldFormat_->getTrigger( triggerIndex );

		CPPUNIT_ASSERT_EQUAL( xmlTrigger.name(), oldTrigger.name() );
		CPPUNIT_ASSERT_EQUAL( xmlTrigger.version(), oldTrigger.version() );
		CPPUNIT_ASSERT_EQUAL( xmlTrigger.thresholdsAreCorrelated(), oldTrigger.thresholdsAreCorrelated() );

		std::vector<std::string> xmlParameterNames=xmlTrigger.parameterNames();
		std::vector<std::string> oldParameterNames=oldTrigger.parameterNames();

		CPPUNIT_ASSERT_EQUAL( xmlParameterNames.size(), oldParameterNames.size() );
		for( size_t parameterIndex=0; parameterIndex<xmlParameterNames.size(); ++parameterIndex )
		{
			CPPUNIT_ASSERT_EQUAL( xmlParameterNames[parameterIndex], oldParameterNames[parameterIndex] );
			CPPUNIT_ASSERT_EQUAL( xmlTrigger.parameter( xmlParameterNames[parameterIndex] ), oldTrigger.parameter( oldParameterNames[parameterIndex] ) );
		} // end of loop over parameters
	} // end of loop over triggers
}

void TriggerMenuUnitTestSuite::testFormatsGiveSameTriggerConstraints()
{
	//
	// The number of constraints should be the same as the number of triggers
	//
	const size_t numberOfTriggers=pMenuFromXMLFormat_->numberOfTriggers();
	CPPUNIT_ASSERT_EQUAL( numberOfTriggers, pMenuFromOldFormat_->numberOfTriggers() );

	// Make sure I can't get too many
	CPPUNIT_ASSERT_THROW( pMenuFromXMLFormat_->getTriggerConstraint(numberOfTriggers), std::out_of_range );
	CPPUNIT_ASSERT_THROW( pMenuFromOldFormat_->getTriggerConstraint(numberOfTriggers), std::out_of_range );
	// Make sure I can get just enough
	CPPUNIT_ASSERT_NO_THROW( pMenuFromXMLFormat_->getTriggerConstraint(numberOfTriggers-1) );
	CPPUNIT_ASSERT_NO_THROW( pMenuFromOldFormat_->getTriggerConstraint(numberOfTriggers-1) );

	// Run through all of them and make sure they're equal
	for( size_t index=0; index<numberOfTriggers; ++index )
	{
		const l1menu::TriggerConstraint& xmlConstraint=pMenuFromXMLFormat_->getTriggerConstraint(index);
		const l1menu::TriggerConstraint& oldConstraint=pMenuFromOldFormat_->getTriggerConstraint(index);
		CPPUNIT_ASSERT_EQUAL( xmlConstraint.type(), oldConstraint.type() );
		CPPUNIT_ASSERT_DOUBLES_EQUAL( xmlConstraint.value(), oldConstraint.value(), 0.0000001 );
	}
}

void TriggerMenuUnitTestSuite::testFormatsGiveSameResult()
{
	//
	// Try running both menus over the same sample and make sure they
	// have the same results.
	//
	std::string sampleFilename=TestParameters<std::string>::instance().getParameter( "TEST_SAMPLE_FILENAME" );
	std::unique_ptr<l1menu::ISample> pSample;
	CPPUNIT_ASSERT_NO_THROW( pSample=l1menu::tools::loadSample( sampleFilename ) );

	//
	// First I need to create a series of cached triggers. These speed up running massively
	// because it cuts out the expensive string comparison when seeing if a trigger passes
	// an event.
	//
	std::vector< std::unique_ptr<l1menu::ICachedTrigger> > cachedTriggersFromXMLFormat;
	for( size_t index=0; index<pMenuFromXMLFormat_->numberOfTriggers(); ++index )
	{
		std::unique_ptr<l1menu::ICachedTrigger> newTrigger=pSample->createCachedTrigger(pMenuFromXMLFormat_->getTrigger(index));
		cachedTriggersFromXMLFormat.push_back( std::move(newTrigger) );
	}

	std::vector< std::unique_ptr<l1menu::ICachedTrigger> > cachedTriggersFromOldFormat;
	for( size_t index=0; index<pMenuFromOldFormat_->numberOfTriggers(); ++index )
	{
		std::unique_ptr<l1menu::ICachedTrigger> newTrigger=pSample->createCachedTrigger(pMenuFromOldFormat_->getTrigger(index));
		cachedTriggersFromOldFormat.push_back( std::move(newTrigger) );
	}

	CPPUNIT_ASSERT_EQUAL( cachedTriggersFromXMLFormat.size(), cachedTriggersFromOldFormat.size() );

	// Loop over all the events
	for( size_t eventNumber=0; eventNumber<pSample->numberOfEvents(); ++eventNumber )
	{
		const l1menu::IEvent& event=pSample->getEvent( eventNumber );

		// Loop over all of the triggers and check each one gives the same result
		for( size_t triggerIndex=0; triggerIndex<cachedTriggersFromXMLFormat.size(); ++triggerIndex )
		{
			std::unique_ptr<l1menu::ICachedTrigger>& pXmlTrigger=cachedTriggersFromXMLFormat[triggerIndex];
			std::unique_ptr<l1menu::ICachedTrigger>& pOldTrigger=cachedTriggersFromOldFormat[triggerIndex];
			CPPUNIT_ASSERT_EQUAL_MESSAGE( "Trigger gives a different result for event "+std::to_string(eventNumber), pXmlTrigger->apply(event), pOldTrigger->apply(event) );
		}
	}
}
