#ifndef __MYCLASSES__
#define __MYCLASSES__

// start data objects  

#include "hstart2cal.h"
#include "myIncludes.h"

//global constants for setup variables

const Int_t nModules = 4;
const Int_t nChPerMod = 20;
const Int_t nChPerModReal = 16;
const Int_t nChannels = nModules*nChPerMod;
const Float_t loLimitChannels = 0.5;
const Float_t hiLimitChannels = nChannels + loLimitChannels;

const Int_t nHistForTimeWalk = 2;

const Int_t nPointsForTimeWalk = 17;
const Float_t widthLoLimitTW = 2.0;
const Float_t widthHiLimitTW = 19.0;
const Float_t widthBinWidth = (widthHiLimitTW - widthLoLimitTW)/nPointsForTimeWalk;

const Float_t refWidthLo[2] = {12.8, 12.2};
const Float_t refWidthHi[2] = {15.1, 14.5};

const Float_t widthLoLimit = 0.0;
const Float_t widthHiLimit = 25.0;

const Float_t tDiffLoLimit = -50.0;
const Float_t tDiffHiLimit =  50.0;

const Int_t nBinsForWidth = 200;
const Int_t nBinsForTimeDiff = 400;

const Int_t refCh[4] = {9, 23, 42, 62};

/*

const Int_t ref1 = 2;
const Int_t ref2 = 22;
const Int_t ref3 = 42; // not important for feb24
const Int_t ref4 = 62; // not important for feb24

*/


Int_t getBinForTimeWalk (Float_t width) {
	
	// this function will give you the bin corresponding to the given width
	// it needs to know global constants: npoints, lo and hi limit for width
	
	/*
const Int_t nPointsForTimeWalk = 15;
const Float_t widthLoLimitTW = 0.0;
const Float_t widthHiLimitTW = 15.0;
const Float_t widthBinWidth = (widthHiLimitTW - widthLoLimitTW)/nPointsForTimeWalk;
	*/
	
	if (width < widthLoLimitTW || width > widthHiLimitTW) return -1;
	Int_t bin = Int_t((width - widthLoLimitTW)/widthBinWidth);
	cout << "this is the getBinForTimeWalk function returning bin "<< bin << " for width = " << width << endl;
	return bin;
	
}


Int_t getRefModule(Int_t channel) {
	
	//ugly hardcoded function i know
	Int_t module = -1;
	if (channel >=1 && channel <= nChPerMod) module = 1;
	else if (channel >=nChPerMod+1 && channel <= 2*nChPerMod) module = 0;
	cout << "getRefModule returning " << module << " for channel = " << channel << endl;
	return module;
}

class StartCalibration : public HReconstructor {

	protected:
	
	    // pointer to outputfile
	    TFile* out; 
	    
	    // settings 
	    Bool_t fillHistograms = true;
	   
	    // histogram declarations
	    TH1F* hMultiplicity;
	    TH2F* hMultiplicityPerModule;
	    TH2F* hWidthVsChannel;
	    TH2F* hAbsTimeVsChannel;
	    TH2F* hTimeDiffVsChannel[nChannels];
	    TH2F* hTimeDiffVsWidth[nChannels];
	    TH1F* hTimeDiffForTimeWalk[nChannels][nPointsForTimeWalk];
	    TH1F* hWidthForTimeWalk[nChannels][nPointsForTimeWalk];
	    
	    // HADES stuff declarations
	    HCategory* Start2CalCategory;
	    HStart2Cal* Start2CalObjectRef;
	    HStart2Cal* Start2CalObject;
	    HEventHeader* eventHeader;

	public:
	
		//default constructor
	    StartCalibration (const Text_t *name = "", const Text_t *title ="", TFile* fout = NULL) : HReconstructor(name, title) { 
	
			out = fout;
	    
	    }
	
		//destructor
	    virtual ~StartCalibration () {
	    }
	
	    Bool_t init() {
			
			cout << ">>> DEBUG <<< Initializing the StartCalibration task..." <<endl;
			
			// this function is called once in the beginning
			// create histograms or get pointer to param containers
			// or data containers here. Tip: Create first your
			// output file and and after that your histograms/ ntuples.
			// In this way you make shure the object will end up in your
		    // root file and not any other one.
		
		//rpcCalCategory = NULL;
		Start2CalObjectRef = NULL;
		Start2CalObject = NULL;
		eventHeader = NULL;

		Start2CalCategory = HCategoryManager::getCategory(catStart2Cal);
	    //rpcClus = HCategoryManager::getCategory(catRpcCluster);   //
	    //candCat = HCategoryManager::getCategory(catParticleCand); //
		//rpcHit = HCategoryManager::getCategory(catRpcHit);
		
		if(out) {
				
		   out->cd();
		
		// histogram definitions

			hWidthVsChannel = new TH2F(
			"hWidthVsChannel", Form("START width vs. channel; %i #times module + channel; START width [a.u.]", nChPerMod), 
			nChannels, loLimitChannels, hiLimitChannels, nBinsForWidth, widthLoLimit, widthHiLimit);

			hMultiplicity = new TH1F(
			"hMultiplicity", "START multiplicity; mult; counts", 
			10, 0, 10);

			hMultiplicityPerModule = new TH2F(
			"hMultiplicityPerModule", "START multiplicity per module; module; mult", 
			nModules, 0.5, nModules + 0.5, 50, 0, 50);
			   
			hAbsTimeVsChannel = new TH2F(
			"hAbsTimeVsChannel", "START absolute time in channels; channel; time", 
			nChannels, 0.5, nChannels + 0.5, 2000, -1000, 1000);
			   
			for (Int_t i = 0; i<nChannels; i++) {
				
				hTimeDiffVsChannel[i] = new TH2F(
				Form("hTimeDiffVsChannel_refCh%i", i+1), 
				Form("Time difference vs. channels, reference channel = %i; %i #times module + channel; t diff", i+1, nChPerMod), 
				nChannels, loLimitChannels, hiLimitChannels, nBinsForTimeDiff, tDiffLoLimit, tDiffHiLimit);
		  
				hTimeDiffVsWidth[i] = new TH2F(
				Form("hTimeDiffVsWidth_%i", i+1), 
				Form("histogram for time walk, ref%i, test%i; test width; t_{ref} - t_{test}", refCh[getRefModule(i+1)], i+1), 
				nBinsForWidth, widthLoLimit, widthHiLimit, nBinsForTimeDiff, tDiffLoLimit, tDiffHiLimit);
		   
				for (Int_t j = 0; j<nPointsForTimeWalk; j++) {
			
					hTimeDiffForTimeWalk[i][j] = new TH1F(
					Form("hTimeDiffForTimeWalk_refCh%i_testCh%i_widthBin%i", refCh[getRefModule(i+1)], i+1, j), 
					Form("Time difference histogram, ref = ch%i, test = ch%i, widthBin%i, refwidth in range %f-%f; t_{diff} [ns]", 
					refCh[getRefModule(i+1)], i+1, j, refWidthLo[getRefModule(i+1)], refWidthHi[getRefModule(i+1)]), 
					nBinsForTimeDiff, tDiffLoLimit, tDiffHiLimit);
					
					hWidthForTimeWalk[i][j] = new TH1F(
					Form("hWidthForTimeWalk_refCh%i_testCh%i_widthBin%i", refCh[getRefModule(i+1)], i+1, j), 
					"", 
					100, widthLoLimitTW, widthHiLimitTW);
					
				}
			}
			
		}	
			return kTRUE;
	}
    
	    Bool_t reinit() {   
			// this function is called for each file
	        // after init()
			// use this place if you need already initialized
	        // containers
			Start2CalCategory = HCategoryManager::getCategory(catStart2Cal);
		    //rpcClus = HCategoryManager::getCategory(catRpcCluster);   //
		    //candCat = HCategoryManager::getCategory(catParticleCand); //
			return kTRUE;
	    }
	
	    Int_t execute() {   
			
			
			//cout << ">>> DEBUG <<< Executing the StartCalibration task..." <<endl;
			
			// this function is called once per event.
			// if the function returns kSkipEvent the event
			// will be skipped a. for all following tasks
			// and b. not appear in output (hades eventLoop())
			
			// this is the part actually responsible for doing the hard work for us
			// find appropriate variables and fill the histograms with them	
			
			Int_t nEntr = Start2CalCategory->getEntries();
			Int_t totalMultiplicityMod0 = 0;
			Int_t totalMultiplicityMod1 = 0;
			Int_t totalMultiplicityMod2 = 0;
			Int_t totalMultiplicityMod3 = 0;
			
			for(Int_t i = 0; i<nEntr; i++) {
				
				//cout << ">>> DEBUG <<< entering i loop, i = " << i << endl;
	                    
				Start2CalObjectRef = (HStart2Cal*) HCategoryManager::getObject(Start2CalObjectRef,Start2CalCategory,i);
	       	    if(!Start2CalObjectRef) continue;
	            	    
			    Int_t moduleRef   = Start2CalObjectRef -> getModule();
	            Int_t channelRef  = Start2CalObjectRef -> getStrip();
	            Int_t indexRef    = nChPerMod*moduleRef + channelRef;
	            if (moduleRef > nModules - 1) continue;
	            
	            //cout << "debug channel, moduleRef = " << moduleRef << " and channelRef = " << channelRef << endl;
	            
	            Int_t multiplicityRef = Start2CalObjectRef -> getMultiplicity();
	            
	            hMultiplicity->Fill(multiplicityRef);
	           if (moduleRef == 0) totalMultiplicityMod0 += multiplicityRef;
	           if (moduleRef == 1) totalMultiplicityMod1 += multiplicityRef;
	           if (moduleRef == 2) totalMultiplicityMod2 += multiplicityRef;
	           if (moduleRef == 3) totalMultiplicityMod3 += multiplicityRef;
				
				// small loop over all entries 
				
				for (Int_t i_mult = 0; i_mult<multiplicityRef; i_mult++) {
					
					
					Double_t widthRef = Start2CalObjectRef -> getWidth(i_mult+1);
					Double_t timeRef = Start2CalObjectRef -> getTime(i_mult+1);		
					hWidthVsChannel -> Fill(indexRef, widthRef);
					hAbsTimeVsChannel -> Fill (indexRef, timeRef);
						
					
				}
				
				for (Int_t j = 0; j<nEntr; j++) {
					
					if (j==i) continue;
					
					//cout << "entering big j loop, j = " << j << endl;
					
					Start2CalObject = (HStart2Cal*) HCategoryManager::getObject(Start2CalObject,Start2CalCategory,j);
					if(!Start2CalObject) continue;
						
					Int_t module   = Start2CalObject -> getModule();
					Int_t channel  = Start2CalObject -> getStrip();
					Int_t index    = nChPerMod*module + channel;
					
					if (module == moduleRef) continue;
					
					Int_t multiplicity = Start2CalObject -> getMultiplicity();
						
					for (Int_t i_mult = 0; i_mult<multiplicityRef; i_mult++) {
							
						//cout << "entering i_mult loop, i_mult = " << i_mult << endl;
						for (Int_t j_mult = 0; j_mult<multiplicity; j_mult++) {
							
							//cout << "entering j_mult loop, j_mult = " << j_mult << endl;
							
							Double_t widthRef = Start2CalObjectRef -> getWidth(i_mult+1);	
							Double_t timeRef = Start2CalObjectRef -> getTime(i_mult+1);	
							
							Double_t width = Start2CalObject -> getWidth(j_mult+1);
							Double_t time = Start2CalObject -> getTime(j_mult+1);
							
							Double_t tDiff = timeRef - time;
							//cout << " about to fill hist, indexRef = " << indexRef << endl;
							hTimeDiffVsChannel[indexRef - 1] -> Fill(index, tDiff);
							
							// time walk analysis
							
							if (indexRef == refCh[0] && widthRef > refWidthLo[0] && widthRef < refWidthHi[0]) {
								
								hTimeDiffVsWidth[index]->Fill(width, tDiff);
								Int_t bin = getBinForTimeWalk(width);
								if (bin>=0 && bin < nPointsForTimeWalk) {
									hTimeDiffForTimeWalk[index][bin]->Fill(tDiff);
									hWidthForTimeWalk[index][bin]->Fill(width);
								}
								
							}
							
							if (indexRef == refCh[1] && widthRef > refWidthLo[1] && widthRef < refWidthHi[1]) {
								
								hTimeDiffVsWidth[index]->Fill(width, tDiff);
								Int_t bin = getBinForTimeWalk(width);
								if (bin>=0 && bin < nPointsForTimeWalk) {
									hTimeDiffForTimeWalk[index][bin]->Fill(tDiff);
									hWidthForTimeWalk[index][bin]->Fill(width);
								}
								
							}
							
							
						}
					}
				}
				
				
	            hMultiplicityPerModule->Fill(1, totalMultiplicityMod0);
	            hMultiplicityPerModule->Fill(2, totalMultiplicityMod1);
	            hMultiplicityPerModule->Fill(3, totalMultiplicityMod2);
	            hMultiplicityPerModule->Fill(4, totalMultiplicityMod3);
				
			}
		
		return 0;
			
		}
	
	    Bool_t finalize() {   
		
		// this function is called once in the end of the
		// runtime of the program. Save histograms to file
		// etc. Tip: Change to your ROOT output file before
		// writing our histograms. You might not be the only
		// user of root files and the current directory could
	        // the one from another user.
		
		std::cout << "Finalizing the StartCalibration task..." << endl;
		
			if(out) {
				
		        out->cd();
		       
		       // write histograms

				hMultiplicity->Write();
				hMultiplicityPerModule->Write();
				hWidthVsChannel->Write();
				hAbsTimeVsChannel->Write();
				
				for (Int_t i = 0; i<nChannels; i++) { 
					
					if (i >= 0*nChPerMod && i < 1*nChPerMod) {
						out->mkdir("time diffs module 0");
						out->cd("time diffs module 0");
					} 
					
					
					else if (i >= 1*nChPerMod && i < 2*nChPerMod) {
						out->mkdir("time diffs module 1");
						out->cd("time diffs module 1");
					} 
					
					
					else if (i >= 2*nChPerMod && i < 3*nChPerMod) {
						out->mkdir("time diffs module 2");
						out->cd("time diffs module 2");
					} 
					
					
					else if (i >= 3*nChPerMod && i < 4*nChPerMod) {
						out->mkdir("time diffs module 3");
						out->cd("time diffs module 3");
					} 
					
					hTimeDiffVsChannel[i]->Write();
					
				}
				
				
				out->mkdir("2D hists for time walk");
				out->mkdir("1D hists for time walk fitting");
					
				for (Int_t i = 0; i<nChannels; i++) { 
					
					out->cd("2D hists for time walk");
					hTimeDiffVsWidth[i]->Write();
					
					for (Int_t j = 0; j<nPointsForTimeWalk; j++) {
						
						out->cd("1D hists for time walk fitting");
						hTimeDiffForTimeWalk[i][j]->Write();
						hWidthForTimeWalk[i][j]->Write();
					}
				}
					
				//out->Save();
			    //out->Close(); // don't close the file since the file is handled outside of the task
		       
			}
	
		return kTRUE;
		
	    }

};

#endif
