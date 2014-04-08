#ifndef l1menu_tools_XMLFile_h
#define l1menu_tools_XMLFile_h

#include <string>
#include <memory>
#include <iosfwd>
#include <vector>

//
// Forward declarations
//
namespace l1menu
{
	namespace tools
	{
		class XMLElement;
	} // end of namespace tools
} // end of namespace l1menu


namespace l1menu
{
	namespace tools
	{
		/** @brief Thin wrapper to aid in manipulating XML files with xerces
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 02/Oct/2013
		 */
		class XMLFile
		{
		public:
			/** @brief Creates an empty XML document that can be added to and saved to disk. */
			XMLFile();

			/** @brief Attempts to load an XML document from the filename provided. */
			XMLFile( const std::string& filename );

			virtual ~XMLFile();

			void parseFromFile( const std::string& filename );

			l1menu::tools::XMLElement rootElement();
			const l1menu::tools::XMLElement rootElement() const;

			void outputToStream( std::ostream& outputStream );
		private:
			std::unique_ptr<class XMLFilePrivateMembers> pImple;
		};

	} // end of namespace tools
} // end of namespace l1menu

#endif
