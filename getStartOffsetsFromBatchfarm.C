#include "myClasses.h"
#include "myIncludes.h"

Double_t timeWalkFuncWithSqrt (Double_t *xarg, Double_t *par) {
	
	Double_t x = xarg[0];
	Double_t result = 0.0;
	
	result = par[0] + par[1] / sqrt(x);
	
	return result;
	
}

void getStartOffsetsFromBatchfarm () {

	//TString inFilePath = "~/lustre/hades/user/jorlinsk/feb24/output/startCalib/038/startCalib_feb24_raw_038_16h03_TEST14h35.root";
	//TString inFilePath = "~/lustre/hades/user/jorlinsk/feb24/output/startCalib/038/startCalib_feb24_raw_038_16h03.root";
	TString inFilePath = "~/lustre/hades/user/jorlinsk/feb24/output/startCalib/039/startCalib_feb24_raw_039_ALLDAY_TW.root";
	//TString inFilePath = "analysisDST/out.root";
	//TString outFilePath = "StartCalib_feb22_raw_042_ALL.root";
	//TString outFilePath = "StartCalib_feb24_raw_038_16h03.root";
	TString outFilePath = "StartCalib_feb24_raw_039_ALLDAY_TW.root";
	//TString outFilePath = "StartCalib_feb24_raw_038_16h03_TEST14h35.root";
	
	TFile* inFile = new TFile (inFilePath, "READ");
	TFile* outFile = new TFile (outFilePath, "RECREATE");
	
	outFile->cd();
	
	TH2F* hTDiff_ref1;
	TH2F* hTDiff_ref2;
	TH2F* hTDiff_ref3;
	TH2F* hTDiff_ref4;
	TH1F* hTDiffOffsets;
    TH1F* hTimeDiffForTimeWalk[nChannels][nPointsForTimeWalk];
    TH1F* hWidthForTimeWalk[nChannels][nPointsForTimeWalk];
    
    TGraphErrors* gTimeWalk[nChannels];
    
	hTDiffOffsets = new TH1F("hTDiffOffsets", "hTDiffOffsets", nChannels, loLimitChannels, hiLimitChannels);
			   
	hTDiff_ref1 = (TH2F*) inFile->Get(Form("time diffs module 0/hTimeDiffVsChannel_refCh%i", refCh[0]));
	hTDiff_ref2 = (TH2F*) inFile->Get(Form("time diffs module 1/hTimeDiffVsChannel_refCh%i", refCh[1]));
	hTDiff_ref3 = (TH2F*) inFile->Get(Form("time diffs module 2/hTimeDiffVsChannel_refCh%i", refCh[2]));
	hTDiff_ref4 = (TH2F*) inFile->Get(Form("time diffs module 3/hTimeDiffVsChannel_refCh%i", refCh[3]));
	
	for (Int_t i = 0; i<nChannels; i++) {
		
		gTimeWalk[i] = new TGraphErrors(nPointsForTimeWalk);
			
		for (Int_t j = 0; j<nPointsForTimeWalk; j++) {
	
			hTimeDiffForTimeWalk[i][j] = (TH1F*) inFile->Get(
			Form("1D hists for time walk fitting/hTimeDiffForTimeWalk_refCh%i_testCh%i_widthBin%i", refCh[getRefModule(i+1)], i+1, j));
			
			hWidthForTimeWalk[i][j] = (TH1F*) inFile->Get(
			Form("1D hists for time walk fitting/hWidthForTimeWalk_refCh%i_testCh%i_widthBin%i", refCh[getRefModule(i+1)], i+1, j));
			
		}
	}
	

	fillTimeOffsets(hTDiffOffsets, hTDiff_ref1, 1*nChPerMod+1, 2*nChPerMod, outFile, "ref0");
	fillTimeOffsets(hTDiffOffsets, hTDiff_ref2, 0*nChPerMod+1, 1*nChPerMod, outFile, "ref1");
	//fillTimeOffsets(hTDiffOffsets, hTDiff_ref3, 49, 96, outFile, "ref3"); 
	//fillTimeOffsets(hTDiffOffsets, hTDiff_ref4, 1, 48, outFile, "ref4");
	
	// time walk analysis
	// mod0
	
	const Float_t fitRadius = 4.5;
	for (Int_t i = 0; i<nChannels; i++) {
		
		for (Int_t j = 0; j<nPointsForTimeWalk; j++) {
			
			cout << "entering time walk analysis, i = " << i << ", j = " << j << endl;
			
			Double_t positionOfMaximum = hTimeDiffForTimeWalk[i][j]->GetBinCenter(hTimeDiffForTimeWalk[i][j]->GetMaximumBin());
			Double_t heightOfMaximum = hTimeDiffForTimeWalk[i][j]->GetMaximum();
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
			hTimeDiffForTimeWalk[i][j]->Fit(gausFit, "W");
			Double_t center = gausFit->GetParameter(1);
			Double_t centerError = gausFit->GetParError(1);
			
			if (centerError > 5.0) continue; //dont include points with a large fit error (this is mostly for cosmetics)
			
			gTimeWalk[i]->SetPoint(j, hWidthForTimeWalk[i][j]->GetMean(), center);
			gTimeWalk[i]->SetPointError(j, hWidthForTimeWalk[i][j]->GetStdDev(), centerError);
			
		}
		
		TF1* timeWalkFit = new TF1 (Form("timeWalkSqrtFit_%i", i+1), timeWalkFuncWithSqrt, widthLoLimitTW, widthHiLimitTW, 2);
		gTimeWalk[i]->Fit(timeWalkFit, "R+");
		
	}
		
	outFile->cd();
		
	hTDiff_ref1->Write();
	hTDiff_ref2->Write();
	hTDiff_ref3->Write();
	hTDiff_ref4->Write();
	hTDiffOffsets->Write();
	
	for (Int_t i=0; i<nChannels; i++) gTimeWalk[i]->Write();
		
	outFile->Save();
	outFile->Close();
}
