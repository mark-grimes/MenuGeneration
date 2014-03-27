#ifndef l1menu_tools_tools_h
#define l1menu_tools_tools_h

/** @file
 * Miscellaneous functions that don't fit into another category.
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
	class L1TriggerDPGEvent;
}


namespace l1menu
{
	namespace tools
	{
		/** @brief Finds all of the parameter names that refer to thresholds.
		 *
		 * Searches through all the parameter names for things that have the form "threshold1",
		 * "threshold2" etcetera. Also looks for things of the form "leg1threshold1", "leg2threshold1"
		 * etcetera for when I get around to implementing the cross triggers.
		 *
		 * @param[in] trigger    The trigger to check.
		 * @return               A std::vector of strings for all of the value parameter names that
		 *                       refer to thresholds.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 28/May/2013
		 */
		std::vector<std::string> getThresholdNames( const l1menu::ITriggerDescription& trigger );

		/** @brief Finds all of the parameter names that don't refer to thresholds.
		 *
		 * Does the opposite of getThresholdNames, so returns all parameter names that getThresholdNames
		 * doesn't.
		 *
		 * @param[in] trigger    The trigger to check.
		 * @return               A std::vector of strings for all of the valued parameter names that
		 *                       don't refer to thresholds.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 30/May/2013
		 */
		std::vector<std::string> getNonThresholdParameterNames( const l1menu::ITriggerDescription& trigger );

		/** @brief Sets all of the thresholds in the supplied trigger as tight as possible but still passing the supplied event.
		 *
		 * Note that this assumes all of the thresholds are independent. If they're not, behaviour is undefined. The supplied
		 * trigger is modified, so make a copy before hand if that's not what you want. Note only the thresholds are changed, any
		 * other parameters e.g. eta cuts are kept as is.
		 *
		 * If no thresholds can be found that would let the trigger pass the supplied event, a std::runtime_error is thrown.
		 *
		 * @param[in]  event      The event to test the trigger on.
		 * @param[out] trigger    The trigger to check and modify.
		 * @param[in]  tolerance  The trigger thresholds will be modified to be within this tolerance of thresholds that would
		 *                        fail the event.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 28/May/2013
		 */
		void setTriggerThresholdsAsTightAsPossible( const l1menu::L1TriggerDPGEvent& event, l1menu::ITrigger& trigger, float tolerance=0.01 );

		/** @brief Gives the eta bounds of the requested calorimeter region.
		 *
		 * @param[in]  calorimeterRegion   The calorimeter region. Must be between 0 and 21 inclusive or a
		 *                                 std::runtime_error is thrown.
		 * @return                         A std::pair where 'first' is the lower eta bound and 'second' is the higher. Note that
		 *                                 the values are not absolute and first is always less than second. Values also overlap,
		 *                                 e.g. calorimeterRegionEtaBounds(6).second==calorimeterRegionEtaBounds(7).first.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 10/Jun/2013
		 */
		std::pair<float,float> calorimeterRegionEtaBounds( size_t calorimeterRegion );

		/** @brief Converts a value in absolute eta to the calorimeter region. */
		float convertEtaCutToRegionCut( float etaCut );

		/** @brief Converts a value in calorimeter region to absolute eta. */
		float convertRegionCutToEtaCut( float regionCut );

		/** @brief Sets the binning for trigger plots to match the binning that was used in the old code.
		 *
		 * Calls TriggerTable::registerSuggestedBinning for each trigger with the values hard coded in the old L1Menu2015.C
		 * file. Only the main (i.e. first) threshold is changed. In the old code all the other thresholds were scaled off
		 * the main threshold.
		 *
		 * @post Any later calls to TriggerTable::getSuggestedNumberOfBins, TriggerTable::getSuggestedLowerEdge and
		 *       TriggerTable::getSuggestedUpperEdge for a given trigger will return the values hard coded in the
		 *       old L1Menu2015.C file.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 28/Aug/2013
		 */
		void setBinningToL1Menu2015Values();

		/** @brief Takes a number of (x,y) points and calculates the line of best fit.
		 *
		 * I couldn't be bothered working this out for myself so just copied the formulae
		 * from http://en.wikipedia.org/wiki/Simple_linear_regression
		 *
		 * @param[in]  dataPoints     A vector of pairs, where 'first' is the x coordinate and 'second' the y.
		 * @return                    A pair where 'first' is the slope and 'second' the intercept of the line
		 *                            of best fit.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk) although I can't really take credit for it.
		 * @date 08/Jul/2013
		 */
		std::pair<double,double> simpleLinearFit( const std::vector< std::pair<double,double> >& dataPoints );
	} // end of the tools namespace
} // end of the l1menu namespace
#endif
