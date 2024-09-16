#include "myClasses.h"
#include "myIncludes.h"

const TString inRootFile = "analysisDST/allParam_Feb22_gen2_27022023.root";
const TString inAsciiFile = "analysisDST/feb24_dst_params.txt";
	
const TString newAsciiFile = "start2calpar_feb24_038_16h03.txt";

const TString startOffsetsFile = "StartCalib_feb24_raw_038_16h03.root";

void generateAsciiStartParFile () {
	
	Int_t nModulesForOffsets = 2;

	Int_t runId = 444571079; // copied from analysisDST
	//Int_t runId = 507316668; // copied from analysisDST
	TString asciiParFile = inAsciiFile;
	TString rootParFile  = inRootFile;
	TString paramSource  = "ascii";

	Hades* myHades       = new Hades;
	HSpectrometer* spec  = gHades->getSetup();
	HRuntimeDb* rtdb     = gHades->getRuntimeDb();

	Int_t mdcMods[6][4]=
	  { {1,1,1,1},
	  {1,1,1,1},
	  {1,1,1,1},
	  {1,1,1,1},
	  {1,1,1,1},
	  {1,1,1,1} };

	Int_t nLayers[6][4] = {
	  {6,6,5,6},
	  {6,6,5,6},
	  {6,6,5,6},
	  {6,6,5,6},
	  {6,6,5,6},
	  {6,6,5,6} };

	HDst::setupSpectrometer("feb24", mdcMods, "rich,mdc,tof,rpc,emc,sts,frpc,start,tbox");
	HDst::setupParameterSources(paramSource, asciiParFile, rootParFile, "now");

	HParAsciiFileIo* output = new HParAsciiFileIo;
	output->open(newAsciiFile.Data(), "out");
	
	rtdb->setOutput(output);
	
	HStart2Calpar* pCalPar = (HStart2Calpar*) rtdb->getContainer("Start2Calpar");
	rtdb->initContainers(runId);
	
	/* TDIFF CALIBRATION */
	
	TFile* fin = new TFile (startOffsetsFile, "read");	   
	TH1F* hTDiffOffsets  = (TH1F*) fin->Get("hTDiffOffsets");
	
	//Float_t moduleDependentOffset = 
	
	for (Int_t m=0; m<nModulesForOffsets; m++) { // loop over modules
		for (Int_t c=1; c<nChPerModReal+1; c++) { // loop over channels
			cout << "m = " << m << ", c = " << c << endl;
			Float_t oldTdcOffset   = (*pCalPar)[m][c].getTdcOffset();
			
			Float_t adjustment = hTDiffOffsets->GetBinContent(m*nChPerMod + c);
			if (adjustment > 1000 || adjustment < -1000) adjustment = 0.0;
			
			Float_t newTdcOffset  = oldTdcOffset + adjustment;
			(*pCalPar)[m][c].setTdcOffset(newTdcOffset);
			
		}
	}
	
	fin->Close();
	delete gHades;
}
