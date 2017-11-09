#include "samples.h"
#include "NTupleReader.h"
#include "StopleAlias.h"
#include "baselineDef.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include <iostream>
#include <cstdio>
#include <string>
#include <ctime>
#include <iomanip>
#include <cmath>
#include "TH1.h"
#include "TRandom3.h"
#include <getopt.h>

int main(int argc, char* argv[])
{
    char nBase[] = "root://cmseos.fnal.gov//store/user/mkilpatr/13TeV/261017_run2017b/merged/met-2017b-22jun17_1_ntuple_postproc.root";
    char nBase2[]= "root://cmseos.fnal.gov//store/user/lpcsusyhad/Stop_production/2017Test/MET/Summer16_80X_Mar_2017_Ntp_v13p0_MET-Run2017B-23Jun2017-v1/170920_010234/0000/stopFlatNtuples_21.root";

    using namespace std;

    int opt;
    int option_index = 0;
    static struct option long_options[] = {
	{"condor",           no_argument, 0, 'c'},
        {"dataSets",   required_argument, 0, 'D'},
        {"numFiles",   required_argument, 0, 'N'},
        {"startFile",  required_argument, 0, 'M'},
        {"numEvts",    required_argument, 0, 'E'},
    };
    bool runOnCondor = false;
    string dataSets = "", sampleloc = AnaSamples::fileDir, outFile = "AliasTestRoot", sampleRatios = "1:1";
    int nFiles = -1, startFile = 0, nEvts = -1, printInterval = 10000;

    while((opt = getopt_long(argc, argv, "cD:N:M:E:", long_options, &option_index)) != -1)
    {
        switch(opt)
        {
        case 'c':
            runOnCondor = true;
            break;

        case 'D':
            dataSets = optarg;
            break;

        case 'N':
            nFiles = int(atoi(optarg));
            break;

        case 'M':
            startFile = int(atoi(optarg));
            break;

        case 'E':
            nEvts = int(atoi(optarg)) - 1;
            break;

        }
    }

    //if running on condor override all optional settings
    if(runOnCondor)
    {
        char thistFile[128];
        sprintf(thistFile, "AliasTestRoot_%s_%d", dataSets.c_str(), startFile);
        outFile = thistFile;
        sampleloc = "condor";
    }

    AnaSamples::SampleSet        ss(sampleloc);
    AnaSamples::SampleCollection sc(ss);

    TH1* electronpt = new TH1D("electronpt","Electron pt", 20, 0, 500);
    TH1* electronpt_LPC = new TH1D("electronpt_LPC","Electron pt", 20, 0, 500);
    TH1* electronCharge = new TH1D("electronCharge", "electron Char", 10, 0, 100);
    TH1* electronCharge_LPC = new TH1D("electronCharge_LPC", "electron Char", 10, 0, 100);

    TFile f("ht.root","recreate");
    size_t t0 = clock();
    
    StopleAlias alias;
    //tr.setReThrow(false);
   // tr.registerFunction(alias);

    //std::cout << "NEVTS: " << tr.getNEntries() << std::endl;

    for(auto& fs : sc[dataSets])
    {
        bool ucsb_tuples = false;
        TChain *ch_socali = new TChain(fs.treePath.c_str());
        fs.addFilesToChain(ch_socali);
        NTupleReader tr(ch_socali);
        StopleAlias alias;
        tr.setReThrow(false);
        if(!tr.checkBranch("jetsLVec"))
        { 
         tr.registerFunction(alias);
         ucsb_tuples = true;
        }
        while(tr.getNextEvent())
        {   
            if(tr.getEvtNum() % 10000 == 0) std::cout << "Event #: " << tr.getEvtNum() << std::endl;
                /*
                 if(tr.getEvtNum() == 1)
                 {
                 //tr.printTupleMembers();
                 FILE * fout = fopen("NewUCSBNTupleTypes.txt", "w");
                 tr.printTupleMembers(fout);
                 fclose(fout);
                 }*/
            if(tr.getEvtNum() > 10000)
              break;
            const std::vector<TLorentzVector>& elesLVec       = tr.getVec<TLorentzVector>("elesLVec");
            //std::cout << "MET " << tr.getVar<double>("met")  <<" met_pt " << tr.getVar<float>("met_pt") << std::endl;
            //std::cout << elesLVec.size() << std::endl;
            if(ucsb_tuples)
            {
                if(elesLVec.size()>0)
                {
                    electronpt_LPC->Fill(elesLVec[0].Pt());
                }
            }
            else
            {
                if(elesLVec.size()>0)
                {
                    electronpt->Fill(elesLVec[0].Pt());

                }
            }
        }
    ch_socali->Reset();
   }
    TFile* outfile = new TFile("test.root", "RECREATE");
    TCanvas *c1 = new TCanvas("c1","c1",800,1000);
    c1->cd();
    electronpt->Draw();
    electronpt_LPC->SetLineColor(2);
    electronpt_LPC->Draw("same");
    electronpt_LPC->Write();
    c1->SaveAs("electronpt.pdf");
    electronpt->Write();

 
    c1->Write();
    outfile->Close();
}


