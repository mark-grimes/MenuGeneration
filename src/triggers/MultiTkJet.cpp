#include "MultiTkJet.h"

#include <cmath>
#include <stdexcept>
#include "../implementation/RegisterTriggerMacro.h"
#include "l1menu/L1TriggerDPGEvent.h"
#include "UserCode/L1TriggerUpgrade/interface/L1AnalysisDataFormat.h"


namespace l1menu
{
	namespace triggers
	{

		/* The REGISTER_TRIGGER macro will make sure that the given trigger is registered in the
		 * l1menu::TriggerTable when the program starts. I also want to provide some suggested binning
		 * however. The REGISTER_TRIGGER_AND_CUSTOMISE macro does exactly the same but lets me pass
		 * a pointer to a function that will be called directly after the trigger has been registered
		 * at program startup. The function takes no parameters and returns void. In this case I'm
		 * giving it a lambda function.
		 */
		
		REGISTER_TRIGGER_AND_CUSTOMISE( MultiTkJet_v1,
			[]() // Use a lambda function to customise rather than creating a named function that never gets used again.
			{
				l1menu::TriggerTable& triggerTable=l1menu::TriggerTable::instance();
				MultiTkJet_v1 tempTriggerInstance;
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "threshold1", 100, 0, 100 );
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "threshold2", 100, 0, 100 );
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "threshold3", 100, 0, 100 );
				triggerTable.registerSuggestedBinning( tempTriggerInstance.name(), "threshold4", 100, 0, 100 );
			} // End of customisation lambda function
		) // End of REGISTER_TRIGGER_AND_CUSTOMISE macro call

		REGISTER_TRIGGER( MultiTkJet_v0 )


	} // end of namespace triggers

} // end of namespace l1menu



bool l1menu::triggers::MultiTkJet_v0::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw = PhysicsBits[0];   // ZeroBias
	if (! raw) return false;

	int n1=0;
	int n2=0;
	int n3=0;
	int n4=0;

	int Nj = analysisDataFormat.NTkjet ;
	for (int ue=0; ue < Nj; ue++) {
		int bx = analysisDataFormat.BxTkjet[ue];
		if (bx != 0) continue;
//		bool isFwdJet = analysisDataFormat.Fwdjet[ue];
//		if (isFwdJet) continue;
//		bool isTauJet = analysisDataFormat.Taujet[ue];
//		if (isTauJet) continue;

		float eta = analysisDataFormat.EtaTkjet[ue];
		if (eta < regionCut_ || eta > 21.-regionCut_) continue;

		float rank = analysisDataFormat.EtTkjet[ue];
		float pt = rank; //CorrectedL1JetPtByGCTregions(analysisDataFormat.Etajet[ue],rank*4.,theL1JetCorrection);
		if (pt >= threshold1_) n1++;
		if (pt >= threshold2_) n2++;
		if (pt >= threshold3_) n3++;
		if (pt >= threshold4_) n4++;
	}

	bool ok = ( n1 >=1 && n2 >= 2 && n3 >= 3 && n4 >= numberOfJets_);
	return ok;
}

bool l1menu::triggers::MultiTkJet_v0::thresholdsAreCorrelated() const
{
	return false;
}

unsigned int l1menu::triggers::MultiTkJet_v0::version() const
{
	return 0;
}


bool l1menu::triggers::MultiTkJet_v1::apply( const l1menu::L1TriggerDPGEvent& event ) const
{
	const L1Analysis::L1AnalysisDataFormat& analysisDataFormat=event.rawEvent();
	const bool* PhysicsBits=event.physicsBits();

	bool raw = PhysicsBits[0];   // ZeroBias
	if (! raw) return false;

        bool ok = false;

	int Nj = analysisDataFormat.NTkjet ;
	int ue = 0;
	while (ue < Nj && !ok) {

        	int n1=0;
        	int n2=0;
        	int n3=0;
        	int n4=0;

		int bx = analysisDataFormat.BxTkjet[ue];
		if (bx != 0) {ue++; continue;}  //ugh...probably should clean this up

		float eta = analysisDataFormat.EtaTkjet[ue];
		if (eta < regionCut_ || eta > 21.-regionCut_) {ue++; continue;}

		float pt = analysisDataFormat.EtTkjet[ue];		
		if (pt >= threshold1_) {
		    n1++;
		    float jet1Zvtx = analysisDataFormat.zVtxTkjet[ue];
		    for(int uj=0; uj<Nj; uj++) {
		      if(fabs(jet1Zvtx - analysisDataFormat.zVtxTkjet[uj]) < zVtxCut_)  {
		        float pt2 = analysisDataFormat.EtTkjet[uj];
			float eta2 =  analysisDataFormat.EtaTkjet[uj];
			if (eta2 < regionCut_ || eta2 > 21.-regionCut_) continue;
		        if (pt2 >= threshold2_) n2++;
		        if (pt2 >= threshold3_) n3++;
		        if (pt2 >= threshold4_) n4++;
		      } // same vertex as primary
		    } // loop over others
		    ok = ( n1 >=1 && n2 >= 2 && n3 >= 3 && n4 >= numberOfJets_);
		    //printf("Tested ue %i Trigger %i \n",ue,ok);
		}       
	   ue++;
	   //printf("Incrementing ue to %i out of %i...\n",ue,Nj);
	}
        //printf("Done looking for trigger: Value %i \n",ok);
	
	return ok;
}

bool l1menu::triggers::MultiTkJet_v1::thresholdsAreCorrelated() const
{
	return false;
}

unsigned int l1menu::triggers::MultiTkJet_v1::version() const
{
	return 1;
}


l1menu::triggers::MultiTkJet::MultiTkJet()
	: threshold1_(20), threshold2_(20), threshold3_(20), threshold4_(20), regionCut_(4.5), numberOfJets_(6), zVtxCut_(1.0)
{
	// No operation other than the initialiser list
}

const std::string l1menu::triggers::MultiTkJet::name() const
{
	return "L1_MultiTkJet";
}

const std::vector<std::string> l1menu::triggers::MultiTkJet::parameterNames() const
{
	std::vector<std::string> returnValue;
	returnValue.push_back("threshold1");
	returnValue.push_back("threshold2");
	returnValue.push_back("threshold3");
	returnValue.push_back("threshold4");
	returnValue.push_back("regionCut");
	returnValue.push_back("zVtxCut");
	returnValue.push_back("numberOfJets");
	return returnValue;
}

float& l1menu::triggers::MultiTkJet::parameter( const std::string& parameterName )
{
	if( parameterName=="threshold1" ) return threshold1_;
	else if( parameterName=="threshold2" ) return threshold2_;
	else if( parameterName=="threshold3" ) return threshold3_;
	else if( parameterName=="threshold4" ) return threshold4_;
	else if( parameterName=="regionCut" ) return regionCut_;
	else if( parameterName=="numberOfJets" ) return numberOfJets_;
	else if( parameterName=="zVtxCut" ) return zVtxCut_;
	else throw std::logic_error( "Not a valid parameter name" );
}

const float& l1menu::triggers::MultiTkJet::parameter( const std::string& parameterName ) const
{
	if( parameterName=="threshold1" ) return threshold1_;
	else if( parameterName=="threshold2" ) return threshold2_;
	else if( parameterName=="threshold3" ) return threshold3_;
	else if( parameterName=="threshold4" ) return threshold4_;
	else if( parameterName=="regionCut" ) return regionCut_;
	else if( parameterName=="numberOfJets" ) return numberOfJets_;
	else if( parameterName=="zVtxCut" ) return zVtxCut_;
	else throw std::logic_error( "Not a valid parameter name" );
}
