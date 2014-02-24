#ifndef l1menu_tools_fileIO_h
#define l1menu_tools_fileIO_h

/** @file
 * Functions for saving and loading from files, but also formatting for output to the
 * standard output.
 */

#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <iosfwd>

//
// Forward declarations
//
namespace l1menu
{
	class ITrigger;
	class ITriggerDescription;
	class IMenuRate;
	class ISample;
	class TriggerMenu;
	class IMenuRate;
	class ITriggerRate;
	namespace tools
	{
		class XMLElement;
	}
}


namespace l1menu
{
	namespace tools
	{
		/** @brief An enum to describe filetypes used to save the objects.
		 *
		 */
		enum class FileFormat : char { XMLFORMAT, OLDFORMAT, CSVFORMAT };


		/** @brief Prints out the trigger rates to the given ostream.
		 *
		 * @param[out] output       The stream to dump the information to.
		 * @param[in]  menuRates    The object containing the information to be dumped.
		 * @param[in]  format       The file format to dump in.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 05/Jul/2013
		 */
		void dumpTriggerRates( std::ostream& output, const l1menu::IMenuRate& menuRates, l1menu::tools::FileFormat format=l1menu::tools::FileFormat::OLDFORMAT );

		/** @brief Prints out the trigger rates to the given ostream.
		 *
		 * @param[out] output             The stream to dump the information to.
		 * @param[in]  menuRates          The object containing the information to be dumped.
		 * @param[in]  offlineThresholds  The menu rates with the thresholds converted to offline thresholds.
		 * @param[in]  format             The file format to dump in.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 05/Jul/2013
		 */
		void dumpTriggerRates( std::ostream& output, const l1menu::IMenuRate& menuRates, const l1menu::IMenuRate& offlineThresholds, l1menu::tools::FileFormat format=l1menu::tools::FileFormat::OLDFORMAT );

		/** @brief Prints out the trigger menu in the same format as the old L1Menu2015 to the given ostream
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 28/Aug/2013
		 */
		void dumpTriggerMenu( std::ostream& output, const l1menu::TriggerMenu& menu, l1menu::tools::FileFormat format=l1menu::tools::FileFormat::OLDFORMAT  );

		/** @brief Examines the file and creates the appropriate concrete implementation of ISample for it.
		 *
		 * Currently only works for ReducedSample, which makes this function a bit pointless. I'll add
		 * support for FullSample soon.
		 *
		 * @param[in]  filename     The filename of the file to open. If the file doesn't exist a std::runtime_error
		 *                          is thrown.
		 * @return                  A pointer to the ISample created.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 07/Jul/2013
		 */
		std::unique_ptr<l1menu::ISample> loadSample( const std::string& filename );

		/** @brief Loads the menu from a file on disk.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 15/Oct/2013
		 */
		std::unique_ptr<l1menu::TriggerMenu> loadMenu( const std::string& filename );

		/** @brief Loads an IMenuRate from a file on disk.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 16/Oct/2013
		 */
		std::unique_ptr<l1menu::IMenuRate> loadRate( const std::string& filename );

		/** @brief Adds a child to the element passed which describes the TriggerMenu.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 14/Oct/2013
		 */
		l1menu::tools::XMLElement convertToXML( const l1menu::TriggerMenu& object, l1menu::tools::XMLElement& parent );

		/** @brief Adds a child to the element passed which describes the ITriggerRate.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 14/Oct/2013
		 */
		l1menu::tools::XMLElement convertToXML( const l1menu::ITriggerRate& object, l1menu::tools::XMLElement& parent );

		l1menu::tools::XMLElement convertToXML( const l1menu::ITriggerDescription& object, l1menu::tools::XMLElement& parent );
		l1menu::tools::XMLElement convertToXML( const l1menu::IMenuRate& object, l1menu::tools::XMLElement& parent );

	} // end of the tools namespace
} // end of the l1menu namespace
#endif
