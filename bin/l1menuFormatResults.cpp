#include <string>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <algorithm>

#include "l1menu/IL1MenuFile.h"
#include "l1menu/IMenuRate.h"
#include "l1menu/ITriggerRate.h"
#include "l1menu/ITriggerDescriptionWithErrors.h"
#include "l1menu/tools/CommandLineParser.h"
#include "l1menu/tools/miscellaneous.h"

#include <TFile.h>
#include <TGraphAsymmErrors.h>

void printUsage( const std::string& executableName, std::ostream& output=std::cout )
{
	output << "Usage:" << "\n"
			<< "\t" << executableName << " <input rate filename>" << "\n"
			<< "\n"
			<< "\t" << executableName << " --help" << "\n"
			<< "\t" << "\t" << "prints this help message"
			<< "\n"
			<< std::endl;
}

int main( int argc, char* argv[] )
{
	std::string inputFilename;
	std::string outputFilename;

	l1menu::tools::CommandLineParser commandLineParser;
	try
	{
		commandLineParser.addOption( "help", l1menu::tools::CommandLineParser::NoArgument );
		commandLineParser.addOption( "output", l1menu::tools::CommandLineParser::RequiredArgument );
		commandLineParser.parse( argc, argv );

		if( commandLineParser.optionHasBeenSet( "help" ) )
		{
			printUsage( commandLineParser.executableName() );
			return 0;
		}

		if( commandLineParser.optionHasBeenSet( "output" ) ) outputFilename=commandLineParser.optionArguments("output").back();
		else
		{
			outputFilename="formatResults.root";
			std::cerr << "No output filename was specified using the '--output' argument. Using default of '" << outputFilename << "'." << std::endl;
		}

		if( commandLineParser.nonOptionArguments().size()!=1 ) throw std::runtime_error( "You should only specify one input filename" );
		inputFilename=commandLineParser.nonOptionArguments()[0];
	} // end of try block
	catch( std::exception& error )
	{
		std::cerr << "Error parsing the command line: " << error.what() << std::endl;
		printUsage( commandLineParser.executableName(), std::cerr );
		return -1;
	}

	try
	{
		std::unique_ptr<l1menu::IL1MenuFile> pInputFile=l1menu::IL1MenuFile::getInputFile( l1menu::IL1MenuFile::FileFormat::XML, inputFilename );
		std::vector< std::unique_ptr<l1menu::IMenuRate> > menuRates=pInputFile->getRates();

		// Sort the menu rates by the total rate. Otherwise the plot will look like
		// it's jumping back and forth.
		std::sort( menuRates.begin(), menuRates.end(),
				[]( const std::unique_ptr<l1menu::IMenuRate>& p1, const std::unique_ptr<l1menu::IMenuRate>& p2)->bool
				{
					return p1->totalRate()<p2->totalRate();
				} );

		// This map will have a TGraphAsymmErrors for each trigger
		std::map<std::string,TGraphAsymmErrors*> allGraphs;

		// Open a file to save the histograms. The custom deleter allows it to automatically save
		// if it goes out of scope (i.e. if an exception is thrown).
		std::unique_ptr<TFile,std::function<void(TFile*)> > pMyRootFile( new TFile( outputFilename.c_str(), "RECREATE" ), [](TFile*p){p->Write();p->Close();delete p;} );

		for( size_t index=0; index<menuRates.size(); ++index )
		{
			const auto& pMenuRate=menuRates[index];
			float totalRate=pMenuRate->totalRate();

			for( const auto& pTriggerRate : pMenuRate->triggerRates() )
			{
				auto& pGraph=allGraphs[pTriggerRate->trigger().name()];
				if( pGraph==nullptr )
				{
					// If this is the first time map has been asked for an entry for this trigger
					// then the pointer will be null. I need to create a new TGraphAsymmErrors for
					// the trigger.
					pGraph=new TGraphAsymmErrors;
					pGraph->SetName( pTriggerRate->trigger().name().c_str() );
					// This next line gives ownership of the TGraph to the TFile. That's why I'm
					// not using a smart pointer for pGraph - the TFile will delete it.
					pMyRootFile->Append( pGraph );
				}

				std::string mainThresholdName=l1menu::tools::getThresholdNames( pTriggerRate->trigger() ).front();
				pGraph->SetPoint( pGraph->GetN(), totalRate, pTriggerRate->trigger().parameter(mainThresholdName)  );
				if( pTriggerRate->trigger().parameterErrorsAreAvailable(mainThresholdName) )
				{
					pGraph->SetPointEYhigh( pGraph->GetN()-1, pTriggerRate->trigger().parameterErrorHigh(mainThresholdName) );
					pGraph->SetPointEYlow( pGraph->GetN()-1, pTriggerRate->trigger().parameterErrorLow(mainThresholdName) );
				}
			}

		}

	}
	catch( std::exception& error )
	{
		std::cerr << "Exception caught: " << error.what() << std::endl;
		return -1;
	}

	return 0;
}
