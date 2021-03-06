#include "HiggsAnalysis/HiggsToTauTau/interface/PlotLimits.h"
#include "vector"
#include <TF1.h>
#include <TFitResult.h>

/// This is the core plotting routine that can also be used within
/// root macros. It is therefore not element of the PlotLimits class.
void plottingTanb(TCanvas& canv, TGraphAsymmErrors* plain_1, TGraphAsymmErrors* plain_2, TGraphAsymmErrors* innerBand_1, TGraphAsymmErrors* innerBand_2, TGraphAsymmErrors* innerBand_3, TGraphAsymmErrors* outerBand_1, TGraphAsymmErrors* outerBand_2, TGraphAsymmErrors* outerBand_3, TGraph* expected_1, TGraph* expected_2, TGraph* expected_3, TGraph* observed_1, TGraph* observed_2, TGraph* observed_3, TGraph* injected_1, TGraph* injected_2, std::map<double, TGraphAsymmErrors*> higgsBands, std::map<std::string, TGraph*> comparisons, std::string& xaxis, std::string& yaxis, std::string& theory, double min=0., double max=50., bool log=false, bool transparent=false, bool expectedOnly=false, bool plotOuterBand=true, bool MSSMvsSM=true, std::string HIG="", bool BlackWhite=false);

struct myclass {
  bool operator() (int i,int j) { return (i<j);}
} myobject;

void
PlotLimits::plotTanb(TCanvas& canv, const char* directory, std::string HIG)
{
  //separate between MSSMvsSM and MSSMvsBG
  double exclusion_=0;
  if(MSSMvsSM_) exclusion_=0.05;
  else exclusion_=1;
  //different MSSM scenarios
  std::string extralabel_ = "";
  const char* model;
  double tanbHigh, tanbLow, tanbLowHigh;
  if(theory_=="MSSM m_{h}^{max} scenario") {extralabel_= "mhmax-"; model = "mhmax-mu+200"; tanbHigh=60; tanbLow=0.5; tanbLowHigh=2;}
  if(theory_=="MSSM m_{h}^{mod-} scenario") {extralabel_= "mhmodm-"; model = "mhmodm"; tanbHigh=60; tanbLow=0.5; tanbLowHigh=2;}
  if(theory_=="MSSM m_{h}^{mod+} scenario") {extralabel_= "mhmodp-"; model = "mhmodp"; tanbHigh=60; tanbLow=0.5; tanbLowHigh=2;}
  if(theory_=="MSSM low-m_{H} scenario") {extralabel_= "lowmH-"; model = "lowmH"; tanbHigh=9.5; tanbLow=1.5; tanbLowHigh=2;}
  if(theory_=="MSSM light-stau scenario") {extralabel_= "lightstau1-"; model = "lightstau1"; tanbHigh=60; tanbLow=0.5; tanbLowHigh=3;}
  if(theory_=="MSSM #scale[1.3]{#bf{#tau}}-phobic scenario") {extralabel_= "tauphobic-"; model = "tauphobic"; tanbHigh=50; tanbLow=1.0; tanbLowHigh=2;}
  if(theory_=="MSSM light-stop scenario") {extralabel_= "lightstopmod-"; model = "lightstopmod"; tanbHigh=60; tanbLow=0.7; tanbLowHigh=2;}
 
  // set up styles
  SetStyle();
 
  TGraphAsymmErrors* plain_1=0;
  TGraphAsymmErrors* plain_2=0;
  plain_1 = new TGraphAsymmErrors();
  plain_2 = new TGraphAsymmErrors();

  TGraph* expected_1 = 0;
  TGraphAsymmErrors* innerBand_1 = 0;
  TGraphAsymmErrors* outerBand_1 = 0;
  TGraph* observed_1 = 0;
  expected_1 = new TGraph();
  innerBand_1 = new TGraphAsymmErrors();
  outerBand_1 = new TGraphAsymmErrors();
  observed_1 = new TGraph();

  TGraph* expected_2 = 0;
  TGraphAsymmErrors* innerBand_2 = 0;
  TGraphAsymmErrors* outerBand_2 = 0;
  TGraph* observed_2 = 0;
  expected_2 = new TGraph();
  innerBand_2 = new TGraphAsymmErrors();
  outerBand_2 = new TGraphAsymmErrors();
  observed_2 = new TGraph();

  TGraph* expected_3 = 0;
  TGraphAsymmErrors* innerBand_3 = 0;
  TGraphAsymmErrors* outerBand_3 = 0;
  TGraph* observed_3 = 0;
  expected_3 = new TGraph();
  innerBand_3 = new TGraphAsymmErrors();
  outerBand_3 = new TGraphAsymmErrors();
  observed_3 = new TGraph();
  
  TGraph* injected_1 = 0;
  TGraph* injected_2 = 0;

  if(HIG != ""){
    std::string HIG_save = HIG;
    // observed limit
    if(!expectedOnly_){
      PlotLimits::fillCentral(HIG_save.c_str(), observed_1, HIG_save.append("-obs").c_str()); HIG_save = HIG;
    }
    // expected limit
    PlotLimits::fillCentral(HIG_save.c_str(), expected_1, HIG_save.append("-exp").c_str()); HIG_save = HIG;
    // 1-sigma uncertainty band
    PlotLimits::fillBand(HIG_save.c_str(), innerBand_1, HIG_save.c_str(), true); HIG_save = HIG;
    // 2-sigma uncertainty band
    PlotLimits::fillBand(HIG_save.c_str(), outerBand_1, HIG_save.c_str(), false); HIG_save = HIG;
  }
  else{
  int ipoint_exp=0, ipoint_obs=0;
  for(unsigned int imass=0, ipoint=0; imass<bins_.size(); ++imass){
    // buffer mass value
    float mass = bins_[imass];
    
    //control plots
    TGraph* graph_minus2sigma = new TGraph();
    TGraph* graph_minus1sigma = new TGraph();
    TGraph* graph_expected = new TGraph();
    TGraph* graph_plus1sigma = new TGraph();
    TGraph* graph_plus2sigma = new TGraph();
    TGraph* graph_observed = new TGraph();

    ofstream exclusion;  // saves the exclusion limits within the directory so it can be used to throw toys only in regions near the exclusion limit
    exclusion.open(TString::Format("%s/%d/exclusion_%d.out", directory, (int)mass, (int)mass)); 

    TString fullpath = TString::Format("%s/%d/HypothesisTest.root", directory, (int)mass);
    std::cout << "open file: " << fullpath << std::endl;

    TFile* file_ = TFile::Open(fullpath); if(!file_){ std::cout << "--> TFile is corrupt: skipping masspoint." << std::endl; continue; }
    TTree* limit = (TTree*) file_->Get("tree"); if(!limit){ std::cout << "--> TTree is corrupt: skipping masspoint." << std::endl; continue; }
    double tanb, exp, obs, plus1sigma, minus1sigma, plus2sigma, minus2sigma;
    limit->SetBranchAddress("tanb", &tanb );  
    limit->SetBranchAddress("minus2sigma", &minus2sigma);
    limit->SetBranchAddress("minus1sigma", &minus1sigma);
    limit->SetBranchAddress("expected", &exp);
    limit->SetBranchAddress("plus1sigma", &plus1sigma); 
    limit->SetBranchAddress("plus2sigma", &plus2sigma);  
    limit->SetBranchAddress("observed", &obs);  
    int nevent = limit->GetEntries();   
    //Drawing variable tanb with no graphics option.
    //variable tanb stored in array fV1 (see TTree::Draw)
    limit->Draw("tanb","","goff");
    Int_t *index = new Int_t[nevent];
    //sort array containing tanb in decreasing order
    //The array index contains the entry numbers in decreasing order in respect to tanb
    TMath::Sort(nevent,limit->GetV1(),index);
    std::vector<double> v_minus2sigma;
    std::vector<double> v_minus1sigma;
    std::vector<double> v_exp;
    std::vector<double> v_plus1sigma;
    std::vector<double> v_plus2sigma;
    std::vector<double> v_obs;
    double minus2sigma_a, minus1sigma_a, exp_a, plus1sigma_a, plus2sigma_a, obs_a, tanb_a; //to determine the crosspoints
    double minus2sigma_b, minus1sigma_b, exp_b, plus1sigma_b, plus2sigma_b, obs_b, tanb_b; //to determine the crosspoints
    int np_minus2sigma=0, np_minus1sigma=0, np_exp=0, np_plus1sigma=0, np_plus2sigma=0, np_obs=0; //to count up to 4 points for each. This points are used to create the asymmetric error graphs. Since at some masses there are cases for which scanning from top tanb to bottom tanb leads to exclusion cases like: excluded - not-excluded - excluded - notexcluded. So first point is always on top. Between first and second point there is a excluded region. Between second and third the region is not excluded and between third and fourth the region is once again excluded. If we just have a top exclusion we simple fix the points to tanb=0.5 (depending on the model), so that the graph is not visible. 
    int k=0; double xmax=0; double ymax=0; //stuff needed for fitting;
    // loop to find the crosspoint between low and high exclusion (tanbLowHigh)
    for(int i=0; i<nevent; ++i){
      limit->GetEntry(index[i]);
      //filling control plots
      graph_minus2sigma->SetPoint(k, tanb, minus2sigma/exclusion_);
      graph_minus1sigma->SetPoint(k, tanb, minus1sigma/exclusion_);
      graph_expected   ->SetPoint(k, tanb, exp/exclusion_);
      graph_plus1sigma ->SetPoint(k, tanb, plus1sigma/exclusion_);
      graph_plus2sigma ->SetPoint(k, tanb, plus2sigma/exclusion_);
      graph_observed   ->SetPoint(k, tanb, obs/exclusion_);
      k++;      
      for(int j=0; j<graph_minus2sigma->GetN(); j++){ 
	if(graph_minus2sigma->GetY()[j]>ymax && graph_minus2sigma->GetX()[j]>=1) {ymax=graph_minus2sigma->GetY()[j]; xmax=graph_minus2sigma->GetX()[j]; tanbLowHigh=xmax;} //tanb>=1 hardcoded to fix that point 	
      }
    }
    // main loop to set the find the crosspoints
    //limit->GetEntry(index[nevent-1]); 
    bool highExcluded_minus2sigma=false, lowExcluded_minus2sigma=false; //double last_minus2sigma=minus2sigma;
    bool highExcluded_minus1sigma=false, lowExcluded_minus1sigma=false; //double last_minus1sigma=minus1sigma;
    bool highExcluded_expected=false, lowExcluded_expected=false; //double last_exp=exp; 
    bool highExcluded_plus1sigma=false, lowExcluded_plus1sigma=false; //double last_plus1sigma=plus1sigma;
    bool highExcluded_plus2sigma=false, lowExcluded_plus2sigma=false; //double last_plus2sigma=plus2sigma;
    bool highExcluded_observed=false, lowExcluded_observed=false; //double last_obs=obs;
    for(int i=0; i<nevent; ++i){
      limit->GetEntry(index[i]);
      if (i==0) {
	v_minus2sigma.push_back(tanbHigh);
	if( minus2sigma>exclusion_ ) v_minus2sigma.push_back((minus2sigma+(1-exclusion_))*tanb);
	if((minus2sigma+(1-exclusion_))*tanb>tanbHigh) highExcluded_minus2sigma=true;
	np_minus2sigma++;
	v_minus1sigma.push_back(tanbHigh);
        if( minus1sigma>exclusion_ ) v_minus1sigma.push_back((minus1sigma+(1-exclusion_))*tanb);
	if((minus1sigma+(1-exclusion_))*tanb>tanbHigh) highExcluded_minus1sigma=true;
	np_minus1sigma++;
	v_exp.push_back(tanbHigh);
        if( exp>exclusion_ ) v_exp.push_back((exp+(1-exclusion_))*tanb);
	if((exp+(1-exclusion_))*tanb>tanbHigh) highExcluded_expected=true;
	np_exp++;
	v_plus1sigma.push_back(tanbHigh);
	if( plus1sigma>exclusion_ ) v_plus1sigma.push_back((plus1sigma+(1-exclusion_))*tanb);
	if((plus1sigma+(1-exclusion_))*tanb>tanbHigh) highExcluded_plus1sigma=true;
	np_plus1sigma++;
	v_plus2sigma.push_back(tanbHigh);
	if( plus2sigma>exclusion_ ) v_plus2sigma.push_back((plus2sigma+(1-exclusion_))*tanb);
	if((plus2sigma+(1-exclusion_))*tanb>tanbHigh) highExcluded_plus2sigma=true;
	np_plus2sigma++;
	v_obs.push_back(tanbHigh);
	if( obs>exclusion_ ) v_obs.push_back((obs+(1-exclusion_))*tanb);
	if((obs+(1-exclusion_))*tanb>tanbHigh) highExcluded_observed=true;
	np_obs++;
      }

      minus2sigma_a = minus2sigma;
      minus1sigma_a = minus1sigma;
      exp_a = exp;
      plus1sigma_a = plus1sigma;
      plus2sigma_a = plus2sigma;
      obs_a = obs;
      tanb_a = tanb;
      if(i+1<nevent){
	limit->GetEntry(index[i+1]);
	minus2sigma_b = minus2sigma;
	minus1sigma_b = minus1sigma;
	exp_b = exp;
	plus1sigma_b = plus1sigma;
	plus2sigma_b = plus2sigma;
	obs_b = obs;
	tanb_b = tanb;
      }
      else { // for excluded at last tanb point we need a point at bottom
	minus2sigma_b = 999;
	minus1sigma_b = 999;
	exp_b = 999;
	plus1sigma_b = 999;
	plus2sigma_b = 999;
	obs_b = 999;	
	tanb_b = tanbLow;
      }
      limit->GetEvent(index[i]);
      // -2sigma
      if(minus2sigma_a == exclusion_) {v_minus2sigma.push_back(tanb_a); np_minus2sigma++;}
      if((minus2sigma_a < exclusion_ && minus2sigma_b > exclusion_ && !highExcluded_minus2sigma) || (minus2sigma_a > exclusion_ && minus2sigma_b < exclusion_ && tanb_b<tanbLowHigh)) {	
	if(minus2sigma_a > exclusion_ && minus2sigma_b < exclusion_ && lowExcluded_minus2sigma) {v_minus2sigma.pop_back(); np_minus2sigma--;}
	double x_up=tanb_a;
	double y_up=minus2sigma_a;
	double x_down=tanb_b;
	double y_down=minus2sigma_b;
	v_minus2sigma.push_back( (exclusion_-y_up)/(y_down-y_up)*(x_down-x_up)+x_up ); 	
	//std::cout<< tanb << " -2sigma " << (exclusion_-y_up)/(y_down-y_up)*(x_down-x_up)+x_up << std::endl;  
	np_minus2sigma++;
	if(minus2sigma_a < exclusion_ && minus2sigma_b > exclusion_ ) highExcluded_minus2sigma = true; 
	if(minus2sigma_a > exclusion_ && minus2sigma_b < exclusion_) lowExcluded_minus2sigma = true;
      }
      // -1sigma
      if(minus1sigma_a == exclusion_) {v_minus1sigma.push_back(tanb_a); np_minus1sigma++;}
      if((minus1sigma_a < exclusion_ && minus1sigma_b > exclusion_ && !highExcluded_minus1sigma) || (minus1sigma_a > exclusion_ && minus1sigma_b < exclusion_ && tanb_b<tanbLowHigh)) {
	if(minus1sigma_a > exclusion_ && minus1sigma_b < exclusion_ && lowExcluded_minus1sigma) {v_minus1sigma.pop_back(); np_minus1sigma--;}
	double x_up=tanb_a;
	double y_up=minus1sigma_a;
	double x_down=tanb_b;
	double y_down=minus1sigma_b;
	v_minus1sigma.push_back( (exclusion_-y_up)/(y_down-y_up)*(x_down-x_up)+x_up ); 	
	//std::cout<< tanb << " -1sigma " << (exclusion_-y_up)/(y_down-y_up)*(x_down-x_up)+x_up << std::endl;  
	np_minus1sigma++;
	if(minus1sigma_a < exclusion_ && minus1sigma_b > exclusion_ ) highExcluded_minus1sigma = true; 
	if(minus1sigma_a > exclusion_ && minus1sigma_b < exclusion_) lowExcluded_minus1sigma = true;
      }
      // expected
      if(exp_a == exclusion_) {v_exp.push_back(tanb_a); np_exp++;}
      if((exp_a < exclusion_ && exp_b > exclusion_ && !highExcluded_expected) || (exp_a > exclusion_ && exp_b < exclusion_ && tanb_b<tanbLowHigh)) {
	if(exp_a > exclusion_ && exp_b < exclusion_ && lowExcluded_expected) {v_exp.pop_back(); np_exp--;}
	double x_up=tanb_a;
	double y_up=exp_a;
	double x_down=tanb_b;
	double y_down=exp_b;
	v_exp.push_back( (exclusion_-y_up)/(y_down-y_up)*(x_down-x_up)+x_up ); 	
	//std::cout<< tanb << " expected " << (exclusion_-y_up)/(y_down-y_up)*(x_down-x_up)+x_up << std::endl;  
	np_exp++;
	if(exp_a < exclusion_ && exp_b > exclusion_ ) highExcluded_expected = true; 
	if(exp_a > exclusion_ && exp_b < exclusion_) lowExcluded_expected = true;
      }
      // +1sigma
      if(plus1sigma_a == exclusion_) {v_plus1sigma.push_back(tanb_a); np_plus1sigma++;}
      if((plus1sigma_a < exclusion_ && plus1sigma_b > exclusion_ && !highExcluded_plus1sigma) || (plus1sigma_a > exclusion_ && plus1sigma_b < exclusion_ && tanb_b<tanbLowHigh)) {
	if(plus1sigma_a > exclusion_ && plus1sigma_b < exclusion_ && lowExcluded_plus1sigma) {v_plus1sigma.pop_back(); np_plus1sigma--;}
	double x_up=tanb_a;
	double y_up=plus1sigma_a;
	double x_down=tanb_b;
	double y_down=plus1sigma_b;
	v_plus1sigma.push_back( (exclusion_-y_up)/(y_down-y_up)*(x_down-x_up)+x_up ); 	
	//std::cout<< tanb << " +1sigma " << (exclusion_-y_up)/(y_down-y_up)*(x_down-x_up)+x_up << std::endl;  
	np_plus1sigma++;
	if(plus1sigma_a < exclusion_ && plus1sigma_b > exclusion_ ) highExcluded_plus1sigma = true; 
	if(plus1sigma_a > exclusion_ && plus1sigma_b < exclusion_) lowExcluded_plus1sigma = true;
      }
      // +2sigma
      if(plus2sigma_a == exclusion_) {v_plus2sigma.push_back(tanb_a); np_plus2sigma++;}
      if((plus2sigma_a < exclusion_ && plus2sigma_b > exclusion_ && !highExcluded_plus2sigma) || (plus2sigma_a > exclusion_ && plus2sigma_b < exclusion_ && tanb_b<tanbLowHigh)) {
	if(plus2sigma_a > exclusion_ && plus2sigma_b < exclusion_ && lowExcluded_plus2sigma) {v_plus2sigma.pop_back(); np_plus2sigma--;}
	double x_up=tanb_a;
	double y_up=plus2sigma_a;
	double x_down=tanb_b;
	double y_down=plus2sigma_b;
	v_plus2sigma.push_back( (exclusion_-y_up)/(y_down-y_up)*(x_down-x_up)+x_up ); 	
	//std::cout<< tanb << " +2sigma " << (exclusion_-y_up)/(y_down-y_up)*(x_down-x_up)+x_up << std::endl;  
	np_plus2sigma++;
	if(plus2sigma_a < exclusion_ && plus2sigma_b > exclusion_ ) highExcluded_plus2sigma = true; 
	if(plus2sigma_a > exclusion_ && plus2sigma_b < exclusion_) lowExcluded_plus2sigma = true;
      }
      // observed
      if(obs_a == exclusion_) {v_obs.push_back(tanb_a); np_obs++;}
      if((obs_a < exclusion_ && obs_b > exclusion_ && !highExcluded_observed) || (obs_a > exclusion_ && obs_b < exclusion_ && tanb_b<tanbLowHigh)) {
	if(obs_a > exclusion_ && obs_b < exclusion_ && lowExcluded_observed) {v_obs.pop_back(); np_obs--;}
	double x_up=tanb_a;
	double y_up=obs_a;
	double x_down=tanb_b;
	double y_down=obs_b;
	v_obs.push_back( (exclusion_-y_up)/(y_down-y_up)*(x_down-x_up)+x_up ); 	
	//std::cout<< tanb << " observed " << (exclusion_-y_up)/(y_down-y_up)*(x_down-x_up)+x_up << std::endl;  
	np_obs++;
	if(obs_a < exclusion_ && obs_b > exclusion_ ) highExcluded_observed = true; 
	if(obs_a > exclusion_ && obs_b < exclusion_) lowExcluded_observed = true;
      }
    }
    //check if last element is excluded or not - if not excluded be conservative and drop low exclusion - this is not yet implemented (region in interest is already excluded by mh)
    //if(lowExcluded_minus2sigma && last_minus2sigma>exclusion_) {v_minus2sigma.pop_back(); np_minus2sigma--;}
    //number of points must be 4
    while(np_minus2sigma<4){
      if(v_minus2sigma[1]<tanbLowHigh) {
	v_minus2sigma.insert(v_minus2sigma.begin()+1, 2, tanbLowHigh );
	np_minus2sigma++;
	np_minus2sigma++;
      }
      else{
	v_minus2sigma.push_back( tanbLow );
	np_minus2sigma++;
      }
    }    
    while(np_minus1sigma<4){
      if(v_minus1sigma[1]<tanbLowHigh) {
	v_minus1sigma.insert(v_minus1sigma.begin()+1, 2, tanbLowHigh );
	np_minus1sigma++;
	np_minus1sigma++;
      }
      else{
	v_minus1sigma.push_back( tanbLow );
	np_minus1sigma++;
      }
    }    
    while(np_exp<4){
      v_exp.push_back( tanbLow );
      np_exp++;
    }    
    while(np_plus1sigma<4){
      if(v_plus1sigma[1]<tanbLowHigh) {
	v_plus1sigma.insert(v_plus1sigma.begin()+1, 2, tanbLowHigh );
	np_plus1sigma++;
	np_plus1sigma++;
      }
      else{
	v_plus1sigma.push_back( tanbLow );
	np_plus1sigma++;
      }
    } 
    while(np_plus2sigma<4){
      if(v_plus2sigma[1]<tanbLowHigh) {
	v_plus2sigma.insert(v_plus2sigma.begin()+1, 2, tanbLowHigh );
	np_plus2sigma++;
	np_plus2sigma++;
      }
      else{
	v_plus2sigma.push_back( tanbLow );
	np_plus2sigma++;
      }
    } 
    while(np_obs<4){
      v_obs.push_back( tanbLow );
      np_obs++;
    }
    
    /*for(unsigned int i=0; i<v_minus2sigma.size(); i++){
      std::cout << i << " -2sigma " << v_minus2sigma[i] <<std::endl;
    }
    for(unsigned int i=0; i<v_minus1sigma.size(); i++){
      std::cout << i << " -1sigma " << v_minus1sigma[i] <<std::endl;
    }
    for(unsigned int i=0; i<v_exp.size(); i++){
      std::cout << i << " exp " << v_exp[i] <<std::endl;
    }    
    for(unsigned int i=0; i<v_plus1sigma.size(); i++){
      std::cout << i << " +1sigma " << v_plus1sigma[i] <<std::endl;
    }  
    for(unsigned int i=0; i<v_plus2sigma.size(); i++){
      std::cout << i << " +2sigma " << v_plus2sigma[i] <<std::endl;
    }  
    for(unsigned int i=0; i<v_obs.size(); i++){
      std::cout << i << " obs " << v_obs[i] <<std::endl;
      }*/

    //control plots showing the CLs value over tanb for each mass
    //minus2sigma
    TCanvas* canv_minus2sigma = new TCanvas(TString::Format("tanb-CLs_025_%d", (int)mass), "", 600, 600);
    canv_minus2sigma->cd();
    canv_minus2sigma->SetGridx(1);
    canv_minus2sigma->SetGridy(1);
    graph_minus2sigma->GetXaxis()->SetTitle("tan#beta");
    graph_minus2sigma->GetXaxis()->SetLabelFont(62);
    graph_minus2sigma->GetXaxis()->SetTitleFont(62);
    graph_minus2sigma->GetXaxis()->SetTitleColor(1);
    graph_minus2sigma->GetXaxis()->SetTitleOffset(1.05);
    graph_minus2sigma->GetYaxis()->SetTitle("CL_{s}");
    graph_minus2sigma->GetYaxis()->SetLabelFont(62);
    graph_minus2sigma->GetYaxis()->SetTitleFont(62);
    graph_minus2sigma->GetYaxis()->SetTitleOffset(1.05);
    graph_minus2sigma->GetYaxis()->SetLabelSize(0.03);
    graph_minus2sigma->SetMarkerStyle(20);
    graph_minus2sigma->SetMarkerSize(1.3);
    ymax=0; ymax=0;
    for(int j=0; j<graph_minus2sigma->GetN(); j++){ 
      if(graph_minus2sigma->GetY()[j]>ymax) {ymax=graph_minus2sigma->GetY()[j]; xmax=graph_minus2sigma->GetX()[j];}
    }
    //TF1 *f1_minus2sigma = new TF1(TString::Format("fit_025_%d", (int)mass), "[0]*exp([1]+x*[2])", xmax, graph_minus2sigma->GetX()[0]);
    //TFitResultPtr r_minus2sigma = graph_minus2sigma->Fit(f1_minus2sigma, "SQR");
    //if (f1_minus2sigma->GetX(0.05, xmax, 200>=xmax) v_minus2sigma[1]=f1_minus2sigma->GetX(0.05, xmax, 200;
    graph_minus2sigma->Draw("AP");
    canv_minus2sigma->Print(TString::Format("%s/%d/tanb-CLs_025_%dGeV.png", directory, (int)mass, (int)mass));
    //minus1sigma
    TCanvas* canv_minus1sigma = new TCanvas(TString::Format("tanb-CLs_160_%d", (int)mass), "", 600, 600);
    canv_minus1sigma->cd();
    canv_minus1sigma->SetGridx(1);
    canv_minus1sigma->SetGridy(1);
    graph_minus1sigma->GetXaxis()->SetTitle("tan#beta");
    graph_minus1sigma->GetXaxis()->SetLabelFont(62);
    graph_minus1sigma->GetXaxis()->SetTitleFont(62);
    graph_minus1sigma->GetXaxis()->SetTitleColor(1);
    graph_minus1sigma->GetXaxis()->SetTitleOffset(1.05);
    graph_minus1sigma->GetYaxis()->SetTitle("CL_{s}");
    graph_minus1sigma->GetYaxis()->SetLabelFont(62);
    graph_minus1sigma->GetYaxis()->SetTitleFont(62);
    graph_minus1sigma->GetYaxis()->SetTitleOffset(1.05);
    graph_minus1sigma->GetYaxis()->SetLabelSize(0.03);
    graph_minus1sigma->SetMarkerStyle(20);
    graph_minus1sigma->SetMarkerSize(1.3);
    ymax=0; ymax=0;
    for(int j=0; j<graph_minus1sigma->GetN(); j++){ 
      if(graph_minus1sigma->GetY()[j]>ymax) {ymax=graph_minus1sigma->GetY()[j]; xmax=graph_minus1sigma->GetX()[j];}
    }
    //TF1 *f1_minus1sigma = new TF1(TString::Format("fit_025_%d", (int)mass), "[0]*exp([1]+x*[2])", xmax, graph_minus1sigma->GetX()[0]);
    //TFitResultPtr r_minus1sigma = graph_minus1sigma->Fit(f1_minus1sigma, "SQR");
    //if (f1_minus1sigma->GetX(0.05, xmax, 200)>=xmax) v_minus1sigma[1]=f1_minus1sigma->GetX(0.05, xmax, 200);
    graph_minus1sigma->Draw("AP");
    canv_minus1sigma->Print(TString::Format("%s/%d/tanb-CLs_160_%dGeV.png", directory, (int)mass, (int)mass));
    //expected
    TCanvas* canv_expected = new TCanvas(TString::Format("tanb-CLs_EXP_%d", (int)mass), "", 600, 600);
    canv_expected->cd();
    canv_expected->SetGridx(1);
    canv_expected->SetGridy(1);
    graph_expected->GetXaxis()->SetTitle("tan#beta");
    graph_expected->GetXaxis()->SetLabelFont(62);
    graph_expected->GetXaxis()->SetTitleFont(62);
    graph_expected->GetXaxis()->SetTitleColor(1);
    graph_expected->GetXaxis()->SetTitleOffset(1.05);
    graph_expected->GetYaxis()->SetTitle("CL_{s}");
    graph_expected->GetYaxis()->SetLabelFont(62);
    graph_expected->GetYaxis()->SetTitleFont(62);
    graph_expected->GetYaxis()->SetTitleOffset(1.05);
    graph_expected->GetYaxis()->SetLabelSize(0.03);
    graph_expected->SetMarkerStyle(20);
    graph_expected->SetMarkerSize(1.3);
    ymax=0; ymax=0;
    for(int j=0; j<graph_expected->GetN(); j++){ 
      if(graph_expected->GetY()[j]>ymax) {ymax=graph_expected->GetY()[j]; xmax=graph_expected->GetX()[j];}
    }
    //TF1 *f1_expected = new TF1(TString::Format("fit_025_%d", (int)mass), "[0]*exp([1]+x*[2])", xmax, graph_expected->GetX()[0]);
    //TFitResultPtr r_expected = graph_expected->Fit(f1_expected, "SQR");
    //if (f1_expected->GetX(0.05, xmax, 200)>=xmax) v_exp[1]=f1_expected->GetX(0.05, xmax, 200);
    graph_expected->Draw("AP");
    canv_expected->Print(TString::Format("%s/%d/tanb-CLs_EXP_%dGeV.png", directory, (int)mass, (int)mass));
    //plus1sigma
    TCanvas* canv_plus1sigma = new TCanvas(TString::Format("tanb-CLs_860_%d", (int)mass), "", 600, 600);
    canv_plus1sigma->cd();
    canv_plus1sigma->SetGridx(1);
    canv_plus1sigma->SetGridy(1);
    graph_plus1sigma->GetXaxis()->SetTitle("tan#beta");
    graph_plus1sigma->GetXaxis()->SetLabelFont(62);
    graph_plus1sigma->GetXaxis()->SetTitleFont(62);
    graph_plus1sigma->GetXaxis()->SetTitleColor(1);
    graph_plus1sigma->GetXaxis()->SetTitleOffset(1.05);
    graph_plus1sigma->GetYaxis()->SetTitle("CL_{s}");
    graph_plus1sigma->GetYaxis()->SetLabelFont(62);
    graph_plus1sigma->GetYaxis()->SetTitleFont(62);
    graph_plus1sigma->GetYaxis()->SetTitleOffset(1.05);
    graph_plus1sigma->GetYaxis()->SetLabelSize(0.03);
    graph_plus1sigma->SetMarkerStyle(20);
    graph_plus1sigma->SetMarkerSize(1.3);
    ymax=0; ymax=0;
    for(int j=0; j<graph_plus1sigma->GetN(); j++){ 
      if(graph_plus1sigma->GetY()[j]>ymax) {ymax=graph_plus1sigma->GetY()[j]; xmax=graph_plus1sigma->GetX()[j];}
    }
    //TF1 *f1_plus1sigma = new TF1(TString::Format("fit_025_%d", (int)mass), "[0]*exp([1]+x*[2])", xmax, graph_plus1sigma->GetX()[0]);
    //TFitResultPtr r_plus1sigma = graph_plus1sigma->Fit(f1_plus1sigma, "SQR");
    //if (f1_plus1sigma->GetX(0.05, xmax, 200)>=xmax) v_plus1sigma[1]=f1_plus1sigma->GetX(0.05, xmax, 200);
    graph_plus1sigma->Draw("AP");
    canv_plus1sigma->Print(TString::Format("%s/%d/tanb-CLs_860_%dGeV.png", directory, (int)mass, (int)mass));
    //plus2sigma
    TCanvas* canv_plus2sigma = new TCanvas(TString::Format("tanb-CLs_975_%d", (int)mass), "", 600, 600);
    canv_plus2sigma->cd();
    canv_plus2sigma->SetGridx(1);
    canv_plus2sigma->SetGridy(1);
    graph_plus2sigma->GetXaxis()->SetTitle("tan#beta");
    graph_plus2sigma->GetXaxis()->SetLabelFont(62);
    graph_plus2sigma->GetXaxis()->SetTitleFont(62);
    graph_plus2sigma->GetXaxis()->SetTitleColor(1);
    graph_plus2sigma->GetXaxis()->SetTitleOffset(1.05);
    graph_plus2sigma->GetYaxis()->SetTitle("CL_{s}");
    graph_plus2sigma->GetYaxis()->SetLabelFont(62);
    graph_plus2sigma->GetYaxis()->SetTitleFont(62);
    graph_plus2sigma->GetYaxis()->SetTitleOffset(1.05);
    graph_plus2sigma->GetYaxis()->SetLabelSize(0.03);
    graph_plus2sigma->SetMarkerStyle(20);
    graph_plus2sigma->SetMarkerSize(1.3);
    ymax=0; ymax=0;
    for(int j=0; j<graph_plus2sigma->GetN(); j++){ 
      if(graph_plus2sigma->GetY()[j]>ymax) {ymax=graph_plus2sigma->GetY()[j]; xmax=graph_plus2sigma->GetX()[j];}
    }
    //TF1 *f1_plus2sigma = new TF1(TString::Format("fit_025_%d", (int)mass), "[0]*exp([1]+x*[2])", xmax, graph_plus2sigma->GetX()[0]);
    //TFitResultPtr r_plus2sigma = graph_plus2sigma->Fit(f1_plus2sigma, "SQR");
    //if (f1_plus2sigma->GetX(0.05, xmax, 200)>=xmax) v_plus2sigma[1]=f1_plus2sigma->GetX(0.05, xmax, 200);
    graph_plus2sigma->Draw("AP");
    canv_plus2sigma->Print(TString::Format("%s/%d/tanb-CLs_975_%dGeV.png", directory, (int)mass, (int)mass));
    //observed
    TCanvas* canv_observed = new TCanvas(TString::Format("tanb-CLs_OBS_%d", (int)mass), "", 600, 600);
    canv_observed->cd();
    canv_observed->SetGridx(1);
    canv_observed->SetGridy(1);
    graph_observed->GetXaxis()->SetTitle("tan#beta");
    graph_observed->GetXaxis()->SetLabelFont(62);
    graph_observed->GetXaxis()->SetTitleFont(62);
    graph_observed->GetXaxis()->SetTitleColor(1);
    graph_observed->GetXaxis()->SetTitleOffset(1.05);
    graph_observed->GetYaxis()->SetTitle("CL_{s}");
    graph_observed->GetYaxis()->SetLabelFont(62);
    graph_observed->GetYaxis()->SetTitleFont(62);
    graph_observed->GetYaxis()->SetTitleOffset(1.05);
    graph_observed->GetYaxis()->SetLabelSize(0.03);
    graph_observed->SetMarkerStyle(20);
    graph_observed->SetMarkerSize(1.3);
    ymax=0; ymax=0;
    for(int j=0; j<graph_observed->GetN(); j++){ 
      if(graph_observed->GetY()[j]>ymax) {ymax=graph_observed->GetY()[j]; xmax=graph_observed->GetX()[j];}
    }
    //TF1 *f1_observed = new TF1(TString::Format("fit_025_%d", (int)mass), "[0]*exp([1]+x*[2])", xmax, graph_observed->GetX()[0]);
    //TFitResultPtr r_observed = graph_observed->Fit(f1_observed, "SQR");
    //if (f1_observed->GetX(0.05, xmax, 200)>=xmax) v_obs[1]=f1_observed->GetX(0.05, xmax, 200);
    graph_observed->Draw("AP");
    canv_observed->Print(TString::Format("%s/%d/tanb-CLs_OBS_%dGeV.png", directory, (int)mass, (int)mass));
    
    //fill the graphs 
    //expected - excluded region is by definition between cross point 1 and 2 and between 3 and 4
    expected_1->SetPoint(ipoint, mass, v_exp[1]);    
    if(np_exp>2){     
      expected_2->SetPoint(ipoint_exp, mass, v_exp[2]); 
      expected_3->SetPoint(ipoint_exp, mass, v_exp[3]);    
      ipoint_exp++;
    }
    //observed - excluded region is by definition between cross point 1 and 2 and between 3 and 4
    observed_1->SetPoint(ipoint, mass, v_obs[1]);  
    plain_1->SetPoint(ipoint, mass, v_obs[1]);
    plain_1->SetPointEYhigh(ipoint, v_obs[0]-v_obs[1]);
    plain_1->SetPointEYlow (ipoint, 0); 
    if(np_obs>2){
      observed_2->SetPoint(ipoint_obs, mass, v_obs[2]);
      observed_3->SetPoint(ipoint_obs, mass, v_obs[3]);        
      plain_2->SetPoint(ipoint_obs, mass, v_obs[3]);
      plain_2->SetPointEYhigh(ipoint_obs, v_obs[2]-v_obs[3]);
      plain_2->SetPointEYlow (ipoint_obs, 0); 
      ipoint_obs++;
    }
    //innerBand - around expected line at crosspoint 2, 3 and 4 we need the band => 3 points
    innerBand_1->SetPoint(ipoint, mass, v_exp[1]);
    innerBand_1->SetPointEYhigh(ipoint, v_plus1sigma[1]-v_exp[1]);
    innerBand_1->SetPointEYlow (ipoint, v_exp[1]-v_minus1sigma[1]); 
    innerBand_2->SetPoint(ipoint, mass, v_exp[2]);
    innerBand_2->SetPointEYhigh(ipoint, v_plus1sigma[2]-v_exp[2]);
    innerBand_2->SetPointEYlow (ipoint, v_exp[2]-v_minus1sigma[2]); 
    //outerBand - around expected line at crosspoint 2, 3 and 4 we need the band => 3 points
    outerBand_1->SetPoint(ipoint, mass, v_exp[1]);
    outerBand_1->SetPointEYhigh(ipoint, v_plus2sigma[1]-v_exp[1]);
    outerBand_1->SetPointEYlow (ipoint, v_exp[1]-v_minus2sigma[1]); 
    outerBand_2->SetPoint(ipoint, mass, v_exp[2]);
    outerBand_2->SetPointEYhigh(ipoint, v_plus2sigma[2]-v_exp[2]);
    outerBand_2->SetPointEYlow (ipoint, v_exp[2]-v_minus2sigma[2]);
 
    exclusion << int(mass) << " " << v_minus2sigma[1] << " " << v_minus1sigma[1] << " " << v_exp[1] << " " << v_plus1sigma[1] << " " << v_plus2sigma[1] << " " << v_obs[1] << std::endl; 
    double help_minus2sigma, help_minus1sigma, help_exp, help_plus1sigma, help_plus2sigma, help_obs;
    if (v_minus2sigma[2]<tanbLowHigh && v_minus2sigma[2]>tanbLow) help_minus2sigma=v_minus2sigma[2];
    else if (v_minus2sigma[2]==tanbLowHigh && tanbLowHigh!=tanbLow) help_minus2sigma=v_minus2sigma[3];
    else help_minus2sigma=v_minus2sigma[1]; 
    if (v_minus1sigma[2]<tanbLowHigh && v_minus1sigma[2]>tanbLow) help_minus1sigma=v_minus1sigma[2];
    else if (v_minus1sigma[2]==tanbLowHigh && tanbLowHigh!=tanbLow) help_minus1sigma=v_minus1sigma[3];
    else help_minus1sigma=v_minus1sigma[1];
    if (v_exp[2]!=tanbLow) help_exp=v_exp[2];
    else help_exp=v_exp[1];
    if (v_plus1sigma[2]<tanbLowHigh && v_plus1sigma[2]>tanbLow) help_plus1sigma=v_plus1sigma[2];
    else if (v_plus1sigma[2]==tanbLowHigh && tanbLowHigh!=tanbLow) help_plus1sigma=v_plus1sigma[3];
    else help_plus1sigma=v_plus1sigma[1];
    if (v_plus2sigma[2]<tanbLowHigh && v_plus2sigma[2]>tanbLow) help_plus2sigma=v_plus2sigma[2];
    else if (v_plus2sigma[2]==tanbLowHigh && tanbLowHigh!=tanbLow) help_plus2sigma=v_plus2sigma[3];
    else help_plus2sigma=v_plus2sigma[1];
    if (v_obs[2]!=tanbLow) help_obs=v_obs[2];
    else help_obs=v_obs[1];
    exclusion << int(mass) << " " << help_minus2sigma << " " << help_minus1sigma << " " << help_exp << " " << help_plus1sigma << " " << help_plus2sigma << " " << help_obs << std::endl; 
    exclusion.close();

    ipoint++;
  }
  }
  
   // create plots for additional comparisons
  std::map<std::string, TGraph*> comparisons; TGraph* comp=0;
  if(arXiv_1211_6956_){ comp = new TGraph(), arXiv_1211_6956 (comp); comp->SetName("arXiv_1211_6956" ); comparisons[std::string("ATLAS H#rightarrow#tau#tau (4.8/fb)")] = comp;}
  if(arXiv_1204_2760_){ comp = new TGraph(); arXiv_1204_2760 (comp); comp->SetName("arXiv_1204_2760" ); comparisons[std::string("ATLAS H^{+} (4.6/fb)")               ] = comp;}
  //if(arXiv_1302_2892_){ comp = new TGraph(); arXiv_1302_2892 (comp); comp->SetName("arXiv_1302_2892" ); comparisons[std::string("CMS bbH#rightarrow 3b (4.8/fb)")     ] = comp;}
  if(arXiv_1302_2892_){ comp = new TGraph(); arXiv_1302_2892 (comp); comp->SetName("arXiv_1302_2892" ); comparisons[std::string("CMS #phi#rightarrowb#bar{b}")     ] = comp;}
  if(arXiv_1205_5736_){ comp = new TGraph(); arXiv_1205_5736 (comp); comp->SetName("arXiv_1205_5736" ); comparisons[std::string("CMS H^{+} (2/fb)")                   ] = comp;}
  if(HIG_12_052_     ){ comp = new TGraph(); HIG_12_052_lower(comp); comp->SetName("HIG_12_052_lower"); comparisons[std::string("CMS H^{+} (2-4.9/fb)")               ] = comp;}
  if(HIG_12_052_     ){ comp = new TGraph(); HIG_12_052_upper(comp); comp->SetName("HIG_12_052_upper"); comparisons[std::string("EMPTY")                              ] = comp;}

  // setup contratins from Higgs mass
  std::map<double, TGraphAsymmErrors*> higgsBands;
  if(higgs125_){
    higgsBands[3] = higgsConstraint(directory, 125., 3., model);
    //higgsBands[2] = higgsConstraint(directory, 125., 2., model);
    //higgsBands[1] = higgsConstraint(directory, 125., 1., model);
    //for(unsigned int deltaM=0; deltaM<3; ++deltaM){
    //  higgsBands[3-deltaM] = higgsConstraint(directory, 125., 4-deltaM, model);
    //}
  }  
  
  // do the plotting
  plottingTanb(canv, plain_1, plain_2, innerBand_1, innerBand_2, innerBand_3, outerBand_1, outerBand_2, outerBand_3, expected_1, expected_2, expected_3, observed_1, observed_2, observed_3, injected_1, injected_2, higgsBands, comparisons, xaxis_, yaxis_, theory_, min_, max_, log_, transparent_, expectedOnly_, outerband_, MSSMvsSM_, HIG, BlackWhite_); 
  /// setup the CMS Preliminary
  //CMSPrelim(dataset_.c_str(), "", 0.145, 0.835);
  //TPaveText* cmsprel = new TPaveText(0.145, 0.835+0.06, 0.145+0.30, 0.835+0.16, "NDC");
  TPaveText* cmsprel = new TPaveText(0.135, 0.835+0.06, 0.145+0.30, 0.835+0.16, "NDC"); // for "unpublished" in header
  cmsprel->SetBorderSize(   0 );
  cmsprel->SetFillStyle(    0 );
  cmsprel->SetTextAlign(   12 );
  cmsprel->SetTextSize ( 0.03 );
  cmsprel->SetTextColor(    1 );
  cmsprel->SetTextFont (   62 );
  cmsprel->AddText(dataset_.c_str());
  cmsprel->Draw();
  
  // write results to files
  if(png_){
    canv.Print(std::string(output_).append("_").append(extralabel_).append(label_).append(".png").c_str());
  }
  if(pdf_){
    canv.Print(std::string(output_).append("_").append(extralabel_).append(label_).append(".pdf").c_str());
    canv.Print(std::string(output_).append("_").append(extralabel_).append(label_).append(".eps").c_str());
  }
  if(txt_){
    print(std::string(output_).append("_").append(extralabel_).append(label_).c_str(), outerBand_1, innerBand_1, expected_1, observed_1, outerBand_2, innerBand_2, expected_2, observed_2, tanbLow, tanbHigh, tanbLowHigh, "txt");
    print(std::string(output_).append("_").append(extralabel_).append(label_).c_str(), outerBand_1, innerBand_1, expected_1, observed_1, outerBand_2, innerBand_2, expected_2, observed_2, tanbLow, tanbHigh, tanbLowHigh, "tex");
  }
  if(root_){
    TFile* output = new TFile(std::string("limits_").append(extralabel_).append(label_).append(".root").c_str(), "update");
    if(!output->cd(output_.c_str())){
      output->mkdir(output_.c_str());
      output->cd(output_.c_str());
    }
    if(observed_1){ 
      observed_1 ->Write("observed_1" );
      observed_2 ->Write("observed_2" );
      observed_3 ->Write("observed_3" );
      plain_1 ->Write("plain_1" );
      plain_2 ->Write("plain_2" );
    }
    expected_1 ->Write("expected_1" );
    expected_2 ->Write("expected_2" );
    expected_3 ->Write("expected_3" );
    innerBand_1->Write("innerBand_1");
    innerBand_2->Write("innerBand_2");
    innerBand_3->Write("innerBand_3");
    outerBand_1->Write("outerBand_1");
    outerBand_2->Write("outerBand_2");
    outerBand_3->Write("outerBand_3");
    output->Close();
  }
  return;
}
