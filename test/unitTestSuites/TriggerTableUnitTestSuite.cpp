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
	CPPUNIT_TEST(dumpTriggerTable);
	CPPUNIT_TEST_SUITE_END();

protected:
	std::ostream* pVerboseOutput_;
public:
	void setUp();

protected:
	void testGettingAndSettingAllTriggerParameters();
	/** @brief Not really a test as such, just prints out all the triggers for the
	 * user to see what triggers are registered. */
	void dumpTriggerTable();
};





#include <cppunit/config/SourcePrefix.h>
#include "l1menu/TriggerTable.h"
#include "l1menu/ITrigger.h"
#include <stdexcept>
#include <cmath>
#include <iomanip>

CPPUNIT_TEST_SUITE_REGISTRATION(TriggerTableUnitTestSuite);

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
