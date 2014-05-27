#include "l1menu/tools/fileIO.h"

#include <exception>
#include <stdexcept>
#include <fstream>
#include "l1menu/TriggerMenu.h"
#include "l1menu/FullSample.h"
#include "l1menu/ReducedSample.h"


void l1menu::tools::dumpTriggerRates( std::ostream& output, const l1menu::IMenuRate& menuRates, l1menu::IL1MenuFile::FileFormat format )
{
	std::unique_ptr<l1menu::IL1MenuFile> pOutputL1MenuFile=l1menu::IL1MenuFile::getOutputFile( format, output );
	pOutputL1MenuFile->add( menuRates );
}

void l1menu::tools::dumpTriggerMenu( std::ostream& output, const l1menu::TriggerMenu& menu, l1menu::IL1MenuFile::FileFormat format )
{
	std::unique_ptr<l1menu::IL1MenuFile> pOutputL1MenuFile=l1menu::IL1MenuFile::getOutputFile( format, output );
	pOutputL1MenuFile->add( menu );
}

std::unique_ptr<l1menu::ISample> l1menu::tools::loadSample( const std::string& filename )
{
	// Open the file, read enough of the start to determine what kind of file
	// it is, then close it.
	std::ifstream inputFile( filename, std::ios_base::binary );
	if( !inputFile.is_open() ) throw std::runtime_error( "The file does not exist or could not be opened" );

	// Look at the first few characters and see if they match some of the file formats
	const size_t bufferSize=20;
	char buffer[bufferSize];
	inputFile.get( buffer, bufferSize );
	inputFile.close();

	if( std::string(buffer)=="l1menuReducedSample" ) return std::unique_ptr<l1menu::ISample>( new l1menu::ReducedSample(filename) );
	else
	{
		// If it's not a ReducedSample then the only other ISample implementation at the
		// moment is a FullSample.
		std::unique_ptr<l1menu::FullSample> pReturnValue( new l1menu::FullSample );

		if( std::string(buffer).substr(0,4)=="root" )
		{
			// File is a root file, so assume it is one of the L1 DPG ntuples and try and load it
			// into the FullSample.
			pReturnValue->loadFile( filename );
			return std::unique_ptr<l1menu::ISample>( pReturnValue.release() );
		}
		else
		{
			// Assume the file is a list of filenames of L1 DPG ntuples.
			// TODO Do some checking to see if the characters I've read so far are valid filepath characters.
			pReturnValue->loadFilesFromList( filename );
			return std::unique_ptr<l1menu::ISample>( pReturnValue.release() );
		}
	}
}

std::unique_ptr<l1menu::TriggerMenu> l1menu::tools::loadMenu( const std::string& filename )
{
	//
	// This function was written before IL1MenuFile was created, but it's still
	// useful as a shorthand.
	//
	std::unique_ptr<l1menu::IL1MenuFile> pFile=l1menu::IL1MenuFile::getInputFile( filename );
	std::vector< std::unique_ptr<l1menu::TriggerMenu> > menusFromFile=pFile->getMenus();
	if( menusFromFile.empty() ) throw std::runtime_error( "l1menu::tools::loadMenu(\""+filename+"\") - Unable to load the menu" );
	return std::move( menusFromFile.front() );
}
