#ifndef l1menu_implementation_XMLL1MenuFile_h
#define l1menu_implementation_XMLL1MenuFile_h

#include "l1menu/IL1MenuFile.h"
#include "l1menu/tools/XMLElement.h"
#include "l1menu/tools/XMLFile.h"
#include <string>

//
// Forward declarations
//
namespace l1menu
{
	class ITriggerRate;
	class ITriggerDescription;
	class ITriggerDescriptionWithErrors;
}


namespace l1menu
{
	namespace implementation
	{
		/** @brief Implementation of the IL1MenuFile interface for XML files.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 21/Nov/2013
		 */
		class XMLL1MenuFile : public l1menu::IL1MenuFile
		{
		public:
			XMLL1MenuFile( std::ostream& outputStream );
			XMLL1MenuFile( const std::string& filename, bool outputOnly );
			virtual ~XMLL1MenuFile();
			virtual void add( const l1menu::TriggerMenu& menu );
			virtual void add( const l1menu::IMenuRate& menuRate );
			virtual std::vector< std::unique_ptr<l1menu::TriggerMenu> > getMenus() const;
			virtual std::vector< std::unique_ptr<l1menu::IMenuRate> > getRates() const;
		protected:
			l1menu::tools::XMLElement convertToXML( const l1menu::TriggerMenu& object, l1menu::tools::XMLElement& parent );
			l1menu::tools::XMLElement convertToXML( const l1menu::IMenuRate& object, l1menu::tools::XMLElement& parent );
			l1menu::tools::XMLElement convertToXML( const l1menu::ITriggerRate& object, l1menu::tools::XMLElement& parent );
			l1menu::tools::XMLElement convertToXML( const l1menu::ITriggerDescription& object, l1menu::tools::XMLElement& parent );
			l1menu::tools::XMLElement convertToXML( const l1menu::ITriggerDescriptionWithErrors& object, l1menu::tools::XMLElement& parent );
			l1menu::tools::XMLFile outputFile_;
			std::ostream* pOutputStream_;
			std::string filenameForOutput_;
		};


	} // end of the implementation namespace
} // end of the l1menu namespace
#endif
