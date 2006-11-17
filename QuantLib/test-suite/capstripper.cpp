/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Francois du Vignaud

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "capstripper.hpp"
#include "utilities.hpp"
#include <ql/Calendars/target.hpp>
#include <ql/DayCounters/actual360.hpp>
#include <ql/Volatilities/capstripper.hpp>
#include <ql/Indexes/euribor.hpp>
#include <ql/TermStructures/flatforward.hpp>
#include <ql/Utilities/dataparsers.hpp>
#include <iostream>
#include <ql/CashFlows/floatingratecoupon.hpp> 
#include <ql/Utilities/dataformatters.hpp>
#include <ql/Instruments/makecapfloor.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(CapsStripperTest)


Calendar calendar;
DayCounter dayCounter;
std::vector<Rate> strikes;
std::vector<Period> tenors;
std::vector<std::vector<Handle<Quote> > > volatilityQuoteHandle;
boost::shared_ptr<FlatForward> myTermStructure;
boost::shared_ptr<Quote> forwardRate;
Handle<Quote> forwardRateQuote;
Handle<YieldTermStructure> rhTermStructure;
boost::shared_ptr<Xibor> xiborIndex;
int fixingDays;
BusinessDayConvention businessDayConvention;
boost::shared_ptr<CapsStripper> capsStripper;
Rate flatForwardRate;
Matrix v;
QL_END_TEST_LOCALS(CapsStripperTest)

Real maxAbs(const Matrix& m){
    Real max = QL_MIN_REAL; 
    for (Size i=0; i<m.rows(); i++) {
        for (Size j=0; j<m.columns(); j++)
            max = std::max(std::fabs(m[i][j]), max);   
    }
    return max;
}


// set a Flat Volatility Term Structure at a given level
void setFlatVolatilityTermStructure(Volatility flatVolatility){
    strikes.resize(10);
    tenors.resize(10);
    tenors.resize(tenors.size());
    for (Size i = 0 ; i < tenors.size(); i++)
        tenors[i] = Period(i+1, Years);
    strikes.resize(strikes.size());
    for (Size j = 0 ; j < strikes.size(); j++)
        strikes[j] = double(j+1)/100;

    volatilityQuoteHandle.resize(tenors.size());
    boost::shared_ptr<SimpleQuote> 
        volatilityQuote(new SimpleQuote(flatVolatility));
    for (Size i = 0 ; i < tenors.size(); i++){
        volatilityQuoteHandle[i].resize(strikes.size());
        for (Size j = 0 ; j < strikes.size(); j++){
            volatilityQuoteHandle[i][j] = Handle<Quote>(volatilityQuote,true);
        }
    }

}

void setMarketVolatilityTermStructure(){
    strikes.resize(13);
    tenors.resize(16);
    v = Matrix(tenors.size(), strikes.size());
    v[0][0]=0.287;	v[0][1]=0.274;	v[0][2]=0.256;	v[0][3]=0.245;	v[0][4]=0.227;	v[0][5]=0.148;	v[0][6]=0.096;	v[0][7]=0.09;	v[0][8]=0.11;	v[0][9]=0.139;	v[0][10]=0.166;	v[0][11]=0.19;	v[0][12]=0.214;
    v[1][0]=0.303;	v[1][1]=0.258;	v[1][2]=0.22;	v[1][3]=0.203;	v[1][4]=0.19;	v[1][5]=0.153;	v[1][6]=0.126;	v[1][7]=0.118;	v[1][8]=0.147;	v[1][9]=0.165;	v[1][10]=0.18;	v[1][11]=0.192;	v[1][12]=0.212;
    v[2][0]=0.303;	v[2][1]=0.257;	v[2][2]=0.216;	v[2][3]=0.196;	v[2][4]=0.182;	v[2][5]=0.154;	v[2][6]=0.134;	v[2][7]=0.127;	v[2][8]=0.149;	v[2][9]=0.166;	v[2][10]=0.18;	v[2][11]=0.192;	v[2][12]=0.212;
    v[3][0]=0.305;	v[3][1]=0.266;	v[3][2]=0.226;	v[3][3]=0.203;	v[3][4]=0.19;	v[3][5]=0.167;	v[3][6]=0.151;	v[3][7]=0.144;	v[3][8]=0.16;	v[3][9]=0.172;	v[3][10]=0.183;	v[3][11]=0.193;	v[3][12]=0.209;
    v[4][0]=0.294;	v[4][1]=0.261;	v[4][2]=0.216;	v[4][3]=0.201;	v[4][4]=0.19;	v[4][5]=0.171;	v[4][6]=0.158;	v[4][7]=0.151;	v[4][8]=0.163;	v[4][9]=0.172;	v[4][10]=0.181;	v[4][11]=0.188;	v[4][12]=0.201;
    v[5][0]=0.276;	v[5][1]=0.248;	v[5][2]=0.212;	v[5][3]=0.199;	v[5][4]=0.189;	v[5][5]=0.172;	v[5][6]=0.16;	v[5][7]=0.155;	v[5][8]=0.162;	v[5][9]=0.17;	v[5][10]=0.177;	v[5][11]=0.183;	v[5][12]=0.195;
    v[6][0]=0.26;	v[6][1]=0.237;	v[6][2]=0.21;	v[6][3]=0.198;	v[6][4]=0.188;	v[6][5]=0.172;	v[6][6]=0.161;	v[6][7]=0.156;	v[6][8]=0.161;	v[6][9]=0.167;	v[6][10]=0.173;	v[6][11]=0.179;	v[6][12]=0.19;
    v[7][0]=0.25;	v[7][1]=0.231;	v[7][2]=0.208;	v[7][3]=0.196;	v[7][4]=0.187;	v[7][5]=0.172;	v[7][6]=0.162;	v[7][7]=0.156;	v[7][8]=0.16;	v[7][9]=0.165;	v[7][10]=0.17;	v[7][11]=0.175;	v[7][12]=0.185;
    v[8][0]=0.244;	v[8][1]=0.226;	v[8][2]=0.206;	v[8][3]=0.195;	v[8][4]=0.186;	v[8][5]=0.171;	v[8][6]=0.161;	v[8][7]=0.156;	v[8][8]=0.158;	v[8][9]=0.162;	v[8][10]=0.166;	v[8][11]=0.171;	v[8][12]=0.18;
    v[9][0]=0.239;	v[9][1]=0.222;	v[9][2]=0.204;	v[9][3]=0.193;	v[9][4]=0.185;	v[9][5]=0.17;	v[9][6]=0.16;	v[9][7]=0.155;	v[9][8]=0.156;	v[9][9]=0.159;	v[9][10]=0.163;	v[9][11]=0.168;	v[9][12]=0.177;
    v[10][0]=0.235;	v[10][1]=0.219;	v[10][2]=0.202;	v[10][3]=0.192;	v[10][4]=0.183;	v[10][5]=0.169;	v[10][6]=0.159;	v[10][7]=0.154;	v[10][8]=0.154;	v[10][9]=0.156;	v[10][10]=0.16;	v[10][11]=0.164;	v[10][12]=0.173;
    v[11][0]=0.227;	v[11][1]=0.212;	v[11][2]=0.197;	v[11][3]=0.187;	v[11][4]=0.179;	v[11][5]=0.166;	v[11][6]=0.156;	v[11][7]=0.151;	v[11][8]=0.149;	v[11][9]=0.15;	v[11][10]=0.153;	v[11][11]=0.157;	v[11][12]=0.165;
    v[12][0]=0.22;	v[12][1]=0.206;	v[12][2]=0.192;	v[12][3]=0.183;	v[12][4]=0.175;	v[12][5]=0.162;	v[12][6]=0.153;	v[12][7]=0.147;	v[12][8]=0.144;	v[12][9]=0.144;	v[12][10]=0.147;	v[12][11]=0.151;	v[12][12]=0.158;
    v[13][0]=0.211;	v[13][1]=0.197;	v[13][2]=0.185;	v[13][3]=0.176;	v[13][4]=0.168;	v[13][5]=0.156;	v[13][6]=0.147;	v[13][7]=0.142;	v[13][8]=0.138;	v[13][9]=0.138;	v[13][10]=0.14;	v[13][11]=0.144;	v[13][12]=0.151;
    v[14][0]=0.204;	v[14][1]=0.192;	v[14][2]=0.18;	v[14][3]=0.171;	v[14][4]=0.164;	v[14][5]=0.152;	v[14][6]=0.143;	v[14][7]=0.138;	v[14][8]=0.134;	v[14][9]=0.134;	v[14][10]=0.137;	v[14][11]=0.14;	v[14][12]=0.148;
    v[15][0]=0.2;	v[15][1]=0.187;	v[15][2]=0.176;	v[15][3]=0.167;	v[15][4]=0.16;	v[15][5]=0.148;	v[15][6]=0.14;	v[15][7]=0.135;	v[15][8]=0.131;	v[15][9]=0.132;	v[15][10]=0.135;	v[15][11]=0.139;	v[15][12]=0.146;

  
    volatilityQuoteHandle.resize(tenors.size());
    for (Size i = 0 ; i < tenors.size(); i++){
        volatilityQuoteHandle[i].resize(strikes.size());
        for (Size j = 0 ; j < strikes.size(); j++){
            boost::shared_ptr<SimpleQuote> 
                volatilityQuote(new SimpleQuote(v[i][j]));
            volatilityQuoteHandle[i][j] = Handle<Quote>(volatilityQuote,true);
        }
        
    }

    strikes.resize(strikes.size());
    strikes[0]=0.015;
    strikes[1]=0.0175;
    strikes[2]=0.02;
    strikes[3]=0.0225;
    strikes[4]=0.025;
    strikes[5]=0.03;
    strikes[6]=0.035;
    strikes[7]=0.04;
    strikes[8]=0.05;
    strikes[9]=0.06;
    strikes[10]=0.07;
    strikes[11]=0.08;
    strikes[12]=0.1;

    tenors.resize(tenors.size());
    tenors[0]= PeriodParser::parse("1Y");
    tenors[1]= PeriodParser::parse("18M");
    tenors[2]= PeriodParser::parse("2Y");
    tenors[3]= PeriodParser::parse("3Y");
    tenors[4]= PeriodParser::parse("4Y");
    tenors[5]= PeriodParser::parse("5Y");
    tenors[6]= PeriodParser::parse("6Y");
    tenors[7]= PeriodParser::parse("7Y");
    tenors[8]= PeriodParser::parse("8Y");
    tenors[9]= PeriodParser::parse("9Y");
    tenors[10]= PeriodParser::parse("10Y");
    tenors[11]= PeriodParser::parse("12Y");
    tenors[12]= PeriodParser::parse("15Y");
    tenors[13]= PeriodParser::parse("20Y");
    tenors[14]= PeriodParser::parse("25Y");
    tenors[15]= PeriodParser::parse("30Y");
}


void setup(Real impliedVolatilityPrecision = 1e-5) {

    calendar = TARGET();
    dayCounter = Actual360();
    fixingDays = 2;
    businessDayConvention = Unadjusted;
    Integer settlementDays = 2;
    flatForwardRate = 0.04;
    forwardRate = boost::shared_ptr<Quote>(new SimpleQuote(flatForwardRate));
    forwardRateQuote.linkTo(forwardRate);
    myTermStructure = boost::shared_ptr<FlatForward>(
                  new FlatForward(settlementDays, calendar, forwardRateQuote,
                                  dayCounter));
    rhTermStructure.linkTo(myTermStructure);

    xiborIndex = boost::shared_ptr<Xibor>(new Euribor6M(rhTermStructure));
    capsStripper = boost::shared_ptr<CapsStripper>(new CapsStripper(tenors,
                                                strikes,
                                                volatilityQuoteHandle, 
                                                xiborIndex,
                                                rhTermStructure,
                                                dayCounter,
                                                impliedVolatilityPrecision));
}


void debug(){
    setMarketVolatilityTermStructure();
    Volatility flatVolatility = .18;
    //setFlatVolatilityTermStructure(flatVolatility);
    setup();
    //Settings::instance().evaluationDate() = Date(39023);
    capsStripper->recalculate();
    //printCapsStripper(*capsStripper);
    std::cout << capsStripper->parametrizedCapletVolStructure()
        ->volatilityParameters();
}

/* We strip a flat volatility surface and we check if 
    the result is equal to the initial surface
*/
void CapsStripperTest::FlatVolatilityStripping() {

    BOOST_MESSAGE("Testing Flat Volatility Stripping...");
    QL_TEST_BEGIN
    Volatility flatVolatility = .18;
    setFlatVolatilityTermStructure(flatVolatility);
    setup();
    const CapMatrix& marketDataCap = capsStripper->marketDataCap();
    for (Size tenorIndex = 0; tenorIndex < tenors.size() ; tenorIndex++){
        Date tenorDate = marketDataCap[tenorIndex][0]->lastFixingDate();
        Time tenorTime =  dayCounter.yearFraction(
            Settings::instance().evaluationDate(), tenorDate);
        for (Size strikeIndex = 0; strikeIndex < strikes.size() ; strikeIndex ++){
            Real blackVariance = 
                capsStripper->blackVariance(tenorDate, strikes[strikeIndex],true);
            Volatility volatility = std::sqrt(blackVariance/tenorTime);
            Real relativeError = (volatility - flatVolatility)/flatVolatility *100;

            if (std::fabs(relativeError)>1e-2)
                BOOST_ERROR(   "tenor:\t" << tenors[tenorIndex]
                            << "\nstrike:\t" << strikes[strikeIndex]*100 << "%"
                            << "\nvolatility=:\t" << volatility
                            << "\nrelativeError=:\t" << relativeError
                            << "\n-------------\n");
        }
    }
    QL_TEST_END
}

  



/* High precision consistency test*/

void CapsStripperTest::highPrecisionTest(){
    BOOST_MESSAGE("Stripping of cap volatilities high precision consistency test ...");
    QL_TEST_BEGIN

    setMarketVolatilityTermStructure();
    Real impliedVolatilityPrecision = 1e-20;
    setup(impliedVolatilityPrecision);
    static const Real tolerance = 1e-12;
    static const Real priceThreshold = 1e-6;
    
    Handle <CapletVolatilityStructure> strippedVolatilityStructureHandle(capsStripper);
    boost::shared_ptr<BlackCapFloorEngine> strippedVolatilityBlackCapFloorEngine
        (new BlackCapFloorEngine(strippedVolatilityStructureHandle));
    for (Size tenorIndex = 0; tenorIndex < tenors.size() ; tenorIndex++){
        for (Size strikeIndex = 0; strikeIndex < strikes.size() ; strikeIndex ++){
            boost::shared_ptr<CapFloor> cap = MakeCapFloor(CapFloor::Cap,
                tenors[tenorIndex], xiborIndex, strikes[strikeIndex], 
                0*Days, strippedVolatilityBlackCapFloorEngine);
            Real priceFromStrippedVolatilty = cap->NPV();
            boost::shared_ptr<PricingEngine> blackCapFloorEngineConstantVolatility(
                new BlackCapFloorEngine(
                    volatilityQuoteHandle[tenorIndex][strikeIndex], dayCounter));
            cap->setPricingEngine(blackCapFloorEngineConstantVolatility);
            Real priceFromConstantVolatilty = cap->NPV();
            Real absError = std::fabs(priceFromStrippedVolatilty - priceFromConstantVolatilty);
            Real relativeError = absError/priceFromConstantVolatilty;
            bool strippedPriceIsAccurate;
            // if we test a short maturity the tolerance is increased because the 
            // vega might be too low so that we haven't stripped the volatility
            if (tenorIndex <=1)
                strippedPriceIsAccurate = relativeError < tolerance *1e2;
            else
                strippedPriceIsAccurate = relativeError < tolerance;
            // if prices are too low the relative discrepancy is not relevant
            bool priceIsBigEnough = priceFromConstantVolatilty > priceThreshold;

            if(!strippedPriceIsAccurate && priceIsBigEnough)
                BOOST_FAIL("\ntenor: " << tenors[tenorIndex] <<
                           "\nstrike: " << io::rate(strikes[strikeIndex]) <<
                           "\nstripped: " << priceFromStrippedVolatilty * 1e4 <<
                           "\nconstant: " << priceFromConstantVolatilty * 1e4 <<
                           "\nabs error: " << QL_SCIENTIFIC << absError << "\n"
                           "\nrel error: " << io::percent(relativeError) << "\n");
         }
    }
    
    QL_TEST_END
}


test_suite* CapsStripperTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("CapsStripper tests");
    suite->add(BOOST_TEST_CASE(&CapsStripperTest::FlatVolatilityStripping));
    suite->add(BOOST_TEST_CASE(&CapsStripperTest::highPrecisionTest));
    //suite->add(BOOST_TEST_CASE(&debug));
    return suite;
}
