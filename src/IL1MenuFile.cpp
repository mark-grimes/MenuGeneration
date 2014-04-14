#include "l1menu/IL1MenuFile.h"

#include <stdexcept>
#include "implementation/XMLL1MenuFile.h"
#include "implementation/OldL1MenuFile.h"

/** @file
 *
 * Although IL1MenuFile is an abstract interface there are some static methods to get the
 * concrete instances. These are defined here.
 */

std::unique_ptr<l1menu::IL1MenuFile> l1menu::IL1MenuFile::getOutputFile( l1menu::IL1MenuFile::FileFormat fileFormat, std::ostream& outputStream )
{
	if( fileFormat==l1menu::IL1MenuFile::FileFormat::XML ) return std::unique_ptr<l1menu::IL1MenuFile>( new l1menu::implementation::XMLL1MenuFile(outputStream) );
	if( fileFormat==l1menu::IL1MenuFile::FileFormat::CSV ) return std::unique_ptr<l1menu::IL1MenuFile>( new l1menu::implementation::OldL1MenuFile(outputStream,',') );
	if( fileFormat==l1menu::IL1MenuFile::FileFormat::OLD ) return std::unique_ptr<l1menu::IL1MenuFile>( new l1menu::implementation::OldL1MenuFile(outputStream,' ') );
	else throw std::logic_error( "Unimplemented value for l1menu::IL1MenuFile::FileFormat" );
}

std::unique_ptr<l1menu::IL1MenuFile> l1menu::IL1MenuFile::getOutputFile( FileFormat fileFormat, const std::string& filename )
{
	if( fileFormat==l1menu::IL1MenuFile::FileFormat::XML ) return std::unique_ptr<l1menu::IL1MenuFile>( new l1menu::implementation::XMLL1MenuFile(filename,true) );
	if( fileFormat==l1menu::IL1MenuFile::FileFormat::CSV ) return std::unique_ptr<l1menu::IL1MenuFile>( new l1menu::implementation::OldL1MenuFile(filename,',',true) );
	if( fileFormat==l1menu::IL1MenuFile::FileFormat::OLD ) return std::unique_ptr<l1menu::IL1MenuFile>( new l1menu::implementation::OldL1MenuFile(filename,' ',true) );
	else throw std::logic_error( "Unimplemented value for l1menu::IL1MenuFile::FileFormat" );
}

std::unique_ptr<l1menu::IL1MenuFile> l1menu::IL1MenuFile::getInputFile( const std::string& inputFilename )
{
	std::unique_ptr<l1menu::IL1MenuFile> returnValue;
	//
	// I don't know what file format the file is in, so I'll just try
	// to load it as an XML file. If that fails then I'll load it as
	// a text file.
	//
	try
	{
		returnValue.reset( new l1menu::implementation::XMLL1MenuFile(inputFilename,false) );
	}
	catch( std::exception& error )
	{
		returnValue.reset( new l1menu::implementation::OldL1MenuFile(inputFilename,' ',false) );
	}
	return returnValue;
}
