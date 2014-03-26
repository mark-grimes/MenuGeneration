#include <cppunit/extensions/HelperMacros.h>


/** @brief A cppunit TestFixture to test the string manipulation functions in the "tools" directory.
 *
 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
 * @date 04/Aug/2013
 */
class StringManipulationUnitTestSuite : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE(StringManipulationUnitTestSuite);
	CPPUNIT_TEST(testSplitByWhitespace);
	CPPUNIT_TEST(testConvertStringToFloat);
	CPPUNIT_TEST_SUITE_END();

protected:

public:
	void setUp();

protected:
	void testSplitByWhitespace();
	void testConvertStringToFloat();
};





#include <cppunit/config/SourcePrefix.h>
#include "l1menu/tools/stringManipulation.h"
#include <stdexcept>
#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION(StringManipulationUnitTestSuite);

void StringManipulationUnitTestSuite::setUp()
{
	// No operation
}

void StringManipulationUnitTestSuite::testSplitByWhitespace()
{
	std::vector<std::string> words;
	words.push_back("Random");
	words.push_back("words");
	words.push_back("for");
	words.push_back("testing");
	words.push_back("this");
	words.push_back("function.");

	std::string sentence=words[0]+" "+words[1]+"\r\n"+words[2]+"   "+words[3]+"\n"+words[4]+"\t\t "+words[5];

	std::vector<std::string> result=l1menu::tools::splitByWhitespace( sentence );
	CPPUNIT_ASSERT_EQUAL( words.size(), result.size() );

	for( std::vector<std::string>::const_iterator iExpected=words.begin(), iActual=result.begin();
			iExpected!=words.end() && iActual!=result.end();
			++iExpected, ++iActual )
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE( "Strings \""+*iExpected+"\" and \""+*iActual+"\" do not match", *iExpected, *iActual );
	}
}

void StringManipulationUnitTestSuite::testConvertStringToFloat()
{
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 9, l1menu::tools::convertStringToFloat("9"), 0 );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( 34.039298, l1menu::tools::convertStringToFloat("34.039298"), std::pow(10,-5) );
	CPPUNIT_ASSERT_DOUBLES_EQUAL( -123.12131, l1menu::tools::convertStringToFloat("-123.12131"), std::pow(10,-5) );
	CPPUNIT_ASSERT_THROW( l1menu::tools::convertStringToFloat("To the pub!"), std::runtime_error );
	CPPUNIT_ASSERT_THROW( l1menu::tools::convertStringToFloat("12 blah"), std::runtime_error );
}
