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
		TriggerConstraint();
		virtual ~TriggerConstraint() {}

		bool thresholdsLocked() const;
		void thresholdsLocked( bool thresholdsLocked );
		float fractionOfTotalBandwidth() const;
		void fractionOfTotalBandwidth( float fractionOfTotalBandwidth );
	protected:
		bool thresholdsLocked_;
		float fractionOfTotalBandwidth_;
	};

} // end of namespace l1menu

#endif
