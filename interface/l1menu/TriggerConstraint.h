#ifndef l1menu_TriggerConstraint_h
#define l1menu_TriggerConstraint_h

#include <iosfwd>

namespace l1menu
{
	/** @brief Description of any constraints that are placed on a trigger when the menu is scaled to fit a particular rate.
	 *
	 * The default value for a TriggerConstraint is FIXED_THRESHOLDS, which means the trigger is not modified
	 * at all.
	 *
	 * There are currently 3 possible types of constraint, distinguished using the Type enum.
	 *   - FIXED_THRESHOLDS :      The trigger is used with the thresholds it currently has. I.e.
	 *                             it is never changed. The number returned or set by value() is
	 *                             meaningless and not used.
	 *   - FIXED_RATE :            The thresholds are modified until the rate matches the number
	 *                             given by value(). Units are arbitrary; It's whatever is used when
	 *                             calling MenuFitter. Generally easier if you stick to kHz everywhere
	 *                             though.
	 *   - FRACTION_OF_BANDWIDTH : The thresholds are modified until the rate divided by the total
	 *                             bandwidth equals the number given by value().
	 *
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 08/Apr/2014
	 */
	class TriggerConstraint
	{
	public:
		/** @brief The type of constraint.
		 *
		 * See the documentation for the TriggerConstraint class for full details. */
		enum class Type : char { FIXED_THRESHOLDS, FIXED_RATE, FRACTION_OF_BANDWIDTH };
	public:
		TriggerConstraint();
		virtual ~TriggerConstraint() {}

		Type type() const;
		void type( Type newType );
		float value() const;
		void value( float newValue );

	protected:
		Type type_;
		float value_;
	};

} // end of namespace l1menu

/** @brief Operator overload to allow TriggerConstraint::Type to be output.
 *
 * I doubt this will ever be used, except that the unit test suite requires it when
 * using CPPUNIT_ASSERT_EQUAL.
 */
std::ostream& operator<<( std::ostream& stream, const l1menu::TriggerConstraint::Type& object );


#endif
