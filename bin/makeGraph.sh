#!/bin/sh 

## Script to draw a graph of threshold vs total Bandwidth for a given L1 trigger
##
## To run, you need to pass 2 arguments:
## ./makeGraph.sh inputTable L1trigger_name
##
## Author: georgia ; 12.03.2014
##

if [[ -z "$1" ]] ; then echo "Please insert input file name!"; exit; fi
if [[ -z "$2" ]] ; then echo "Please insert trigger name!"; exit; fi

_input=$1
echo "Drawing graph for Trigger = "$2

_file="test.txt"
rm -rf $_file

_macro="test.C"
rm -rf $_macro

echo "void test() {" >> $_macro

cat $_input | grep "Total L1 Rate (with overlaps)" | awk '{print $7}' > $_file 
#cat $_file

# x Array : bandwidth values
_n=$(wc -l < $_file)
echo "x Array dimension="$_n

_lumiarray=( `cat $_file` )
_xarray=$(IFS=, ; echo "${_lumiarray[*]}")

echo "x="$_xarray 
echo "Double_t x["$_n"]={"$_xarray"};" >> $_macro
rm -rf $_file

#bandwidth error array
cat $_input | grep "Total L1 Rate (with overlaps)" | awk '{print $9}' > $_file 
#cat $_file

_earray=( `cat $_file` )
_xearray=$(IFS=, ; echo "${_earray[*]}")

echo "ex="$_xearray
echo "Double_t ex["$_n"]={"$_xearray"};" >> $_macro


# Trigger name array
_triggerArray=( L1_SingleEG L1_SingleIsoEG L1_SingleMu L1_SingleIsoMu L1_SingleTau L1_SingleIsoTau L1_isoEG_EG L1_isoMu_Mu L1_isoTau_Tau L1_isoEG_Mu L1_isoMu_EG L1_isoEG_Tau L1_isoMu_Tau L1_SingleJetC L1_DoubleJet L1_QuadJetC L1_SingleIsoEG_CJet L1_SingleMu_CJet L1_SingleIsoEG_HTM L1_SingleMu_HTM L1_HTM L1_HTT )

_triggerArray1=( L1_SingleEG L1_SingleIsoEG L1_SingleMu L1_SingleIsoMu L1_SingleTau L1_SingleIsoTau L1_SingleJetC L1_HTM L1_HTT )
_triggerArray2=( L1_isoEG_EG L1_isoMu_Mu L1_isoTau_Tau L1_isoEG_Mu L1_isoMu_EG L1_isoEG_Tau L1_isoMu_Tau L1_DoubleJet L1_SingleIsoEG_CJet L1_SingleMu_CJet L1_SingleIsoEG_HTM L1_SingleMu_HTM )
_triggerArray4=( L1_QuadJetC )

iT=-1; obj=-1

_testit=0

for _trigger in "${_triggerArray1[@]}"
do
  if [ $_trigger == $2 ]; then obj=1; 
      echo "This is a Single object trigger"
      break; fi
done
for _trigger in "${_triggerArray2[@]}"
do
  if [ $_trigger == $2 ]; then obj=2; 
      echo "This is a double object trigger"
      break; fi
done
for _trigger in "${_triggerArray4[@]}"
do
  if [ $_trigger == $2 ]; then obj=4; 
      echo "This is a quad object trigger"
      break; fi
done

if [ $obj -lt $_testit ] 
    then echo "Could not verify Trigger. Please check trigger name!"; exit
fi

i=0
for _trigger in "${_triggerArray[@]}"
do
#  echo $_trigger
  if [ $_trigger == $2 ]; then iT=$i; break; fi
  let i=${i}+1
done
if [ $iT -lt $_testit ] 
    then echo "Please insert Trigger name!!!"; exit
fi

echo ${_triggerArray[$iT]}


# Num object array
_obj=( 1 2 4 ) 

if [ $obj = ${_obj[0]} ]; then

# y arrays: threshold values
    rm -rf $_file
    cat $_input | grep "${_triggerArray[$iT]} " | awk '{ print $2 }' > $_file 
    _yarray0=( `cat $_file` )
    
# threshold error low
    rm -rf $_file
    cat $_input | grep "${_triggerArray[$iT]} " | awk '{ print $7 }' > $_file 
    _yelarray0=( `cat $_file` )
    
# threshold error high
    rm -rf $_file
    cat $_input | grep "${_triggerArray[$iT]} " | awk '{ print $8 }' > $_file 
    _yeharray0=( `cat $_file` )

elif [ $obj = ${_obj[1]} ]; then

# y arrays: threshold values
    rm -rf $_file
    cat $_input | grep "${_triggerArray[$iT]} " | awk '{ print $2 }' > $_file 
    _yarray0=( `cat $_file` )
    
# threshold error low
    rm -rf $_file
    cat $_input | grep "${_triggerArray[$iT]} " | awk '{ print $8 }' > $_file 
    _yelarray0=( `cat $_file` )
    
# threshold error high
    rm -rf $_file
    cat $_input | grep "${_triggerArray[$iT]} " | awk '{ print $9 }' > $_file 
    _yeharray0=( `cat $_file` )

elif [ $obj = ${_obj[2]} ]; then

# y arrays: threshold values
    rm -rf $_file
    cat $_input | grep "${_triggerArray[$iT]} " | awk '{ print $2 }' > $_file 
    _yarray0=( `cat $_file` )
    
# threshold error low
    rm -rf $_file
    cat $_input | grep "${_triggerArray[$iT]} " | awk '{ print $10 }' > $_file 
    _yelarray0=( `cat $_file` )
    
# threshold error high
    rm -rf $_file
    cat $_input | grep "${_triggerArray[$iT]} " | awk '{ print $11 }' > $_file 
    _yeharray0=( `cat $_file` )
fi

_yarray=$(IFS=, ; echo "${_yarray0[*]}")
echo "y="$_yarray
echo "Double_t y["$_n"]={"$_yarray"};" >> $_macro

_yelarray=$(IFS=, ; echo "${_yelarray0[*]}")
echo "eylow="$_yelarray
echo "Double_t eyl["$_n"]={"$_yelarray"};" >> $_macro

_yeharray=$(IFS=, ; echo "${_yeharray0[*]}")
echo "eyhigh="$_yeharray
echo "Double_t eyh["$_n"]={"$_yeharray"};" >> $_macro

unset _yarray0; unset _yarray
unset _yelarray0; unset _yelarray
unset _yeharray0; unset _yeharray

echo "TGraphAsymmErrors *g= new TGraphAsymmErrors("$_n",x,y,ex,ex,eyl,eyh);" >> $_macro
echo "TCanvas *c1=new TCanvas(\"can1\",\"\");" >> $_macro
echo "gPad->SetGridx(); gPad->SetGridy();" >> $_macro
echo "g->Draw(\"AP4\");" >> $_macro
echo "g->GetYaxis()->SetRangeUser(0.,500.);" >> $_macro
echo "g->SetTitle(\";Bandwidth (kHz);Threshold (GeV)\");" >> $_macro
echo "g->SetFillColor(3);" >> $_macro
echo "}" >> $_macro

root -l $_macro
