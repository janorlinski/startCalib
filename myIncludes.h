#ifndef __MYINCLUDES__
#define __MYINCLUDES__

#include "hades.h"
#include "hruntimedb.h"
#include "htask.h"

#include "htaskset.h"

#include "hevent.h"
#include "hcategory.h"
#include "hdst.h"
#include "htime.h"
#include "TH2F.h"
#include "TH2.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TGaxis.h"
#include "TAxis.h"
#include "TGraph.h"
#include "TF1.h"
#include "TArrayD.h"
#include "TNtuple.h"
#include "hiterator.h"
#include "TGraph.h"
#include "TGraphErrors.h"

#include "haddef.h"
#include "richdef.h"
#include "hmdcdef.h"
#include "hmdctrackddef.h"
#include "hmdctrackgdef.h"
#include "hmetamatch2.h"
#include "hsplinetrack.h"
#include "showerdef.h"
#include "rpcdef.h"
#include "tofdef.h"
#include "walldef.h"
#include "hloop.h"
#include "hcategorymanager.h"
#include "htofcluster.h"
#include "htofhit.h"
#include "hrpccal.h"
#include "hrpchit.h"
#include "hstartdef.h"
#include "hstart2hit.h"
#include "hparticlecand.h"

#include "hrpccluster.h"
#include "hlocation.h"
#include "hphysicsconstants.h"

#include "hrpccalpar.h"
#include "hparasciifileio.h"
#include "hstart2calpar.h"

#include "TSystem.h"
#include "TROOT.h"
#include "TFile.h"
#include "TString.h"
#include "TStopwatch.h"
#include "TString.h"
#include "TList.h"
#include "TCanvas.h"
#include "TStyle.h"

#include "TCutG.h"
#include "hparticletracksorter.h"

#include "TMath.h"

#include "myClasses.h"

#include <iostream>
#include <fstream>


Double_t gaussianWithConstBckg (Double_t *xarg, Double_t *par) {
	
	Double_t x = xarg[0];
	Double_t result = 0.0;
	Double_t pi = TMath::Pi();
	
	result = par[0]*exp(-0.5*(((x-par[1])/par[2])*(x-par[1])/par[2])/(sqrt(2*pi)*par[2])) + par[3];
	
	return result;
	
}

void fillTimeOffsets (TH1F* hOffsets, TH2F* hSource, Int_t firstBinToFill, Int_t lastBinToFill, TFile* out, TString dirname) {
	
	out->cd();
	out->mkdir(dirname);
	out->cd(dirname);
	
	const Float_t fitRadius = 7.5;
	
	for (Int_t i=firstBinToFill; i<=lastBinToFill; i++) { // loop over channels
	
		// cout << "Calculating offset for cell with index " << i << endl; 
		// get appropriate projection
		
		TH1F* projection = (TH1F*) hSource->ProjectionY(Form("projection_"+dirname+"_ch%i", i), i, i);
		//Int_t nBinsOfSource = projection->GetNbinsX();
		
		// set fit range
		
		Double_t positionOfMaximum = projection->GetBinCenter(projection->GetMaximumBin());
		Double_t heightOfMaximum = projection->GetMaximum();
		//Double_t positionOfMaximum = 0.0;
		Double_t fitLoEdge = positionOfMaximum - fitRadius;
		Double_t fitHiEdge = positionOfMaximum + fitRadius;
		std::cout << "Gaus will be fitted in range [" << fitLoEdge << ", " << fitHiEdge << "] \n";
		
		// fit and fill offset
		
		TF1* gausFit = new TF1 (Form("fit_%i", i), gaussianWithConstBckg, fitLoEdge, fitHiEdge, 4);
		gausFit->SetParameter(0, 1000);
		gausFit->SetParLimits(0, 0.2*heightOfMaximum, 1000000.0);
		
		gausFit->SetParameter(1, 1000.0);
		gausFit->SetParLimits(1, fitLoEdge + 0.25*fitRadius, fitHiEdge-0.25*fitRadius);
		
		gausFit->SetParameter(2, 3.0);
		gausFit->SetParLimits(2, 0.5, fitRadius);
		
		
		gausFit->SetParameter(3, 0.0);
		gausFit->SetParLimits(3, 0.0, 0.25*heightOfMaximum);
		
		
		gausFit->SetRange(fitLoEdge, fitHiEdge);
		projection->Fit(gausFit, "R");
		Double_t center = gausFit->GetParameter(1);
		hOffsets->SetBinContent(i, center);
		
		projection->Write();
		

	}
}

#endif
