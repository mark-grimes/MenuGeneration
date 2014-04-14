#include <string>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "l1menu/IL1MenuFile.h"
#include "l1menu/IMenuRate.h"
#include "l1menu/TriggerMenu.h"
#include "l1menu/tools/CommandLineParser.h"


void printUsage( const std::string& executableName, std::ostream& output=std::cout )
{
	output << "Converts between the different formats (currently text and XML) used for menu files and rate results" << "\n"
			<< "\n"
			<< "Usage:" << "\n"
			<< "\t" << executableName << " [--format <CSV | OLD | XML>] [--output outputFilename] inputFilename" << "\n"
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
	l1menu::IL1MenuFile::FileFormat outputFormat=l1menu::IL1MenuFile::FileFormat::XML;

	l1menu::tools::CommandLineParser commandLineParser;
	try
	{
		commandLineParser.addOption( "help", l1menu::tools::CommandLineParser::NoArgument );
		commandLineParser.addOption( "format", l1menu::tools::CommandLineParser::RequiredArgument );
		commandLineParser.addOption( "output", l1menu::tools::CommandLineParser::RequiredArgument );
		commandLineParser.parse( argc, argv );

		if( commandLineParser.optionHasBeenSet( "help" ) )
		{
			printUsage( commandLineParser.executableName() );
			return 0;
		}

		if( commandLineParser.optionHasBeenSet( "format" ) )
		{
			std::string formatString=commandLineParser.optionArguments("format").back();
			if( formatString=="XML" ) outputFormat=l1menu::IL1MenuFile::FileFormat::XML;
			else if( formatString=="OLD" ) outputFormat=l1menu::IL1MenuFile::FileFormat::OLD;
			else if( formatString=="CSV" ) outputFormat=l1menu::IL1MenuFile::FileFormat::CSV;
			else throw std::runtime_error( "format must be one of 'XML', 'OLD', or 'CSV'" );
		}

		if( commandLineParser.optionHasBeenSet( "output" ) ) outputFilename=commandLineParser.optionArguments("output").back();
		else
		{
			if( outputFormat==l1menu::IL1MenuFile::FileFormat::XML ) outputFilename="convertedFile.xml";
			else if( outputFormat==l1menu::IL1MenuFile::FileFormat::OLD ) outputFilename="convertedFile.txt";
			else if( outputFormat==l1menu::IL1MenuFile::FileFormat::CSV ) outputFilename="convertedFile.csv";
			std::cerr << "No output filename was specified using the '--output' argument. Using default of '" << outputFilename << "'." << std::endl;
		}

		if( commandLineParser.nonOptionArguments().size()!=1 ) throw std::runtime_error( "You should specify one (and only one) input filename" );
		inputFilename=commandLineParser.nonOptionArguments()[0];
	} // end of try block
	catch( std::exception& error )
	{
		std::cerr << "Error parsing the command line: " << error.what() << "\n" << std::endl;
		printUsage( commandLineParser.executableName(), std::cerr );
		return -1;
	}

	try
	{
		std::unique_ptr<l1menu::IL1MenuFile> pInputFile=l1menu::IL1MenuFile::getInputFile( inputFilename );
		if( pInputFile==nullptr ) throw std::runtime_error( "Unable to open the input file '"+inputFilename+"'" );

		std::unique_ptr<l1menu::IL1MenuFile> pOutputFile=l1menu::IL1MenuFile::getOutputFile( outputFormat, outputFilename );
		if( pOutputFile==nullptr ) throw std::runtime_error( "Unable to open the output file '"+outputFilename+"'" );

		//
		// Loop over all of the objects that are in the input file
		// and add them to the output file. If the output format is
		// not XML and there is more than one menu (or a menu and
		// anything else), show a warning and only add the first
		// menu. Only the XML files can hold multiple objects. The
		// other formats can hold multiple rates however, since these
		// have headers and footers to separate them.
		//
		bool atLeastOneMenuAdded=false;
		for( const auto& pMenu : pInputFile->getMenus() )
		{
			if( atLeastOneMenuAdded && outputFormat!=l1menu::IL1MenuFile::FileFormat::XML )
			{
				std::cerr << "Skipping a TriggerMenu because the output file already contains a menu, and the selected format does not support multiple objects." << std::endl;
				continue;
			}
			atLeastOneMenuAdded=true;
			pOutputFile->add( *pMenu );
		}

		try
		{
			for( const auto& pRate : pInputFile->getRates() )
			{
				if( atLeastOneMenuAdded && outputFormat!=l1menu::IL1MenuFile::FileFormat::XML )
				{
					std::cerr << "Skipping a menu rate because the output file already contains a menu, and the selected format does not support multiple objects." << std::endl;
					continue;
				}
				pOutputFile->add( *pRate );
			}
		}
		catch( std::exception& error )
		{
			// Currently the old format can't load IMenuRates. I don't see the need for it so I'll probably
			// never implement it. This checks the error message - not the best way to check what the exception
			// is but my exception types are a bit of a mess at the moment. I'll sort them out soon and only
			// catch this specific exception.
			if( std::string(error.what())=="OldL1MenuFile::getRates() not implemented yet - Loading results from the old format file is not implemented yet. Might never be." )
			{
				// If a menu has already been added, then I don't care that I can't get any IMenuRate objects.
				// The old format can't have TriggerMenus and IMenuRate objects mixed in the same file. If there
				// hasn't, then I should point out the error
				if( !atLeastOneMenuAdded ) throw std::runtime_error( "The input format doesn't support loading rates." );
			}
			else throw; // Some unexpected error, so pass it on.
		}

	}
	catch( std::exception& error )
	{
		std::cerr << "Exception caught: " << error.what() << std::endl;
		return -1;
	}

	return 0;
}
