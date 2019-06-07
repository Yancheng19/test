void pl_HitQa(Int_t iSector=18, Int_t iPlane=3, Int_t iDet=3) {

   TFile* fHistFile = new TFile( "StETofQA.hst.root","READ");
	gStyle->SetPalette(1);
	gStyle->SetOptStat(kTRUE);
  	gStyle->SetPadGridX(kFALSE);
  	gStyle->SetPadGridY(kFALSE);
	gStyle->SetOptStat(1111111);

  TCanvas *can = new TCanvas("can","can",50,0,800,800);
  can->Divide(2,4); 

	gPad->SetFillColor(0);


	 TH1 *h;
	 TH1 *h1;
	 TH2 *h2;

 TString hname;

 // if (h!=NULL) h->Delete();

can->cd(1);
 fHistFile->cd();
 hname=Form("cl_Sector%02d_ZPlane%d_Det%d_Size", iSector, iPlane, iDet );
 h2=(TH2 *)fHistFile->FindObjectAny(hname);
 if (h2!=NULL) {
  h2->Draw("colz");
  h2->ProfileX()->Draw("same");
  gPad->SetLogz();
 }else { cout << hname << " not found -> return" << endl;}
 
 can->cd(2);
fHistFile->cd();
 //fHistFile->cd("/ROOT Files/StETofQA.hst.root"); 
 hname=Form("cl_Sector%02d_ZPlane%d_Det%d_Pos", iSector, iPlane, iDet);
 h2=(TH2 *)fHistFile->FindObjectAny(hname);
 if (h2!=NULL) {
  h2->Draw("colz");
  h2->ProfileX()->Draw("same");
  gPad->SetLogz();
 }else { cout << hname << " not found -> return" << endl;}

 can->cd(3);
fHistFile->cd();
 //fHistFile->cd("StETofQA.hst.root"); 
 hname=Form("cl_Sector%02d_ZPlane%d_Det%d_TotDigi", iSector, iPlane, iDet );
 h2=(TH2 *)fHistFile->FindObjectAny(hname);
 if (h2!=NULL) {
  h2->Draw("colz");
  h2->ProfileX()->Draw("same");
  gPad->SetLogz();
 }else { cout << hname << " not found -> return" << endl;}

 can->cd(4);
fHistFile->cd();
 //fHistFile->cd("StETofQA.hst.root"); 
 hname=Form("cl_Sector%02d_ZPlane%d_Det%d_Mul", iSector, iPlane, iDet );
 h1=(TH1 *)fHistFile->FindObjectAny(hname);
 if (h1!=NULL) {
  h1->Draw("colz");
  gPad->SetLogy();
 }else { cout << hname << " not found -> return" << endl;}

 can->cd(5);
fHistFile->cd();
 //fHistFile->cd("StETofQA.hst.root"); 
 hname=Form("cl_Sector%02d_ZPlane%d_Det%d_AvWalk", iSector, iPlane, iDet );
 h2=(TH2 *)fHistFile->FindObjectAny(hname);
 if (h2!=NULL) {
  h2->Draw("colz");
  h2->ProfileX()->Draw("same");
  gPad->SetLogz();
 }else { cout << hname << " not found -> return" << endl; }

 can->cd(6);
fHistFile->cd();
 //fHistFile->cd("StETofQA.hst.root"); 
 hname=Form("cl_Sector%02d_ZPlane%d_Det%d_AvCluWalk", iSector, iPlane, iDet );
 h2=(TH2 *)fHistFile->FindObjectAny(hname);
 if (h2!=NULL) {
  h2->Draw("colz");
  h2->ProfileX()->Draw("same");
  gPad->SetLogz();
 }else { cout << hname << " not found -> return" << endl; }

 can->cd(7);
fHistFile->cd();
 //fHistFile->cd("StETofQA.hst.root");
 hname=Form("cl_Sector%02d_ZPlane%d_Det%d_TOff", iSector, iPlane, iDet );
 h2=(TH2 *)fHistFile->FindObjectAny(hname);
 if (h2!=NULL) {
  h2->Draw("colz");
  h2->ProfileX()->Draw("same");
  gPad->SetLogz();
 }else { cout << hname << " not found -> return" << endl;}

 can->cd(8);
fHistFile->cd();
 //fHistFile->cd("StETofQA.hst.root");
 hname=Form("G_matched_t0corr_mom_s%dm%dc%d", iSector, iPlane, iDet );
 h2=(TH2 *)fHistFile->FindObjectAny(hname);
 if (h2!=NULL) {
  //h2->ProjectionY("_temp_proj", 20, -1 )->Draw("colz");
  h2->Draw("colz");
 // gPad->SetLogy();
 }else { cout << hname << " not found -> return" << endl;}



 can->SaveAs(Form("pl_HitQA_%01d_%01d_%01d.pdf",iSector,iPlane,iDet));
}
