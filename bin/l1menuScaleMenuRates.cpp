#include <stdexcept>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "l1menu/tools/CommandLineParser.h"
#include "l1menu/tools/fileIO.h"
#include "l1menu/IMenuRate.h"
#include "l1menu/TriggerMenu.h"
#include "l1menu/MenuRatePlots.h"
#include "l1menu/IL1MenuFile.h"
#include "l1menu/scalings/MCDataScaling.h"
#include "l1menu/scalings/MuonScaling.h"
#include "l1menu/scalings/OnlineToOfflineScaling.h"
#include <TFile.h>

namespace // Unnamed namespace for things only used in this file
{

} // end of the unnamed namespace

void printUsage( const std::string& executableName, std::ostream& output=std::cout )
{
	output << "Usage:" << "\n"
			<< "\t" << executableName << "  [--rateplots <unscaled rateplot filename>] [--muonscaling <muon scaling filename>]" << "\n"
			<< "\t" << std::setw(executableName.size()) << " " << "    [--datascaling <data scaling filename> --montecarloscaling <MC scaling filename>]" << "\n"
			<< "\t" << std::setw(executableName.size()) << " " << "    [--format <CSV | OLD | XML>] [--offlinescaling <offline scaling file>]" << "\n"
			<< "\t" << std::setw(executableName.size()) << " " << "    <menu rate to scale 1> [menu rate to scale 2] [menu rate to scale 3]..." << "\n"
			<< "\t" << "\t" << "Scales the provided menu rates with the scaling options provided. The menu rates should be in XML" << "\n"
			<< "\t" << "\t" << "format, i.e. the output from l1menuFitMenu or l1menuCalculateRate with the '--format XML' option." << "\n"
			<< "\t" << "\t" << "Most of the scalings require 'rateplots' to be set, which should be the unscaled output from the " << "\n"
			<< "\t" << "\t" << "l1menuCreateRatePlots program. If 'datascaling' is set then 'montecarloscaling' must also be set," << "\n"
			<< "\t" << "\t" << "since the scaling performed is the ratio of the two." << "\n"
			<< "\n"
			<< "\t" << executableName << " --help" << "\n"
			<< "\t" << "\t" << "prints this help message" << "\n"
			<< std::endl;
}

int main( int argc, char* argv[] )
{
	std::vector<std::string> menuRates;
	std::string muonScalingFilename; // The filename of the file used to scale muon rates. Optional and can be empty.
	std::string offlineScalingFilename; // The filename of the file used to scale thresholds from online to offline.
	std::vector< std::unique_ptr<l1menu::IScaling> > scalingsToApply; // List of all the scalings that should be applied
	std::string unscaledRatesFilename;
	l1menu::IL1MenuFile::FileFormat fileFormat=l1menu::IL1MenuFile::FileFormat::XML;


	l1menu::tools::CommandLineParser commandLineParser;
	try
	{
		commandLineParser.addOption( "help", l1menu::tools::CommandLineParser::NoArgument );
		commandLineParser.addOption( "muonscaling", l1menu::tools::CommandLineParser::RequiredArgument );
		commandLineParser.addOption( "rateplots", l1menu::tools::CommandLineParser::RequiredArgument );
		commandLineParser.addOption( "offlinescaling", l1menu::tools::CommandLineParser::RequiredArgument );
		commandLineParser.addOption( "montecarloscaling", l1menu::tools::CommandLineParser::RequiredArgument );
		commandLineParser.addOption( "datascaling", l1menu::tools::CommandLineParser::RequiredArgument );
		commandLineParser.addOption( "format", l1menu::tools::CommandLineParser::RequiredArgument );
		commandLineParser.parse( argc, argv );

		if( commandLineParser.optionHasBeenSet( "help" ) )
		{
			printUsage( commandLineParser.executableName() );
			return 0;
		}

		if( commandLineParser.nonOptionArguments().empty() ) throw std::runtime_error( "You need to specify a filename for at least one MenuRate" );
		if( commandLineParser.optionHasBeenSet( "format" ) )
		{
			std::string formatString=commandLineParser.optionArguments("format").back();
			if( formatString=="XML" ) fileFormat=l1menu::IL1MenuFile::FileFormat::XML;
			else if( formatString=="OLD" ) fileFormat=l1menu::IL1MenuFile::FileFormat::OLD;
			else if( formatString=="CSV" ) fileFormat=l1menu::IL1MenuFile::FileFormat::CSV;
			else throw std::runtime_error( "format must be one of 'XML', 'OLD', or 'CSV'" );
		}
		if( commandLineParser.optionHasBeenSet("rateplots") ) unscaledRatesFilename=commandLineParser.optionArguments("rateplots").back();
		if( commandLineParser.optionHasBeenSet("montecarloscaling") || commandLineParser.optionHasBeenSet("datascaling") )
		{
			if( !commandLineParser.optionHasBeenSet("montecarloscaling") ) throw std::runtime_error( "If the 'datascaling' option is set then 'montecarloscaling' must also be set.");
			else if( !commandLineParser.optionHasBeenSet("datascaling") ) throw std::runtime_error( "If the 'montecarloscaling' option is set then 'datascaling' must also be set.");
			else
			{
				if( unscaledRatesFilename.empty() ) throw std::runtime_error( "Scaling for data/Monte Carlo also requires the unscaled rate plots set with the 'rateplots' option" );
				std::string monteCarloFilename=commandLineParser.optionArguments("montecarloscaling").back();
				std::string dataFilename=commandLineParser.optionArguments("datascaling").back();
				scalingsToApply.push_back( std::unique_ptr<l1menu::IScaling>( new l1menu::scalings::MCDataScaling(monteCarloFilename,dataFilename,unscaledRatesFilename) ) );
			}
		}
		if( commandLineParser.optionHasBeenSet("muonscaling") )
		{
			if( unscaledRatesFilename.empty() ) throw std::runtime_error( "Scaling for muons also requires the unscaled rate plots set with the 'rateplots' option" );
			muonScalingFilename=commandLineParser.optionArguments("muonscaling").back();
			scalingsToApply.push_back( std::unique_ptr<l1menu::IScaling>( new l1menu::scalings::MuonScaling(muonScalingFilename,unscaledRatesFilename) ) );
		}
		if( commandLineParser.optionHasBeenSet("offlinescaling") )
		{
			scalingsToApply.push_back( std::unique_ptr<l1menu::IScaling>( new l1menu::scalings::OnlineToOfflineScaling(commandLineParser.optionArguments("offlinescaling").back()) ) );
		}

		if( scalingsToApply.empty() ) std::cerr << "No scalings have been requested on the command line. Is this really what you want?"
				<< " You can do this to convert menu rates between the different file formats." << std::endl;
		menuRates=commandLineParser.nonOptionArguments();
	} // end of try block
	catch( std::exception& error )
	{
		std::cerr << "Error parsing the command line: " << error.what() << std::endl;
		printUsage( commandLineParser.executableName(), std::cerr );
		return -1;
	}


	try
	{
		//
		// First scale all of the rate plots.
		//
		if( !unscaledRatesFilename.empty() )
		{
			std::cerr << "Scaling " << unscaledRatesFilename << " with..." << std::endl;
			std::unique_ptr<TFile> pRatePlotsRootFile( TFile::Open( unscaledRatesFilename.c_str() ) );
			std::unique_ptr<l1menu::MenuRatePlots> pRatePlots( new l1menu::MenuRatePlots( pRatePlotsRootFile.get() ) );

			for( const auto& pScaling : scalingsToApply )
			{
				std::cerr << "   " << pScaling->briefDescription() << std::endl;
				// Replace the current menu with the newly scaled one, and repeat until all scaling has been done.
				pRatePlots=pScaling->scale( *pRatePlots );
			}

			std::unique_ptr<TFile> pOutputScaledRatePlotsFile( TFile::Open( "scaledRatePlots.root", "RECREATE" ) );
			pRatePlots->setDirectory( pOutputScaledRatePlotsFile.get() );
			pRatePlots->relinquishOwnershipOfPlots();
			pOutputScaledRatePlotsFile->Write();
			std::cerr << "Scaled rate plots written to file " << "scaledRatePlots.root" << std::endl;
		}


		std::unique_ptr<l1menu::IL1MenuFile> pOutputFile=l1menu::IL1MenuFile::getOutputFile( fileFormat, std::cout );

		for( const auto& menuRateFilename : menuRates )
		{
			try
			{
				std::cerr << "Loading menu rates from file " << menuRateFilename << std::endl;
				std::unique_ptr<l1menu::IL1MenuFile> pInputFile=l1menu::IL1MenuFile::getInputFile( menuRateFilename );
				std::vector< std::unique_ptr<l1menu::IMenuRate> > menuRates=pInputFile->getRates();


				for( auto& pMenuRate : menuRates )
				{
					std::cerr << "Scaling rate from file " << menuRateFilename << " with..." << std::endl;
					for( const auto& pScaling : scalingsToApply )
					{
						std::cerr << "   " << pScaling->briefDescription() << std::endl;
						// Replace the current menu with the newly scaled one, and repeat until all scaling has been done.
						pMenuRate=pScaling->scale( *pMenuRate );
					}


					pOutputFile->add( *pMenuRate );
				}
			}
			catch( std::exception& error )
			{
				std::cerr << "Couldn't process file \"" << menuRateFilename << "\" because: " << error.what() << std::endl;
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
