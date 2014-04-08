#ifndef l1menu_IL1MenuFile_h
#define l1menu_IL1MenuFile_h

#include <string>
#include <memory>
#include <iosfwd>
#include <vector>


namespace l1menu
{
	class TriggerMenu;
	class IMenuRate;
}

namespace l1menu
{
	/** @brief Interface for the different implementations of the file formats.
	 *
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 18/Nov/2013
	 */
	class IL1MenuFile
	{
	public:
		enum class FileFormat : char { XML, OLD, CSV };

	public:
		static std::unique_ptr<l1menu::IL1MenuFile> getOutputFile( FileFormat fileFormat, std::ostream& outputStream );
		static std::unique_ptr<l1menu::IL1MenuFile> getOutputFile( FileFormat fileFormat, const std::string& filename );

		/** @brief Get an instance populated with some previously saved information.
		 *
		 * I wanted this to take a std::istream but I'd have to change a lot of other things
		 * to get that to work.
		 */
		static std::unique_ptr<l1menu::IL1MenuFile> getInputFile( FileFormat fileFormat, const std::string& inputFilename );

	public:
		virtual ~IL1MenuFile() {}

		virtual void add( const l1menu::TriggerMenu& menu ) = 0;
		virtual void add( const l1menu::IMenuRate& menuRate ) = 0;
		virtual std::vector< std::unique_ptr<l1menu::TriggerMenu> > getMenus() const = 0;
		virtual std::vector< std::unique_ptr<l1menu::IMenuRate> > getRates() const = 0;
	};

} // end of namespace l1menu

#endif
