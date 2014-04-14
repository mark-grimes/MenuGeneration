#ifndef l1menu_MenuFitter_h
#define l1menu_MenuFitter_h

#include <memory>

// Forward declarations
namespace l1menu
{
	class ISample;
	class ITrigger;
	class TriggerMenu;
	class IMenuRate;
	class MenuRatePlots;
	class TriggerRatePlot;
	class TriggerConstraint;
}

namespace l1menu
{
	/** @brief Changes the thresholds in a trigger menu to provide a given rate.
	 *
	 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
	 * @date 07/Jul/2013
	 */
	class MenuFitter
	{
	public:
		MenuFitter( const l1menu::ISample& sample, const l1menu::TriggerMenu& menu );
		MenuFitter( const l1menu::ISample& sample, const l1menu::TriggerMenu& menu, const l1menu::MenuRatePlots& menuRatePlots );
		virtual ~MenuFitter();
		const l1menu::TriggerMenu& menu() const;
		std::shared_ptr<const l1menu::IMenuRate> fit( float totalRate, float tolerance );
		const std::string debugLog(); ///< @brief Returns output describing how the most recent fit proceeded.
		void addTrigger( const l1menu::ITrigger& trigger, const TriggerConstraint& constraint );

		// TODO need to tidy these methods. Not very consistent.
		const l1menu::TriggerRatePlot& triggerRatePlot( size_t triggerNumber ) const;
		const l1menu::MenuRatePlots& menuRatePlots() const;
	private:
		std::unique_ptr<class MenuFitterPrivateMembers> pImple_;
	};

}
#endif
