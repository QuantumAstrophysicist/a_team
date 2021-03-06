#include "tempTrender.h"
// ROOT library objects
#include <TF1.h> // 1d function class
#include <TH1.h> // 1d histogram classes
#include <TStyle.h>  // style object
#include <TMath.h>   // math functions
#include <TCanvas.h> // canvas object
#include <TGraph.h>
#include <TLegend.h>
#include <TLine.h>

//calculate average temperature on each day
void calcAverageTemp(const vector <vector <string> > &data, vector <vector <double> > &averageTemp){

    int day;
    
    //load data first day
    int yearPrevius = stoi(data.at(0).at(0));
    int monthPrevius = stoi(data.at(0).at(1));
    int dayPrevius = stoi(data.at(0).at(2));
    
    double tempSum = stod(data.at(0).at(6));
    int counterTimes = 1;
    
    //loop through lines in the data
    for (int i=1; i < int(data.size()); i++)
    {
        day = stoi(data.at(i).at(2)); //stoi converts str to int
        
        vector<double> outputLine;
    
        if (day == dayPrevius){
            //sum temperaturs of the same date
            tempSum += stod(data.at(i).at(6));
            counterTimes++;
        }
        else{
            //save data in vector
            outputLine.push_back(yearPrevius);
            outputLine.push_back(monthPrevius);
            outputLine.push_back(dayPrevius);
            //calc average temperature of a date and save to vector
            outputLine.push_back(tempSum/counterTimes);
            
            //save date and average temperature in vector
            averageTemp.push_back(outputLine);
            
            //start temperature summation for new date
            tempSum = stod(data.at(i).at(6)); //stod converts str to double
            counterTimes = 1;
        }
        
        yearPrevius = stoi(data.at(i).at(0));
        monthPrevius = stoi(data.at(i).at(1));
        dayPrevius = day;
    }
}

//concatenates the date in the vector and returns it as int
template <class T>
int dateToInt (const T vec, int line) {
    //if (int(vec.at(line).size()) >= 3) {
    string year = to_string(int(vec.at(line).at(0)));
    string month = to_string(int(vec.at(line).at(1)));
    string day = to_string(int(vec.at(line).at(2)));
    
    //make every date the same length
    if (month.length() == 1) {
        month = "0" + month;
    }
    if (day.length() == 1) {
        day = "0" + day;
    }

    int date = stoi(year + month + day);
    return date;
}

//check if the second season date is after the start of the first season
bool seasonsInOrder(int yearStart, int i, string season,
                     const vector <vector <double> > &secondSeason,
                     const vector <vector <int> > &firstSeason){
    
    bool rightOrder = true;
    bool compareWinter = false;
    
    if (season == "spring") {
        compareWinter = true;
    }

    //loop trough first season to find corresponding year
    for (int j=0; j < int(firstSeason.size()); j++){
        if (yearStart == firstSeason.at(j).at(0)) {
            
            if ((compareWinter && firstSeason.at(j).at(1) < 6) || !compareWinter) {
                //get date of seasons to compare
                int firstSeasonDate = dateToInt<vector <vector <int> >>(firstSeason, j);
                int secondSeasonDate = dateToInt<vector <vector <double> >>(secondSeason, i);
                
                if (firstSeasonDate > secondSeasonDate) {
                    rightOrder = false;
                }
            }
            break;
        }
    }
    
    return rightOrder;
}

//check if it is a winter/summer temperature
bool rightTemperature(string season, double temp) {
    if (season == "winter") {
        return (temp <= 0);
    }
    else if (season == "summer") {
        return (temp >= 10);
    }
    else {return false;}
}

//check is if it is a new winter/summer
bool newWinterSummer(string season, int yearFirst, int monthFirst,
                     int yearPrevius, int monthPrevius){
    if (season == "winter") {
        //The winter does not always start once per year (month 1 to 12)
        //There are 4 cases of how the begin days of winters can follow
        
        //successive winters both early in year (month < 6)
        bool case1 = (yearFirst != yearPrevius) &&
                     ((monthPrevius < 6) && (monthFirst < 6));
        //successive winters both late in year (month > 6)
        bool case2 = (yearFirst != yearPrevius) &&
                     ((monthPrevius > 6) && (monthFirst > 6));
        //succesive winters are in the same year (one early the other late)
        bool case3 = (monthPrevius < 6) && (monthFirst > 6);
        //succesive winters skip a year (one winter late, the next early)
        bool case4 = (monthPrevius > 6) && (yearFirst - yearPrevius == 2);
        
        //new winter if true
        return (case1 || case2 || case3 || case4);
    }
    else if (season == "summer"){
        return (yearFirst != yearPrevius);
    }
    else {return false;}
}

//finds the first day of each winter and saves the date in a vector
void tempTrender::beginningWinterSummer(const vector <vector <double> > &averageTemp,
                                        vector <vector <int> > &beginDay,
                                        string season){
    
    int yearPrevius = averageTemp.at(0).at(0)-1; //first year in data -1
    int monthPrevius = 12; //assume the winter has started in the previous year (before data)
    int yearFirst = averageTemp.at(0).at(0)-1, monthFirst = 0, dayFirst = 0;
    int counterDays = 0;
    bool rightTemp = false, newSeason = false;
    
    cout << "Calculating the first day of each " << season << "..." << endl;
    
    //loop through dates in the vector
    for (int i=0; i < int(averageTemp.size()); i++){
        
        rightTemp = rightTemperature(season, averageTemp.at(i).at(3));
        
        //is it a winter/summer temperature
        if (rightTemp){
            //remember date of first winter/summer day
            if (counterDays == 0) {
                yearFirst = averageTemp.at(i).at(0);
                monthFirst = averageTemp.at(i).at(1);
                dayFirst = averageTemp.at(i).at(2);
            }
            counterDays++;
            
            //begining of winter/summer
            if (counterDays == 5) {
                
                newSeason = newWinterSummer(season, yearFirst, monthFirst, yearPrevius, monthPrevius);
                
                //a new winter/summer
                if (newSeason) {
                
                    vector<int> outputLine;
                    int dayOfYear = getDayOfYear(yearFirst, monthFirst, dayFirst);
                    
                    //save date first day winter/summer of a year
                    outputLine.push_back(yearFirst);
                    outputLine.push_back(monthFirst);
                    outputLine.push_back(dayFirst);
                    
                    outputLine.push_back(dayOfYear);
                    
                    beginDay.push_back(outputLine);
                    
                    yearPrevius = averageTemp.at(i).at(0);
                    monthPrevius = averageTemp.at(i).at(1);
                    counterDays = 0;
                }
            }
        }
        else {
            counterDays = 0;
        }
    }
}

//finds the first day of spring or fall for each year and saves the date in a vector
void tempTrender::beginningSpringFall(const vector <vector <double> > &averageTemp,
                                      const vector <vector <int> > &beginDayFirst,
                                      vector <vector <int> > &beginDaySecond,
                                      string season){
    
    int yearPrevius = averageTemp.at(0).at(0) -1; //first year in data -1
    int yearFirst = averageTemp.at(0).at(0)-1;
    int monthFirst = 0, dayFirst = 0;
    int counterDays = 0, nDays = 0;
    
    cout << "Calculating the first day of each " << season << "..." << endl;
    
    //how many days in a row between 0 and 10 degrees before the seasons starts
    if (season == "spring") {
        nDays = 7;
    }
    else if (season == "fall") {
        nDays = 5;
    }
    
    //loop through dates in the vector
    for (int i=0; i < int(averageTemp.size()); i++){
        
        //is it a spring/fall temperature
        if (averageTemp.at(i).at(3) > 0 && averageTemp.at(i).at(3) < 10){
            //remember date of first spring/fall day
            if (counterDays == 0) {
                yearFirst = averageTemp.at(i).at(0);
                monthFirst = averageTemp.at(i).at(1);
                dayFirst = averageTemp.at(i).at(2);
            }
            counterDays++;
            
            //definition begining of spring/fall
            if (counterDays == nDays) {
                
                //check if the first spring/fall date is after the start of winter/summer
                bool afterFirst = seasonsInOrder(yearFirst, i-(nDays-1), season, averageTemp, beginDayFirst);
                
                //a new fall
                if ((yearFirst != yearPrevius) && afterFirst) {
                    
                    vector<int> outputLine;
                    int dayOfYear = getDayOfYear(yearFirst, monthFirst, dayFirst);
                    
                    //save date of the first day spring/fall of a year
                    outputLine.push_back(yearFirst);
                    outputLine.push_back(monthFirst);
                    outputLine.push_back(dayFirst);
                    
                    outputLine.push_back(dayOfYear);
                    
                    beginDaySecond.push_back(outputLine);
                    
                    yearPrevius = averageTemp.at(i).at(0);
                    counterDays = 0;
                }
                else {
                    counterDays--;
                }
            }
        }
        else {
            counterDays = 0;
        }
    }
}

//stores data to plot (or show in a histogram) in arrays
void arrayData(const vector <vector <int> > &beginDaySeason, string season,
                   int n, Int_t x[], Int_t y[]){
    for (int i=0; i < n; i++){
        x[i] = beginDaySeason.at(i).at(0);
        if (season == "winter" && beginDaySeason.at(i).at(1) > 6) {
            //make winters at the end of the year a negative number (wrt year ending)
            int year = beginDaySeason.at(i).at(0);
            if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
                //leap year
                y[i] = beginDaySeason.at(i).at(3) -366;
            }
            y[i] = beginDaySeason.at(i).at(3) -365;
        }
        else {
            y[i] = beginDaySeason.at(i).at(3);
        }
    }
}

//plots the first day of a season for each year in a bar graph
void plotSeason(const vector <vector <int> > &beginDaySeason, string season){
    int n = int(beginDaySeason.size());
    Int_t xYear[n];
    Int_t yDay[n];
    
    arrayData(beginDaySeason, season, n, xYear, yDay);
    
    TCanvas* cSeason = new TCanvas("cSeason", Form("The first day of %s", season.c_str()),900, 600);
    TGraph* gSeason = new TGraph (n, xYear, yDay);
    
    gSeason->SetFillColor(kBlue -3);
    
    gSeason->Draw(); //need to draw before you can set axis title
    if (season == "winter") {
        gSeason->SetMinimum(-183);
        gSeason->SetMaximum(183);
        gSeason->GetYaxis()->SetTitle("Day after the end of the year");
    }
    else{
        gSeason->SetMinimum(1);
        gSeason->SetMaximum(366);
        gSeason->GetYaxis()->SetTitle("Day of the year");
    }
    gSeason->GetXaxis()->SetTitle("Year");
    gSeason->GetXaxis()->CenterTitle();
    gSeason->GetYaxis()->CenterTitle();
    
    gSeason->Draw("AB");
    
    //legend
    TLegend* leg = new TLegend(0.58, 0.87, 0.949, 0.93);
    leg->SetBorderSize(0);
    leg->AddEntry(gSeason, Form("First day of %s", season.c_str()), "f");
    leg->SetTextSize(.05);
    leg->Draw();
    
    cSeason->SaveAs(Form("tempSeasonPlots/%sStart.png", season.c_str()));
    cSeason->Close();
}

//create a histogram of the start days of a season
void histogram(const vector <vector <int> > &beginDaySeason, string season){
    int n = int(beginDaySeason.size());
    Int_t x[n], y[n];
    
    arrayData(beginDaySeason, "none", n, x, y);
    
    TH1I* hist = new TH1I("hist", Form("Spread in the first day of %s; Day of year; Entries", season.c_str()), 366, 1, 366);
    TCanvas* c_hist = new TCanvas("c1", "v2 canvas", 900, 600);
    
    //fill the histogram
    for (int i=0; i < n; i++){
        hist->Fill(y[i]);
    }
    
    hist->SetFillColor(kBlue + 3);
    hist->Draw();
    hist->GetXaxis()->CenterTitle();
    hist->GetYaxis()->CenterTitle();
    
    //legend
    if (season == "fall") {
        //at different position for fall histogram
        TLegend* leg = new TLegend(0.2, 0.87, 0.549, 0.93);
        leg->SetBorderSize(0);
        leg->AddEntry(hist, Form("First day of %s", season.c_str()), "l");
        leg->SetTextSize(.05);
        leg->Draw();
    }
    else {
        TLegend* leg = new TLegend(0.58, 0.87, 0.949, 0.93);
        leg->SetBorderSize(0);
        leg->AddEntry(hist, Form("First day of %s", season.c_str()), "l");
        leg->SetTextSize(.05);
        leg->Draw();
    }
    
    c_hist->SaveAs(Form("tempSeasonPlots/%sHistogram.png", season.c_str()));
    c_hist->Close();
    delete hist;
}

//calculates the first day of seasons for every year in the dataset Lund
//plots data in bar graphs and histograms
void tempTrender::startDaySeasons(){
    cout << endl;
    cout << "Analysing the first day of the seasons" << endl;
    cout << endl;
    
    //definitions start seasons
    //https://www.smhi.se/kunskapsbanken/meteorologi/arstider-1.1082
    //winter t_average =< 0 for 5 days -> first of those days season begins
    //spring 0< t_average <10 for 7 days
    //summer t_average >= 10 for 5 days
    //fall 0< t_average <10 for 5 days
    
    vector <vector <string> > dataSeasons;
    vector <vector <double> > averageTempDay;
    vector <vector <int> > firstDayWinter, firstDaySpring, firstDaySummer, firstDayFall;
    
    readData("smhi-opendata_Lund.csv", dataSeasons);
    calcAverageTemp(dataSeasons, averageTempDay);
    
    //calculate start of seasons
    beginningWinterSummer(averageTempDay, firstDayWinter, "winter");
    beginningSpringFall(averageTempDay, firstDayWinter, firstDaySpring, "spring");
    beginningWinterSummer(averageTempDay, firstDaySummer, "summer");
    beginningSpringFall(averageTempDay, firstDaySummer, firstDayFall, "fall");
    
    //print<vector <vector <int> >>(firstDayWinter, int(firstDayWinter.size()));
    
    //make bar graphs of the results
    cout << endl << "Plotting the results"<< endl;
    plotSeason(firstDaySpring, "spring");
    plotSeason(firstDaySummer, "summer");
    plotSeason(firstDayFall, "fall");
    plotSeason(firstDayWinter, "winter");
    
    //making histograms of the result
    cout << endl << "Making histograms" << endl;
    histogram(firstDaySpring, "spring");
    histogram(firstDaySummer, "summer");
    histogram(firstDayFall, "fall");
    histogram(firstDayWinter, "winter");
    cout << endl;
}

















