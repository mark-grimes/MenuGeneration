#ifndef l1menu_TriggerMenu_h
#define l1menu_TriggerMenu_h

#include <memory>
#include <vector>
#include "l1menu/TriggerTable.h"

//
// Forward declarations
//
namespace l1menu
{
	class ITrigger;
	class TriggerConstraint;
	class L1TriggerDPGEvent;
	class MenuFitter;
	class MenuScan;
	class TriggerMenu;
	namespace tools
	{
		class XMLElement;
		std::unique_ptr<l1menu::TriggerMenu> loadMenu( const std::string& filename );
	} // end of namespace tools
} // end of namespace l1menu

namespace l1menu
{
	/** @brief A collection of ITriggers that make up a menu.
	 *
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date sometime around May 2013
	 */
	class TriggerMenu
	{
	public:
		TriggerMenu();
		virtual ~TriggerMenu();
		TriggerMenu( const TriggerMenu& otherTriggerMenu );
		TriggerMenu( TriggerMenu&& otherTriggerMenu ) noexcept;
		TriggerMenu& operator=( const TriggerMenu& otherTriggerMenu );
		TriggerMenu& operator=( TriggerMenu&& otherTriggerMenu ) noexcept;

		ITrigger& addTrigger( const std::string& triggerName );
		ITrigger& addTrigger( const std::string& triggerName, unsigned int version );
		/** @brief Copies the given trigger including all parameters.
		 *
		 * @param[in] triggerToCopy    The trigger to copy.
		 * @return                     A reference to the trigger just added. Note that since it's copied,
		 *                             this is not the same as the reference supplied as input.
		 */
		ITrigger& addTrigger( const ITriggerDescription& triggerToCopy );

		size_t numberOfTriggers() const;
		ITrigger& getTrigger( size_t position );
		const ITrigger& getTrigger( size_t position ) const;

		std::unique_ptr<l1menu::ITrigger> getTriggerCopy( size_t position ) const;

		bool apply( const l1menu::L1TriggerDPGEvent& event ) const;

		/** @brief Get any constraints placed on the particular trigger when the menu is scaled.
		 *
		 * This method is only used when fitting the menu to give a particular rate. It is not
		 * necessary for most uses. It allows you to place restrictions on which thresholds can
		 * be adjusted and what the target rate for the trigger is.
		 *
		 * @param[in] position   The position in the menu of the trigger that the constraint applies
		 *                       to. This should be, in STL notation, [0,numberOfTriggers()) i.e.
		 *                       zero or greater and less than but NOT equal to numberOfTriggers().
		 *                       Calling getTriggerConstraint(n) gets the constraint that applies to
		 *                       the getTrigger(n) trigger.
		 * @return               A reference to the trigger constraint. Note that the object you get
		 *                       back will be empty if there are no constraints on the trigger, which
		 *                       is the default.
		 * @throws               std::out_of_range exception if position is greater than or equal to
		 *                       numberOfTriggers().
		 */
		l1menu::TriggerConstraint& getTriggerConstraint( size_t position );

		/** @brief const version of getTriggerConstraint. See the documentation for that. */
		const l1menu::TriggerConstraint& getTriggerConstraint( size_t position ) const;

	private:
		/** @brief Hide implementation details in a pimple. */
		std::unique_ptr<class TriggerMenuPrivateMembers> pImple_;
	};

}
#endif
