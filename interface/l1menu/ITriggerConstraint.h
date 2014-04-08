#ifndef l1menu_ITriggerConstraint_h
#define l1menu_ITriggerConstraint_h


namespace l1menu
{
	/** @brief Description of any constraints that are placed on a trigger when the menu is scaled to fit a particular rate.
	 *
	 * Developer note - current implementation is in the TriggerMenu.cpp file because that's where
	 * the only instatiation is made.
	 *
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 08/Apr/2014
	 */
	class ITriggerConstraint
	{
	public:
		virtual ~ITriggerConstraint() {}
	};

} // end of namespace l1menu

#endif
