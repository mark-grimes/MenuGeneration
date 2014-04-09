#ifndef l1menu_TriggerConstraint_h
#define l1menu_TriggerConstraint_h


namespace l1menu
{
	/** @brief Description of any constraints that are placed on a trigger when the menu is scaled to fit a particular rate.
	 *
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 08/Apr/2014
	 */
	class TriggerConstraint
	{
	public:
		virtual ~TriggerConstraint() {}
	};

} // end of namespace l1menu

#endif
