#ifndef l1menu_implementation_MenuRateImplementation_h
#define l1menu_implementation_MenuRateImplementation_h

#include "l1menu/IMenuRate.h"
#include <vector>
#include "TriggerRateImplementation.h"

//
// Forward declarations
//
namespace l1menu
{
	class ITrigger;
	class ITriggerRate;
	class TriggerMenu;
	class ISample;
	class MenuRatePlots;
	namespace tools
	{
		class XMLElement;
	}
}


namespace l1menu
{
	namespace implementation
	{
		/** @brief Implementation of the IMenuRate interface.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 28/Jun/2013
		 */
		class MenuRateImplementation : public l1menu::IMenuRate
		{
		public:
			MenuRateImplementation();
			MenuRateImplementation( const l1menu::TriggerMenu& menu, const l1menu::ISample& sample );
			MenuRateImplementation( const l1menu::TriggerMenu& menu, const l1menu::ISample& sample, const l1menu::MenuRatePlots& menuRatePlots );
			MenuRateImplementation( const l1menu::tools::XMLElement& xmlDescription );

			// Methods to allow modification of the underlying data
			void setTotalFraction( float totalFraction );
			void setTotalFractionError( float totalFractionError );
			void setTotalRate( float totalRate );
			void setTotalRateError( float totalRateError );
			void addTriggerRate( l1menu::implementation::TriggerRateImplementation&& triggerRate );

			// Methods required by the l1menu::IMenuRate interface
			virtual float totalFraction() const;
			virtual float totalFractionError() const;
			virtual float totalRate() const;
			virtual float totalRateError() const;
			virtual const std::vector<const l1menu::ITriggerRate*>& triggerRates() const;
		protected:
			float totalFraction_;
			float totalFractionError_;
			float totalRate_;
			float totalRateError_;
			std::vector<TriggerRateImplementation> triggerRates_;
		private:
			void commonConstruction( const l1menu::TriggerMenu& menu, const l1menu::ISample& sample );
			mutable std::vector<const l1menu::ITriggerRate*> baseClassPointers_; ///< Vector to return for calls to triggerRates()
		};


	} // end of the implementation namespace
} // end of the l1menu namespace
#endif
