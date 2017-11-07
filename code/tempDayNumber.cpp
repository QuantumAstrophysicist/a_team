#include <iostream>
#include "tempTrender.h"
#include <sstream> // maybe include in tempTrender.h instead
// ROOT library objects
#include <TF1.h> // 1d function class
#include <TH1.h> // 1d histogram classes
#include <TStyle.h>  // style object
#include <TMath.h>   // math functions
#include <TCanvas.h> // canvas object
#include <TLegend.h>
// Adding the possibility to choose time of day?
// Leap years?

double Gaussian3(double* x, double* par) { //A custom function
    return par[0]*exp(-0.5*(x[0]*x[0] - 2*x[0]*par[1] + par[1]*par[1])/(par[2]*par[2]));
}

void tempTrender::tempOnDayNumber(int dateToCalculate){
    cout << endl;
    cout << "Calculating the temperature for a certain day in Uppsala" << endl;
    
    vector <vector <string> > dataOnDay;
    //readData("smhi-openda_Karlstad.csv", dataOnDay);
    readData("uppsala_tm_1722-2013.dat", dataOnDay);
    
    
    vector <float> tempCalculatedDay; // Vector to store temperatures for the chosen day
    // Create histogram
    TH1F* hist = new TH1F("hist", "Temperature;Temperature [#circC];Entries", 300, -20, 40);

    // Get date (dayToCalculate and monthToCalculate) from the input day number
    int dayToCalculate;
    int monthToCalculate = 0;
    
    //int months[13] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}; // For leapyears if day 60
    int months[13] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
    
    
    while (months[monthToCalculate] < dateToCalculate) {
        monthToCalculate++;
    }
    dayToCalculate = dateToCalculate - months[monthToCalculate - 1];
    
  
    // Find the right day, month and time in the data
    for (int i = 1 ; i < (int)dataOnDay.size(); i++ ) {
        if ( (stoi(dataOnDay.at(i).at(1)) == monthToCalculate) && (stoi(dataOnDay.at(i).at(2)) == dayToCalculate) ){ //&&(dataOnDay.at(i).at(3) == "12") ) {
            tempCalculatedDay.push_back(strtof((dataOnDay.at(i).at(6)).c_str(),0)); // Vector (float) with temperatures for chosen day
        }
    }
    
    // Fill the histogram
    for(Int_t i = 0; i < (int)tempCalculatedDay.size(); i++) {
        hist->Fill(tempCalculatedDay[i]);
    }
    
    
    hist->SetFillColor(kRed + 1);
    double mean = hist->GetMean(); //The mean of the distribution
    double stdev = hist->GetRMS(); //The standard deviation
    TCanvas* c2 = new TCanvas("c2", "Number of day");
    hist->Draw();
    
    TF1* fitfunc = new TF1("Gaussian", Gaussian3, -20, 40, 3);
    fitfunc->SetParameters(1, 5, 3); //Starting values for fitting
    fitfunc->SetLineColor(kBlack);

    hist->Fit(fitfunc, "Q1R");

    fitfunc->GetParameter(0); //The constant
    double fitmean = fitfunc->GetParameter(1);//The mean
    double fitstdev = fitfunc->GetParameter(2);//The standard deviation
    double errormean = fitfunc->GetParError(1); //Error of parameter
    double errorstdev = fitfunc->GetParError(2);
    
    TLegend* leg = new TLegend(0.2,0.7,0.5,0.9);
    leg->SetFillStyle(0); //Hollow fill (transparent)
    leg->SetBorderSize(0); //Get rid of the border
    //leg->SetHeader("The Legend Title");
    leg->AddEntry(hist,"Temperature on 19/7","f");
    leg->AddEntry(fitfunc, "Gaussian fit", "l");
    leg->Draw();
   
    // Save the canvas as a picture
    c2->SaveAs("tempOnDayNumber.png");
    
    cout << "Data mean: "<<mean << ", fitted function mean: " << fitmean<< " with error: " << errormean<< endl;
    cout << "Data standard deviation: "<<stdev << ", fitted function standard deviation: " << fitstdev<< " with error: " << errorstdev<< endl;

}





