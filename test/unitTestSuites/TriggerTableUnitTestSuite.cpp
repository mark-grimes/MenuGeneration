#include <cppunit/extensions/HelperMacros.h>


/** @brief A cppunit TestFixture to test getting triggers from the table and getting and
 * setting their parameters.
 *
 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
 * @date 04/Aug/2013
 */
class TriggerTableUnitTestSuite : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(TriggerTableUnitTestSuite);
	CPPUNIT_TEST(testGettingAndSettingAllTriggerParameters);
	CPPUNIT_TEST(testRegionCut);
	//CPPUNIT_TEST(dumpTriggerTable); // Commented this out because it's pointless and messy
	CPPUNIT_TEST_SUITE_END();

protected:
	std::ostream* pVerboseOutput_;
public:
	void setUp();

protected:
	void testGettingAndSettingAllTriggerParameters();
	void testRegionCut();
	/** @brief Not really a test as such, just prints out all the triggers for the
	 * user to see what triggers are registered. */
	void dumpTriggerTable();
};





#include <cppunit/config/SourcePrefix.h>
#include "l1menu/TriggerTable.h"
#include "l1menu/ITrigger.h"
#include "l1menu/ISample.h"
#include "l1menu/L1TriggerDPGEvent.h"
#include "UserCode/L1TriggerUpgrade/interface/L1AnalysisDataFormat.h"
#include <stdexcept>
#include <cmath>
#include <iomanip>

CPPUNIT_TEST_SUITE_REGISTRATION(TriggerTableUnitTestSuite);

//
// Use the unnamed namespace for things only used in this file
//
namespace
{
	/** @brief Dummy implementation of ISample
	 *
	 * Only required because some of the tests create fake L1TriggerDPGEvent objects, and
	 * the constructor requires a reference to an ISample. This only contains enough code
	 * so that an instantiation can be made.
	 *
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 21/Apr/2014
	 */
	class MockSample : public l1menu::ISample
	{
	public:
		virtual size_t numberOfEvents() const { throw std::runtime_error("MockSample has no functionality"); }
		virtual const l1menu::IEvent& getEvent( size_t eventNumber ) const { throw std::runtime_error("MockSample has no functionality"); }
		virtual std::unique_ptr<l1menu::ICachedTrigger> createCachedTrigger( const l1menu::ITrigger& trigger ) const { throw std::runtime_error("MockSample has no functionality"); }
		virtual float eventRate() const { throw std::runtime_error("MockSample has no functionality"); }
		virtual void setEventRate( float rate ) { throw std::runtime_error("MockSample has no functionality"); }
		virtual float sumOfWeights() const { throw std::runtime_error("MockSample has no functionality"); }
		virtual std::shared_ptr<const l1menu::IMenuRate> rate( const l1menu::TriggerMenu& menu ) const { throw std::runtime_error("MockSample has no functionality"); }
		virtual std::shared_ptr<const l1menu::IMenuRate> rate( const l1menu::TriggerMenu& menu, const l1menu::MenuRatePlots& ratePlots ) const { throw std::runtime_error("MockSample has no functionality"); }
	};

} // end of the unnamed namespace

void TriggerTableUnitTestSuite::setUp()
{
	pVerboseOutput_=nullptr;
	//pVerboseOutput_=&std::cout;
}

void TriggerTableUnitTestSuite::testGettingAndSettingAllTriggerParameters()
{
	// Add a newline, because cppunit starts this function with half a line already written
	if( pVerboseOutput_!=nullptr ) *pVerboseOutput_ << "\n";
	l1menu::TriggerTable& table=l1menu::TriggerTable::instance();

	const auto& allTriggerDetails=table.listTriggers();

	// Loop over all of the triggers
	for( const auto& triggerDetails : allTriggerDetails )
	{
		std::unique_ptr<l1menu::ITrigger> pTrigger=table.getTrigger( triggerDetails.name, triggerDetails.version );
		CPPUNIT_ASSERT( pTrigger!=nullptr );

		if( pVerboseOutput_!=nullptr ) *pVerboseOutput_ << "Testing trigger " << triggerDetails.name << " v" << triggerDetails.version << std::endl;

		const auto& parameterNames=pTrigger->parameterNames();
		for( const auto& parameterName : parameterNames )
		{
			if( pVerboseOutput_!=nullptr ) *pVerboseOutput_ << "\t" << "testing parameter \"" << parameterName << "\"" << std::endl;

			CPPUNIT_ASSERT_NO_THROW( pTrigger->parameter(parameterName) );
			float newValue=std::rand();
			pTrigger->parameter(parameterName)=newValue;
			CPPUNIT_ASSERT_DOUBLES_EQUAL( newValue, pTrigger->parameter(parameterName), std::pow(10,-7) );
		}
	}
}

void TriggerTableUnitTestSuite::testRegionCut()
{
	//
	// Set up a mock event that I can inject data into for the test
	//
	::MockSample mockSample;
	l1menu::L1TriggerDPGEvent mockEvent( mockSample );
	mockEvent.physicsBits()[0]=true; // most (all?) triggers check this. I've never understood why.
	// add a high pT calo jet with arbitrary other parameters. I'll set the eta properly later.
	++mockEvent.rawEvent().Njet;
	mockEvent.rawEvent().Bxjet.push_back(0);
	mockEvent.rawEvent().Etjet.push_back(90.0);
	mockEvent.rawEvent().Phijet.push_back(0.0);
	mockEvent.rawEvent().Etajet.push_back(0.0);
	mockEvent.rawEvent().Taujet.push_back(false);
	mockEvent.rawEvent().isoTaujet.push_back(false);
	mockEvent.rawEvent().Fwdjet.push_back(false);
	// second jet for double jet trigger
	++mockEvent.rawEvent().Njet;
	mockEvent.rawEvent().Bxjet.push_back(0);
	mockEvent.rawEvent().Etjet.push_back(10.0); // lower pT for second jet so that it doesn't affect single jet test
	mockEvent.rawEvent().Phijet.push_back(0.0);
	mockEvent.rawEvent().Etajet.push_back(0.0);
	mockEvent.rawEvent().Taujet.push_back(false);
	mockEvent.rawEvent().isoTaujet.push_back(false);
	mockEvent.rawEvent().Fwdjet.push_back(false);

//	// add a track jet as well
//	mockEvent.rawEvent().NTkjet=1;
//	mockEvent.rawEvent().BxTkjet.push_back(0);
//	mockEvent.rawEvent().EtTkjet.push_back(0.0);
//	mockEvent.rawEvent().PhiTkjet.push_back(0.0);
//	mockEvent.rawEvent().EtaTkjet.push_back(0.0);
//	mockEvent.rawEvent().zVtxTkjet.push_back(0.0);

	const float testRegionCut=6.0;
	//
	// At the moment (21/Apr/2014) rawEvent().Etajet is in calorimeter region. This
	// is about to be changed to absolute eta, and all handling of of the value will
	// have to be changed accordingly. I'll test that handling here. First I need to
	// make sure the test works, so I'll set it in calorimeter region for now.
	//
	mockEvent.rawEvent().Etajet[0]=testRegionCut;
	mockEvent.rawEvent().Etajet[1]=10; // Put it central so the eta test is only on the first jet
	// define a delta value, so that I can test triggers with a cut just less than, equal
	// to, and just more than.
	float triggerCutDelta=0.1;

	l1menu::TriggerTable& table=l1menu::TriggerTable::instance();

	//
	// Test L1_SingleJetC trigger.
	//
	// Version 0 was the last one that used region cuts, so I need to make sure that
	// still gives the same result when changing the internal storage from region cut
	// to absolute eta.
	//
	std::unique_ptr<l1menu::ITrigger> pTrigger=table.getTrigger( "L1_SingleJetC", 0 );
	CPPUNIT_ASSERT( pTrigger!=nullptr );
	// Make sure it less than the first mock jet, so that only eta cut is tested. Also make
	// sure that is more than the second jet so that doesn't affect the test.
	pTrigger->parameter("threshold1")=20;

	// Test just less than
	pTrigger->parameter("regionCut")=testRegionCut-triggerCutDelta;
	CPPUNIT_ASSERT_EQUAL( true, pTrigger->apply(mockEvent) );
	// Test equal to
	pTrigger->parameter("regionCut")=testRegionCut;
	CPPUNIT_ASSERT_EQUAL( true, pTrigger->apply(mockEvent) );
	// Test just more than
	pTrigger->parameter("regionCut")=testRegionCut+triggerCutDelta;
	CPPUNIT_ASSERT_EQUAL( false, pTrigger->apply(mockEvent) );
}

void TriggerTableUnitTestSuite::dumpTriggerTable()
{
	// No tests performed with this one, just prints out the available triggers
	// for the user to inspect and see what's registered.
	l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();

	std::cout << "\n"
			<< "------ Available triggers ------" << "\n"
			<< std::left << std::setw(25) << "Name" << "Version" << "\n"
			<< "--------------------------------" << std::endl;
	std::vector<l1menu::TriggerTable::TriggerDetails> listOfTriggers=triggerTable.listTriggers();
	for( std::vector<l1menu::TriggerTable::TriggerDetails>::const_iterator iTriggerEntry=listOfTriggers.begin(); iTriggerEntry!=listOfTriggers.end(); ++iTriggerEntry )
	{
		std::cout << std::left << std::setw(25) << iTriggerEntry->name << iTriggerEntry->version << std::endl;
	}
	std::cout << "------- End of triggers -------" << std::endl;
}
