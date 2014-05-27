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

// Need this for the definition of l1menu::IL1MenuFile::FileFormat
#include "l1menu/IL1MenuFile.h"
//
// Forward declarations
//
namespace l1menu
{
	class IMenuRate;
	class ISample;
	class TriggerMenu;
}


namespace l1menu
{
	namespace tools
	{
		/** @brief Prints out the trigger rates to the given ostream.
		 *
		 * @param[out] output       The stream to dump the information to.
		 * @param[in]  menuRates    The object containing the information to be dumped.
		 * @param[in]  format       The file format to dump in.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 05/Jul/2013
		 */
		void dumpTriggerRates( std::ostream& output, const l1menu::IMenuRate& menuRates, l1menu::IL1MenuFile::FileFormat format=l1menu::IL1MenuFile::FileFormat::OLD );

		/** @brief Prints out the trigger menu in the same format as the old L1Menu2015 to the given ostream
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 28/Aug/2013
		 */
		void dumpTriggerMenu( std::ostream& output, const l1menu::TriggerMenu& menu, l1menu::IL1MenuFile::FileFormat format=l1menu::IL1MenuFile::FileFormat::OLD  );

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
		 * Note that some file formats allow you to have more than one menu in the file. In this case
		 * this function will silently just return the first one. If you want access to the others
		 * go through l1menu::IL1MenuFile::getMenus() instead. This convenience function just delegates
		 * to that anyway.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 15/Oct/2013
		 */
		std::unique_ptr<l1menu::TriggerMenu> loadMenu( const std::string& filename );

	} // end of the tools namespace
} // end of the l1menu namespace
#endif
