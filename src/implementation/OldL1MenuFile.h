#ifndef l1menu_implementation_OldL1MenuFile_h
#define l1menu_implementation_OldL1MenuFile_h

#include "l1menu/IL1MenuFile.h"
#include <fstream>

namespace l1menu
{
	namespace implementation
	{
		/** @brief Implementation of the IL1MenuFile interface for the old style files.
		 *
		 * This can also produce comma separated value format files if you supply a comma as
		 * the delimeter in the constructor.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 21/Nov/2013
		 */
		class OldL1MenuFile : public l1menu::IL1MenuFile
		{
		public:
			OldL1MenuFile( std::ostream& outputStream, const char delimeter );
			OldL1MenuFile( const std::string& inputFilename, const char delimeter, bool write );
			virtual ~OldL1MenuFile();
			virtual void add( const l1menu::TriggerMenu& menu );
			virtual void add( const l1menu::IMenuRate& menuRate );
			virtual std::vector< std::unique_ptr<l1menu::TriggerMenu> > getMenus() const;
			virtual std::vector< std::unique_ptr<l1menu::IMenuRate> > getRates() const;
		protected:
			std::ostream* pOutputStream_;
			std::fstream file_;
			std::string filename_;
			const char delimeter_;
		};


	} // end of the implementation namespace
} // end of the l1menu namespace
#endif
