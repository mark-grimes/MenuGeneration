MenuGeneration
==============

L1 trigger menu generation project

To include in a CMSSW build (assuming you have initialised it for git somehow):

    cd $CMSSW_BASE/src
    git clone git@github.com:cms-l1trigger-menustudies/MenuGeneration.git L1Trigger/MenuGeneration
    
This will put the package in L1Trigger/MenuGeneration. You will need to have git setup for github, follow the instructions from the cms-sw page if you haven't done so already.

*Dependencies*

The package requires some L1 Trigger DPG packages that were always kept in UserCode for some reason. As such they haven't been transferred over to git and have to be downloaded with cvs:

    cvs co UserCode/L1TriggerDPG
    cvs co UserCode/L1TriggerUpgrade
    cvs co -d L1Trigger/UCT2015 UserCode/dasu/L1Trigger/UCT2015

*Usage*

There is some documentation in doc/menuGenerationDocumentation.doc which doxygen can make into a webpage. To do that in CMSSW you also need the Documentation/ReferenceManualScripts package in $CMSSW_BASE/src or it won't work, then just execute "scram b doc". The files will be put in $CMSSW_BASE/doc.

*Instructions from scratch*

    scram project CMSSW CMSSW_6_0_1
    cd CMSSW_6_0_1/src
    cmsenv
    # This next command gives an error for me but still seems to work
    git cms-addpkg Documentation/ReferenceManualScripts
    git clone git@github.com:cms-l1trigger-menustudies/MenuGeneration.git L1Trigger/MenuGeneration
    cvs co UserCode/L1TriggerDPG
    cvs co UserCode/L1TriggerUpgrade
    cvs co -d L1Trigger/UCT2015 UserCode/dasu/L1Trigger/UCT2015
    scram b -j 6
    scram b doc
    
This will build the package and create the documentation. Note that L1Trigger/MenuGeneration is not particularly dependant on the CMSSW version. The versions of the L1 DPG pacakages is more important. To view the documentation:

    firefox $CMSSW_BASE/doc/html/L1Trigger_MenuGeneration.html &
