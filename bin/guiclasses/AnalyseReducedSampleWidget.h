#ifndef guiclasses_AnalyseReducedSampleWidget_h
#define guiclasses_AnalyseReducedSampleWidget_h

/*                                     --------------------------
 *                                     ----- **IMPORTANT** ------
 *                                     --------------------------
 *
 * If you modify this file by adding more slots you will need to regenerate the AnalyseReducedSampleWidget.moc file.
 * This can be done with the commands:
 *
 *     cd $CMSSW_BASE/src/L1Trigger/MenuGeneration/bin/guiclasses
 *     moc AnalyseReducedSampleWidget.h > AnalyseReducedSampleWidget.moc
 *
 * "moc" is part of QT. If you have QT installed (which is included with CMSSW so you should have) then
 * moc will be available on the command line.
 *
 */

#include <QtGui>
#include <vector>
#include <memory>

#include "l1menu/TriggerMenu.h"
#include "MenuWidget.h"
//
// Forward declarations
//
namespace l1menu
{
	class ReducedSample;

	namespace guiclasses
	{
		class TriggerWidget;
	}
}

namespace l1menu
{
	namespace guiclasses
	{
		/** @brief Widget that displays all triggers available for a reduced sample, allows selecting, and
		 * then buttons to calculate the rate
		 *
		 * Note that the l1menu::ReducedSample passed in the constructor must exist for
		 * the entire time this object does.
		 *
		 * @author Mark Grimes (mark.grimes@bristol.ac.uk)
		 * @date 25/Jun/2014
		 */
		class AnalyseReducedSampleWidget : public QWidget
		{
			Q_OBJECT
		public:
			AnalyseReducedSampleWidget( l1menu::ReducedSample& sample );
		private slots:
			void calculateRates();
			void saveMenu();
		private:
			l1menu::ReducedSample& sample_;
			l1menu::TriggerMenu menu_;
			l1menu::guiclasses::MenuWidget menuWidget_;
			std::unique_ptr<QDoubleSpinBox> pCollisionRate_;
			std::unique_ptr<QPushButton> pCalculateRateButton_;
		};

	} // end of namespace guiclasses

} // end of namespace l1menu

#endif // of ifndef guiclasses_AnalyseReducedSampleWidget_h
