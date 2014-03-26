#include <cppunit/extensions/HelperMacros.h>
#include "l1menu/TriggerMenu.h"

//
// Forward definitions
//
namespace l1menu
{
	class ISample;
	class TriggerRatePlot;
}

/** @brief A cppunit TestFixture to test TriggerRatePlot objects.
 *
 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
 * @date 04/Aug/2013
 */
class TriggerRatePlotUnitTestSuite : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(TriggerRatePlotUnitTestSuite);
	CPPUNIT_TEST(testConstructingFromTH1);
	CPPUNIT_TEST(testFindThresholdError);
	CPPUNIT_TEST_SUITE_END();

protected:
	std::ostream* pVerboseOutput_;
	std::unique_ptr<l1menu::ISample> pSample_;
	std::unique_ptr<l1menu::TriggerMenu> pTriggerMenu_;
	std::string inputSampleFilename_;
	std::string inputMenuFilename_;
	std::unique_ptr<l1menu::TriggerRatePlot> pTestTriggerRatePlot_;
public:
	TriggerRatePlotUnitTestSuite();
	void setUp();

protected:
	void testConstructingFromTH1();
	void testFindThresholdError();
};





#include <cppunit/config/SourcePrefix.h>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include "l1menu/ISample.h"
#include "l1menu/TriggerTable.h"
#include "l1menu/ITrigger.h"
#include "l1menu/TriggerRatePlot.h"
#include "l1menu/tools/miscellaneous.h"
#include "l1menu/tools/fileIO.h"
#include "TestParameters.h"
#include <TFile.h>
#include <TH1.h>

CPPUNIT_TEST_SUITE_REGISTRATION(TriggerRatePlotUnitTestSuite);

TriggerRatePlotUnitTestSuite::TriggerRatePlotUnitTestSuite() : pTriggerMenu_( new l1menu::TriggerMenu )
{
	pVerboseOutput_=nullptr;
	//pVerboseOutput_=&std::cout;

	inputSampleFilename_=TestParameters<std::string>::instance().getParameter( "TEST_SAMPLE_FILENAME" );
	inputMenuFilename_=TestParameters<std::string>::instance().getParameter( "TEST_MENU_FILENAME" );
}

void TriggerRatePlotUnitTestSuite::setUp()
{
	// Add a newline, because cppunit starts this function with half a line already written
	if( pVerboseOutput_!=nullptr ) *pVerboseOutput_ << "\n";

	if( pVerboseOutput_!=nullptr ) *pVerboseOutput_ << "Loading sample from file " << inputSampleFilename_ << std::endl;
	CPPUNIT_ASSERT_NO_THROW( pSample_=l1menu::tools::loadSample( inputSampleFilename_ ) );

	if( pVerboseOutput_!=nullptr ) *pVerboseOutput_ << "Loading menu from file " << inputMenuFilename_ << std::endl;
	CPPUNIT_ASSERT_NO_THROW( pTriggerMenu_=l1menu::tools::loadMenu( inputMenuFilename_ ) );
	CPPUNIT_ASSERT_MESSAGE( "TriggerMenu supplied needs at least one trigger for the tests", pTriggerMenu_->numberOfTriggers()>=1 );

	//
	// Create a trigger rate plot for testing
	//
	const l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();

	l1menu::ITrigger& trigger=pTriggerMenu_->getTrigger(1);
	std::vector<std::string> thresholdNames=l1menu::tools::getThresholdNames( trigger );
	CPPUNIT_ASSERT( !thresholdNames.empty() );
	std::string& mainThreshold=thresholdNames.front();

	unsigned int numberOfBins=100;
	float lowerEdge=0;
	float upperEdge=100;
	try
	{
		numberOfBins=triggerTable.getSuggestedNumberOfBins( trigger.name(), mainThreshold );
		lowerEdge=triggerTable.getSuggestedLowerEdge( trigger.name(), mainThreshold );
		upperEdge=triggerTable.getSuggestedUpperEdge( trigger.name(), mainThreshold );
	}
	catch( std::exception& error) { /* Do nothing. If no binning suggestions have been set for this trigger use the defaults I set above. */ }

	pTestTriggerRatePlot_.reset( new l1menu::TriggerRatePlot( trigger, "testRatePlot", numberOfBins, lowerEdge, upperEdge, mainThreshold, thresholdNames ) );
	if( pVerboseOutput_!=nullptr ) *pVerboseOutput_ << "Adding test sample to rate plot. This could take a while." << std::endl;
	pTestTriggerRatePlot_->addSample( *pSample_ );
}

void TriggerRatePlotUnitTestSuite::testConstructingFromTH1()
{
	const l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();

	//
	// First I need to create a TriggerRatePlot the normal way, so that I can
	// test creating a different one from its TH1 histogram. I'll take the first
	// trigger from the menu.
	//
	l1menu::ITrigger& trigger=pTriggerMenu_->getTrigger(1);
	std::vector<std::string> thresholdNames=l1menu::tools::getThresholdNames( trigger );
	CPPUNIT_ASSERT( !thresholdNames.empty() );
	std::string& mainThreshold=thresholdNames.front();

	unsigned int numberOfBins=100;
	float lowerEdge=0;
	float upperEdge=100;
	try
	{
		numberOfBins=triggerTable.getSuggestedNumberOfBins( trigger.name(), mainThreshold );
		lowerEdge=triggerTable.getSuggestedLowerEdge( trigger.name(), mainThreshold );
		upperEdge=triggerTable.getSuggestedUpperEdge( trigger.name(), mainThreshold );
	}
	catch( std::exception& error) { /* Do nothing. If no binning suggestions have been set for this trigger use the defaults I set above. */ }

	l1menu::TriggerRatePlot ratePlot( trigger, "testRatePlot", numberOfBins, lowerEdge, upperEdge, mainThreshold, thresholdNames );
	if( pVerboseOutput_!=nullptr ) *pVerboseOutput_ << "Adding test sample to rate plot. This could take a while." << std::endl;
	ratePlot.addSample( *pSample_ );

	//
	// Now I have a functional TriggerRatePlot, I'll try and create another one just from the
	// underlying TH1 and see if the two objects are the same.
	//
	l1menu::TriggerRatePlot duplicateRatePlot( ratePlot.getPlot() );

	// Check that the threshold plotted against is the same
	CPPUNIT_ASSERT_EQUAL( ratePlot.versusParameter(), duplicateRatePlot.versusParameter() );

	std::vector<std::string> parametersAlreadyChecked;
	parametersAlreadyChecked.push_back( ratePlot.versusParameter() );

	// Check that parameters that are scaled along with the main threshold are all the same.
	const auto expectedScaledParameters=ratePlot.otherScaledParameters();
	const auto actualScaledParameters=duplicateRatePlot.otherScaledParameters();

	CPPUNIT_ASSERT_EQUAL( expectedScaledParameters.size(), actualScaledParameters.size() );

	for( auto iExpected=expectedScaledParameters.begin(), iActual=actualScaledParameters.begin();
			iExpected!=expectedScaledParameters.end() && iActual!=actualScaledParameters.end();
			++iExpected, ++iActual )
	{
		CPPUNIT_ASSERT_EQUAL( iExpected->first, iActual->first );
		CPPUNIT_ASSERT_EQUAL( iExpected->second, iActual->second );
		parametersAlreadyChecked.push_back( iExpected->first );
	}

	// Now compare the triggers
	const l1menu::ITriggerDescription& expectedTrigger=ratePlot.getTrigger();
	const l1menu::ITriggerDescription& actualTrigger=duplicateRatePlot.getTrigger();
	CPPUNIT_ASSERT_EQUAL( expectedTrigger.name(), actualTrigger.name() );
	CPPUNIT_ASSERT_EQUAL( expectedTrigger.version(), actualTrigger.version() );

	// I'll assume that if the trigger name and version are the same (checked above) that
	// I don't need to test that the parameter names are all the same.
	for( const auto& parameterName : expectedTrigger.parameterNames() )
	{
		// Make sure this parameter isn't one of the ones that are scaled
		if( std::find(parametersAlreadyChecked.begin(),parametersAlreadyChecked.end(),parameterName)==parametersAlreadyChecked.end() )
		{
			CPPUNIT_ASSERT_DOUBLES_EQUAL( expectedTrigger.parameter(parameterName), actualTrigger.parameter(parameterName), std::pow(10,-7) );
		}
	}
}

void TriggerRatePlotUnitTestSuite::testFindThresholdError()
{
	//
	// Not much I can test here except that it runs. To test anything else I need
	// to know what the answers should be. Maybe later I'll create a histogram
	// with purely fake data so that I can determine the results.
	//
	std::pair<float,float> thresholdErrors;
	CPPUNIT_ASSERT_NO_THROW( thresholdErrors=pTestTriggerRatePlot_->findThresholdError( 50 ) );
	// Although I can make sure it throws an exception if you get
	// the underflow and overflow bins.
	CPPUNIT_ASSERT_THROW( thresholdErrors=pTestTriggerRatePlot_->findThresholdError( -1 ), std::runtime_error );
	CPPUNIT_ASSERT_THROW( thresholdErrors=pTestTriggerRatePlot_->findThresholdError( 99999999 ), std::runtime_error );
}
