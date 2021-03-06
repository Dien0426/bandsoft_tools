#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TVector3.h"
#include "TH2.h"
#include "TH1.h"
#include "TF1.h"
#include "TCanvas.h"

#include "reader.h"
#include "bank.h"
#include "clas12fiducial.h"
#include "e_pid.h"

#include "BParticle.h"
#include "BCalorimeter.h"
#include "BScintillator.h"
#include "BEvent.h"

#include "RCDB/Connection.h"

#include "constants.h"
#include "readhipo_helper.h"

using namespace std;
double FF(double E, double sf1, double sf2, double sf3, double sf4){
  return sf1 * (sf2 + (sf3/E) + (sf4/(E*E)) ); 
}

double diag(double x, double C){
  return C - x;
}

int main(int argc, char** argv) {

  if( argc != 3 ){
    cerr << "Incorrect number of arugments. Instead use:\n\t./code [outputFile] [inputFile] \n\n";
    return -1;
  }
  

  //Define Variables
  int Runno;
  double Ebeam, gated_charge, livetime, starttime, current;
  clashit * eHit = new clashit;
 //Creat input tree
  TFile * inFile = new TFile(argv[2]);
  TTree * inTree = (TTree*)inFile->Get("electrons");
  // 	Event branches:
  inTree->SetBranchAddress("Runno"               ,&Runno                 );
  inTree->SetBranchAddress("Ebeam"               ,&Ebeam                 );
  inTree->SetBranchAddress("gated_charge"        ,&gated_charge          );
  inTree->SetBranchAddress("livetime"            ,&livetime              );
  inTree->SetBranchAddress("starttime"           ,&starttime             );
  inTree->SetBranchAddress("current"             ,&current               );
  //	Electron branches:
  inTree->SetBranchAddress("eHit"		 ,&eHit			 );

  // Create output tree
  TFile * outFile = new TFile(argv[1],"RECREATE");
  TTree * outTree = new TTree("electrons","CLAS Electrons");
  // 	Event branches:
  outTree->Branch("Runno"		,&Runno			);
  outTree->Branch("Ebeam"		,&Ebeam			);
  outTree->Branch("gated_charge"	,&gated_charge		);
  outTree->Branch("livetime"	        ,&livetime		);
  outTree->Branch("starttime"	        ,&starttime		);
  outTree->Branch("current"	        ,&current		);
  //	Electron branches:
  outTree->Branch("eHit"		,&eHit			);

  //Make some histograms
  vector<TH2*> hist_list;
  //Minimum pcal plots
  TH2D * h2_Eec_Epcal[6];
  //Hit Postion plots
  TH2D * h2_SF_V_Wide[6];
  TH2D * h2_SF_V_Narrow[6];
  TH2D * h2_SF_W_Wide[6];
  TH2D * h2_SF_W_Narrow[6];
  //Sampling Fraction PID
  TH2D * h2_SF_Epcal[6];
  TH2D * h2_SF_p[6];
  TH2D * h2_SF_Etot[6];
  //Comparing elements of ECal
  TH2D * h2_SFpcal_SFecin[6];
  TH2D * h2_SFpcal_SFecin_bin[6];
  //TH2D * h2_SFpcal_SFecin_wCut[6];
  //H2D * h2_SFpcal_SFecout[6];

  char temp[100];

  for(int i = 0; i<6; i++){
    sprintf(temp,"Eec_v_Epcal_sec%d",i);
    h2_Eec_Epcal[i] = new TH2D(temp,"Eec_v_Epcal;Epcal;Eec;Events",100,0,0.5,350,0.0,0.35);
    hist_list.push_back(h2_Eec_Epcal[i]);
  }

  for(int i = 0; i<6; i++){
    sprintf(temp,"SF_v_V_Wide_sec%d",i);
    h2_SF_V_Wide[i] = new TH2D(temp,"SF_v_V_Wide;V;SF;Events",400,0,400,150,0.05,0.40);
    hist_list.push_back(h2_SF_V_Wide[i]);
  }  

  for(int i = 0; i<6; i++){
    sprintf(temp,"SF_v_V_Narrow_sec%d",i);
    h2_SF_V_Narrow[i] = new TH2D(temp,"SF_v_V_Narrow;V;SF;Events",60,0,30,150,0.05,0.40);
    hist_list.push_back(h2_SF_V_Narrow[i]);
  }  

  for(int i = 0; i<6; i++){
    sprintf(temp,"SF_v_W_Wide_sec%d",i);
    h2_SF_W_Wide[i] = new TH2D(temp,"SF_v_W_Wide;W;SF;Events",400,0,400,150,0.05,0.40);
    hist_list.push_back(h2_SF_W_Wide[i]);
  }  

  for(int i = 0; i<6; i++){
    sprintf(temp,"SF_v_W_Narrow_sec%d",i);
    h2_SF_W_Narrow[i] = new TH2D(temp,"SF_v_W_Narrow;W;SF;Events",60,0,30,150,0.05,0.40);
    hist_list.push_back(h2_SF_W_Narrow[i]);
  }  

  for(int i = 0; i<6; i++){
    sprintf(temp,"SF_v_Epcal_sec%d",i);
    h2_SF_Epcal[i] = new TH2D(temp,"SF_v_Epcal;Epcal;SF;Events",250,0,1.7,150,0.05,0.40);
    hist_list.push_back(h2_SF_Epcal[i]);
  }

  for(int i = 0; i<6; i++){
    sprintf(temp,"SF_v_p_sec%d",i);
    h2_SF_p[i] = new TH2D(temp,"SF_v_p;p;SF;Events",250,0,10,150,0.05,0.40);
    hist_list.push_back(h2_SF_p[i]);
  }

  for(int i = 0; i<6; i++){
    sprintf(temp,"SF_v_Etot_sec%d",i);
    h2_SF_Etot[i] = new TH2D(temp,"SF_v_Etot;Etot;SF;Events",200,0,2.5,150,0.05,0.40);
    hist_list.push_back(h2_SF_Etot[i]);
  }

  for(int i = 0; i<6; i++){
    sprintf(temp,"SFpcal_v_SFecin_sec%d",i);
    h2_SFpcal_SFecin[i] = new TH2D(temp,"SFpcal_v_SFecin;SFecin;SFpcal;Events",350,0.001,0.15,350,0.0,0.35);
    hist_list.push_back(h2_SFpcal_SFecin[i]);
  }

  for(int i = 0; i<4; i++){
    sprintf(temp,"SFpcal_v_SFecin_sec4_bin%d",i);
    h2_SFpcal_SFecin_bin[i] = new TH2D(temp,"SFpcal_v_SFecin_binned;SFecin;SFpcal;Events",350,0.001,0.15,350,0.0,0.35);
    hist_list.push_back(h2_SFpcal_SFecin_bin[i]);
  }

  /*  for(int i = 0; i<6; i++){
    sprintf(temp,"SFpcal_v_SFecin_wCut_sec%d",i);
    h2_SFpcal_SFecin_wCut[i] = new TH2D(temp,"SFpcal_v_SFecin_wCut;SFecin_wCut;SFpcal;Events",350,0.001,0.15,350,0.0,0.35);
    hist_list.push_back(h2_SFpcal_SFecin_wCut[i]);
  }

  for(int i = 0; i<6; i++){
    sprintf(temp,"SFpcal_v_SFecout_sec%d",i);
    h2_SFpcal_SFecout[i] = new TH2D(temp,"SFpcal_v_SFecout;SFecout;SFpcal;Events",350,0.001,0.15,350,0.0,0.35);
    hist_list.push_back(h2_SFpcal_SFecout[i]);
    }*/
 
  int fin = inTree->GetEntries();
  int lowP = 0;
  int highP = 0;
  
  //Initiallize PID with beam energy
  inTree->GetEntry(0);
  e_pid eHitPID;
  eHitPID.setParamsRGB(Ebeam); 

  for(int i = 0; i < fin; i++){
    eHit->Clear();
    inTree->GetEntry(i);

    //Display completed  
    if((i%100000) == 0){
      cerr << (i*100.)/fin <<"% complete \n";
    }

    //Do ECal Fid and PID cuts
    if(eHit->getV() < 14){ continue; }
    if(eHit->getW() < 14){ continue; }
    if(!eHitPID.isElectron(eHit)){ continue; }

    int sector = eHit->getSector() - 1;
    double p = eHit->getMomentum();
    h2_Eec_Epcal[sector]->Fill(eHit->getEpcal(),eHit->getEecin() + eHit->getEecout());
    h2_SF_V_Wide[sector]->Fill(eHit->getV(),eHit->getEoP());
    h2_SF_V_Narrow[sector]->Fill(eHit->getV(),eHit->getEoP());
    h2_SF_W_Wide[sector]->Fill(eHit->getW(),eHit->getEoP());
    h2_SF_W_Narrow[sector]->Fill(eHit->getW(),eHit->getEoP());
    h2_SF_Epcal[sector]->Fill(eHit->getEpcal(),eHit->getEoP());
    h2_SF_p[sector]->Fill(p,eHit->getEoP());
    h2_SF_Etot[sector]->Fill(eHit->getEtot(),eHit->getEoP());
    h2_SFpcal_SFecin[sector]->Fill(eHit->getEecin()/p,eHit->getEpcal()/p);
    //h2_SFpcal_SFecout[sector]->Fill(eHit->getEecout()/p,eHit->getEpcal()/p);
    
    if(sector == 3){
      if(p < 2.5){
	h2_SFpcal_SFecin_bin[0]->Fill(eHit->getEecin()/p,eHit->getEpcal()/p);
      }
      else if(p < 4.5){
	h2_SFpcal_SFecin_bin[1]->Fill(eHit->getEecin()/p,eHit->getEpcal()/p);
      }
      else if(p < 7){
	h2_SFpcal_SFecin_bin[2]->Fill(eHit->getEecin()/p,eHit->getEpcal()/p);
      }
      else{
	h2_SFpcal_SFecin_bin[3]->Fill(eHit->getEecin()/p,eHit->getEpcal()/p);
      }

    }

    lowP++;
    if(eHit->getEpcal()>0.07){highP++;}

    //double SFpi = (eHit->getEpcal() + eHit->getEecin()) / p;
    //if((p>4.5) && (SFpi<0.2)){continue;}
    //h2_SFpcal_SFecin_wCut[sector]->Fill(eHit->getEecin()/p,eHit->getEpcal()/p);
    //outTree->Fill();

  }

  /*
  TF1 * Pos_Step1 = new TF1("step",[&](double *x, double *p){ return step(x[0],p[0]); }, 0.0,30,1 );
  Pos_Step1->SetLineColor(1);
  TF1 * Pos_Step2 = new TF1("step",[&](double *x, double *p){ return step(x[0],p[0]); }, 0.0,30,1 );
  Pos_Step1->SetLineColor(2);
  TF1 * Pos_Step3 = new TF1("step",[&](double *x, double *p){ return step(x[0],p[0]); }, 0.0,30,1 );
  Pos_Step3->SetLineColor(3);

  for(int i = 0; i<6; i++){
    sprintf(temp,"SF_v_V_sec%d",i);
    TCanvas * c1 = new TCanvas(temp,temp,1200,1000);
    c1->cd();
    h2_SF_V_Narrow[i]->Draw("colz");

    Pos_Step1->SetParameter(0,9);
    Pos_Step1->Draw("SAME");

    Pos_Step2->SetParameter(0,14);
    Pos_Step2->Draw("SAME");

    Pos_Step2->SetParameter(0,19);
    Pos_Step2->Draw("SAME");

    sprintf(temp,"SF_v_V_sec%d.pdf",i);
    c1->SaveAs(temp);
  }

  for(int i = 0; i<6; i++){
    sprintf(temp,"SF_v_W_sec%d",i);
    TCanvas * c1 = new TCanvas(temp,temp,1200,1000);
    c1->cd();
    h2_SF_W_Narrow[i]->Draw("colz");

    Pos_Step1->SetParameter(0,9);
    Pos_Step1->Draw("SAME");

    Pos_Step2->SetParameter(0,14);
    Pos_Step2->Draw("SAME");

    Pos_Step2->SetParameter(0,19);
    Pos_Step2->Draw("SAME");

    sprintf(temp,"SF_v_W_sec%d.pdf",i);
    c1->SaveAs(temp);
  }


  
  TF1 * SF_mu = new TF1("SF_mu",[&](double *x, double *p){ return mu_SF(x[0],p[0],p[1],p[2]); }, 0.01,1.7,3 );
  SF_mu->SetLineColor(1);
  TF1 * SF_max = new TF1("SF_max",[&](double *x, double *p){ return max_SF(x[0],p[0],p[1],p[2],p[3]); }, 0.01,1.7,4 );
  TF1 * SF_min = new TF1("SF_min",[&](double *x, double *p){ return min_SF(x[0],p[0],p[1],p[2],p[3]); }, 0.01,1.7,4 );
  

  for(int i = 0; i<6; i++){
    sprintf(temp,"SF_v_Epcal_sec%d",i);
    TCanvas * c1 = new TCanvas(temp,temp,1200,1000);
    c1->cd();
    h2_SF_Epcal[i]->Draw("colz");

    
    SF_mu->SetParameter(0,SF1[i]);
    SF_mu->SetParameter(1,SF3[i]);
    SF_mu->SetParameter(2,SF4[i]);
    SF_mu->Draw("SAME");

    SF_max->SetParameter(0,SF1[i]);
    SF_max->SetParameter(1,SF3[i]);
    SF_max->SetParameter(2,SF4[i]);
    SF_max->SetParameter(3,SFs1[i]);
    SF_max->Draw("SAME");

    SF_min->SetParameter(0,SF1[i]);
    SF_min->SetParameter(1,SF3[i]);
    SF_min->SetParameter(2,SF4[i]);
    SF_min->SetParameter(3,SFs1[i]);
    SF_min->Draw("SAME");
    
    sprintf(temp,"SF_v_Epcal_sec%d.pdf",i);
    c1->SaveAs(temp);
  }
  
  TF1 * SF_diag1 = new TF1("diag",[&](double *x, double *p){ return diag(x[0],p[0]); }, 0.001,1.5,1 );
  SF_diag1->SetLineColor(1);
  TF1 * SF_diag2 = new TF1("diag",[&](double *x, double *p){ return diag(x[0],p[0]); }, 0.001,1.5,1 );

  for(int i = 0; i<6; i++){
    sprintf(temp,"SFpcal_v_SFecin_sec%d",i);
    TCanvas * c1 = new TCanvas(temp,temp,1200,1000);
    c1->cd();
    h2_SFpcal_SFecin[i]->Draw("colz");

    SF_diag1->SetParameter(0,0.2);
    SF_diag1->Draw("SAME");

    SF_diag2->SetParameter(0,0.15);
    SF_diag2->Draw("SAME");

    sprintf(temp,"SFpcal_v_SFecin_sec%d.pdf",i);
    c1->SaveAs(temp);
  }

  for(int i = 0; i<6; i++){
    sprintf(temp,"SFpcal_v_SFecin_wCut_sec%d",i);
    TCanvas * c1 = new TCanvas(temp,temp,1200,1000);
    c1->cd();
    h2_SFpcal_SFecin_wCut[i]->Draw("colz");

    SF_diag1->SetParameter(0,0.2);
    SF_diag1->Draw("SAME");

    SF_diag2->SetParameter(0,0.15);
    SF_diag2->Draw("SAME");

    sprintf(temp,"SFpcal_v_SFecin_wCut_sec%d.pdf",i);
    c1->SaveAs(temp);
  }
  */
  cout<< ((double)lowP - (double)highP) * 100 / (double)lowP << "% survival\n";

  inFile->Close();
  outFile->cd();
  for(int i=0; i<hist_list.size(); i++){
    hist_list[i]->Write();
  }
  //outTree->Write();
  outFile->Close();
  cout<<"Finished making file: "<<argv[1]<<"\n";
  return 0;
}
