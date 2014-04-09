/** @file Copied this almost verbatim from the cppunit example cppunit/examples/simple
 *
 * Main function to run any registered test suites. Which test suites and the name
 * of the executable produced depends on whatever is setup in BuildFile.xml. At time
 * of writing the executable will be called "SimGeneral_TrackingAnalysis_unitTests"
 * and the test suites will be any in this directory that have a filename ending
 * in "_UnitTests.cpp".
 *
 * Remember that if you add any more *UnitTests.cpp files to the directory they won't
 * be noticed unless you update the modification time of BuildFile.xml, e.g. with
 * "touch BuildFile.xml".
 *
 * @author copied by Mark Grimes (mark.grimes@bristol.ac.uk)
 * @date 26/Jul/2013
 */

#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include "unitTestSuites/TestParameters.h"
#include "l1menu/tools/CommandLineParser.h"


/** @brief Subclass the parameter store so that I can have methods to modify the parameters.
 *
 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
 * @date 12/Aug/2013
 */
template<class T>
class MutableTestParameters : public TestParameters<T>
{
public:
	static void setParameter( const std::string& parameterName, T parameterValue )
	{
		TestParameters<T>::parameters_[parameterName]=parameterValue;
	}
};

void printUsage( const std::string& executableName, std::ostream& output=std::cout )
{
	output << "Usage:" << "\n"
			<< "\t" << executableName << "\n"
			<< "\t" << "\t" << "runs the unit tests with the hard coded default input filenames." << "\n"
			<< "\n"
			<< "\t" << executableName << " [test input file] [test menu file]" << "\n"
			<< "\t" << "\t" << "runs the unit tests with input files named" << "\n"
			<< "\n"
			<< "\t" << executableName << " --help" << "\n"
			<< "\t" << "\t" << "prints this help message" << "\n"
			<< "\n"
			<< "\t" << executableName << " --list" << "\n"
			<< "\t" << "\t" << "prints a list of all the available unit tests" << "\n"
			<< "\n"
			<< "\t" << executableName << " --test <test name 1> [--test <test name 2>  ...]" << "\n"
			<< "\t" << "\t" << "runs only the specified unit tests. Valid names can be found with the \"--list\" option."
			<< "\n"
			<< std::endl;
}

/** @brief Prints all of the tests that have been registered in the cppunit registry.
 * @parameter output   Where to print the information */
void printAvailableTests( std::ostream& output=std::cout )
{
	output << "You can use any of these names with the \"--test\" option to run a subset of the tests."
			<< " If nothing is set the default is \"All Tests\"." << "\n"
			<< "Available tests:" << std::endl;

	CPPUNIT_NS::Test *pCurrentTest=CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();
	std::vector< std::pair<CPPUNIT_NS::Test*,int> > stack;
	while( pCurrentTest!=nullptr )
	{
		// Add some indentation according to the hierarchy level
		for( size_t index=0; index<stack.size()+1; ++index ) output << "   ";
		// Print the name of this test
		output << pCurrentTest->getName() << std::endl;
		// If the test has any children, set the first one as the element to process
		// on the next loop. Also add this element to the stack so that I can come
		// back to it later and process any other children.
		if( pCurrentTest->getChildTestCount()>0 )
		{
			CPPUNIT_NS::Test *pChildTest=pCurrentTest->getChildTestAt(0);
			// Second parameter is "1" because that's the index of the next child I
			// want to try. A check on whether this is valid is made later.
			stack.push_back( std::make_pair( pCurrentTest, 1 ) );
			pCurrentTest=pChildTest;
		}
		// If this test has no children then I need to find the first element
		// on the stack that has another child element left to process.
		else
		{
			pCurrentTest=nullptr;
			while( !stack.empty() && pCurrentTest==nullptr )
			{
				CPPUNIT_NS::Test *pParentTest=stack.back().first;
				int& nextChildIndex=stack.back().second;
				if( nextChildIndex < pParentTest->getChildTestCount() )
				{
					pCurrentTest=pParentTest->getChildTestAt(nextChildIndex);
					++nextChildIndex;
				}
				else stack.pop_back();
			}
		}
	}
}

/** @brief Parses the command line.
 *
 * @return                         Returns true if everything went okay, false if the program should
 *                                 exit without error e.g. if "help" option was specified the program
 *                                 should exit with no error.
 * @throw   std::runtime_error     If anything goes wrong and the program should exit with an error.
 */
bool handleCommandLine( int argc, char* argv[] )
{
	l1menu::tools::CommandLineParser commandLineParser;
	commandLineParser.addOption( "help", l1menu::tools::CommandLineParser::NoArgument );
	commandLineParser.addOption( "list", l1menu::tools::CommandLineParser::NoArgument );
	commandLineParser.addOption( "test", l1menu::tools::CommandLineParser::RequiredArgument );

	// This vector lists the information I want to get from the command line. The first entry
	// in the pair is the name I'm going to store the value under in the TestParameters singleton.
	// The second is the default value if it hasn't been set on the command line. The position in
	// the vector equates to the position on the command line. The defaults are relative to
	// $CMSSW_BASE.
	std::vector<std::pair<std::string,std::string> > requiredParametersAndDefaults;
	requiredParametersAndDefaults.push_back( std::make_pair("TEST_SAMPLE_FILENAME","src/L1Trigger/MenuGeneration/test/unitTestData/Fallback_NeutrinoGun_PU140-v22.proto") );
	requiredParametersAndDefaults.push_back( std::make_pair("TEST_MENU_FILENAME","src/L1Trigger/MenuGeneration/test/unitTestData/L1Menu_v22m20_std.txt") );
	requiredParametersAndDefaults.push_back( std::make_pair("TEST_RATEPLOT_FILENAME","src/L1Trigger/MenuGeneration/test/unitTestData/output_rates_PU140_v23_trk.root") );
	requiredParametersAndDefaults.push_back( std::make_pair("TEST_XMLMENU_FILENAME","src/L1Trigger/MenuGeneration/test/unitTestData/L1Menu_v22m20_std.xml") );

	try{ commandLineParser.parse( argc, argv ); }
	catch( std::runtime_error& exception )
	{
		// I just want to print the usage here and let the caller handle the exception
		printUsage( commandLineParser.executableName(), std::cerr );
		throw;
	}

	if( commandLineParser.optionHasBeenSet( "help" ) )
	{
		printUsage( commandLineParser.executableName() );
		return false;
	}

	if( commandLineParser.optionHasBeenSet( "list" ) )
	{
		printAvailableTests();
		return false;
	}

	if( commandLineParser.optionHasBeenSet( "test" ) )
	{
		MutableTestParameters< std::vector<std::string> >::setParameter( "Tests to run", commandLineParser.optionArguments("test") );
	}
	// If no tests were specified, use a default of "All Tests"
	else MutableTestParameters< std::vector<std::string> >::setParameter( "Tests to run", std::vector<std::string>(1,"All Tests") );

	if( commandLineParser.nonOptionArguments().size()>requiredParametersAndDefaults.size() )
	{
		printUsage( commandLineParser.executableName(), std::cerr );
		throw std::runtime_error( "Too many command line arguments" );
	}

	//
	// Need to set filenames for an input sample and a menu for the tests to run on. I'll
	// first see if the user specified them on the command line, if not I'll use hard
	// coded defaults.
	//
	for( size_t index=0; index<requiredParametersAndDefaults.size(); ++index )
	{
		const std::string& parameterName=requiredParametersAndDefaults[index].first;
		const std::string& defaultValue=requiredParametersAndDefaults[index].second;

		if( commandLineParser.nonOptionArguments().size()>index ) MutableTestParameters<std::string>::setParameter( parameterName, commandLineParser.nonOptionArguments()[index] );
		else
		{
			std::string filenamePrefix="";
			// If it has been set add $CMSSW_BASE to the start of the default filename
			char* pEnvironmentVariable=std::getenv("CMSSW_BASE");
			if( pEnvironmentVariable!=nullptr ) filenamePrefix=pEnvironmentVariable+std::string("/");
			MutableTestParameters<std::string>::setParameter( parameterName, filenamePrefix+=defaultValue );
		}
	}

	//
	// Loop over the parameters and print to the screen what has been set
	//
	std::cout << "Using the following input files for the tests. These can be changed by specifying them "
			<< "on the command line, in the order they appear here." << std::endl;
	for( size_t index=0; index<requiredParametersAndDefaults.size(); ++index )
	{
		std::cout << "  " << requiredParametersAndDefaults[index].first << "=" << TestParameters<std::string>::instance().getParameter( requiredParametersAndDefaults[index].first ) << "\n";
	}
	// Add another blank line to separate from the output
	std::cout << std::endl;

	return true;
}

int main( int argc, char* argv[] )
{
	try
	{
		// Exit if something on the command line says so (e.g. "help" option should print usage and exit).
		if( !handleCommandLine( argc, argv ) ) return 0;
	}
	catch( std::runtime_error& exception )
	{
		std::cerr << "Error parsing the command line: " << exception.what() << std::endl;
		return -1;
	}

	// Create the event manager and test controller
	CPPUNIT_NS::TestResult controller;

	// Add a listener that collects test result
	CPPUNIT_NS::TestResultCollector result;
	controller.addListener( &result );

	// Add a listener that print dots as test run.
	CPPUNIT_NS::BriefTestProgressListener progress;
	controller.addListener( &progress );

	// Create a TestRunner and add the tests to it
	CPPUNIT_NS::TestRunner runner;
	// Get the main parent test and query it to get the tests that the user has asked for.
	// The test names to run will have been set in the "Tests to run" parameter during
	// handleCommandLine(). If the user has not specified anything then this vector will
	// only contain "All Tests".
	CPPUNIT_NS::Test *pMainTest=CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();
	for( const auto& testName : TestParameters< std::vector<std::string> >::instance().getParameter("Tests to run") )
	{
		try
		{
			runner.addTest( pMainTest->findTest( testName ) );
			//std::cout << "Added \"" << testName << "\" to the list of tests to run." << std::endl;
		}
		catch( std::invalid_argument& error )
		{
			std::cerr << "\"" << testName << "\" appears to be an invalid name for a test. Use the \"--list\" option to see available tests." << std::endl;
		}
	}

	//runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
	runner.run( controller );

	// Print test in a compiler compatible format.
	CPPUNIT_NS::CompilerOutputter outputter( &result, CPPUNIT_NS::stdCOut() );
	outputter.write();

	return result.wasSuccessful() ? 0 : 1;
}
