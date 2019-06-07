void pl_TimingQa(){
  //  TCanvas *can = new TCanvas("can22","can22");
  //  can->Divide(2,2); 

   TFile* fHistFile = new TFile( "StETofQA.hst.root","READ");

	gStyle->SetPalette(1);
	gStyle->SetOptStat(kTRUE);
  	gStyle->SetPadGridX(kFALSE);
  	gStyle->SetPadGridY(kFALSE);
	gStyle->SetOptStat(1111111);

  TCanvas *can = new TCanvas("can","can",50,0,800,800);
  can->Divide(2,3); 

	gPad->SetFillColor(0);


	 TH1 *h;
	 TH1 *h1;
	 TH2 *h2;

 TString hname;

 // if (h!=NULL) h->Delete();

can->cd(1);
 fHistFile->cd();
 hname=Form("Av_TDiff_ETof_BTof");
 h1=(TH1 *)fHistFile->FindObjectAny(hname);
 if (h1!=NULL) {
  h1->Draw("colz");
  gPad->SetLogy();
 }else { cout << hname << " not found -> return" << endl; return;}

can->cd(2);
 fHistFile->cd();
 hname=Form("Hit_TDiff_ETof_Trg");
 h1=(TH1 *)fHistFile->FindObjectAny(hname);
 if (h1!=NULL) {
  h1->Draw("colz");
  gPad->SetLogy();
 }else { cout << hname << " not found -> return" << endl; return;}

can->cd(3);
 fHistFile->cd();
 hname=Form("Digi_TDiff_ETof_Trg");
 h1=(TH1 *)fHistFile->FindObjectAny(hname);
 if (h1!=NULL) {
  h1->Draw("colz");
  gPad->SetLogy();
 }else { cout << hname << " not found -> return" << endl; return;}

can->cd(4);
 fHistFile->cd();
 hname=Form("DigiRaw_TDiff_ETof_Trg");
 h1=(TH1 *)fHistFile->FindObjectAny(hname);
 if (h1!=NULL) {
  h1->Draw("colz");
  gPad->SetLogy();
 }else { cout << hname << " not found -> return" << endl; return;}


can->cd(5);
 fHistFile->cd();
 hname=Form("Mul_ETof_BTof");
 h2=(TH2 *)fHistFile->FindObjectAny(hname);
 if (h2!=NULL) {
  h2->Draw("colz");
  gPad->SetLogz();
 }else { cout << hname << " not found -> return" << endl; return;}

can->cd(6);
 fHistFile->cd();
 hname=Form("Av_TDiff_v_Mul_ETof");
 h2=(TH2 *)fHistFile->FindObjectAny(hname);
 if (h2!=NULL) {
  h2->Draw("colz");
  gPad->SetLogz();
 }else { cout << hname << " not found -> return" << endl; return;}




 can->SaveAs(Form("pl_TimeQA.pdf"));
}
