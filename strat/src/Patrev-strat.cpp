#include "Patrev-strat.hpp"

#include <cmath>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <typeinfo>
#include <pirate/tr/Trader.hpp>
#include <pirate/tr/cfg/MktDef.hpp>
#include <cassert>

using namespace std;
using pirate::ind::naValue;
using pirate::ind::TmPx;
using boost::gregorian::date;
using boost::gregorian::date_duration;
using boost::posix_time::ptime;
using boost::posix_time::time_duration;
using pirate::tr::data::OHLC;
using pirate::ind::naValue;
using pirate::ind::na;
using pirate::ind::EMACalc;
using pirate::tr::cfg::MktDef;

#include <pirate/tr/Trader.hpp>
#include "pirate/tr/to/TimeInForce.hpp"
using pirate::tr::to::TimeInForce;
#include "pirate/tr/to/TifRoll.hpp"
using pirate::tr::to::TifRoll;

Strat::Strat(
    const pirate::ind::period_type aNearPeriod,
    const pirate::ind::period_type aMedPeriod,
    const pirate::ind::period_type aFarPeriod,
    const pirate::ind::period_type aLongAaaPeriod, 
    const pirate::ind::period_type aShortAaaPeriod,
    const pirate::ind::period_type aRicciPeriod,
    const pirate::ind::period_type aAllocAtrPeriod,    
    const pirate::ind::period_type aRefAtrPeriod,    
    const pirate::ind::period_type aPctPeriod,
    const pirate::ind::period_type aExtrDistance,

    const int aMaxPosAge,

    const double aInd1LBPct,
    const double aInd1UBPct,
    const double aInd2LBPct,
    const double aInd2UBPct,
    const double aInd3LBPct,
    const double aInd3UBPct,

    const int aNumOfUnits,
    const float aRiskFactor,
    const float aTickMult, 
    const float aTpfMult,
    const float aAlpha,
    const float aBeta) 
: pirate::tr::bar::BarStrat(),
  intNA(naValue<int>()),
  floatNA(naValue<float>()),
  doubleNA(naValue<double>()),
  
  farPeriod(aFarPeriod),
  medPeriod(aMedPeriod),
  nearPeriod(aNearPeriod),

  hiNearCalc(aNearPeriod),
  hiMedCalc(aMedPeriod),
  hiFarCalc(aFarPeriod),
  loNearCalc(aNearPeriod),
  loMedCalc(aMedPeriod),
  loFarCalc(aFarPeriod),

  extrDistance(aExtrDistance),
  
  longAaaUpCalc(EMACalc::Period(static_cast<unsigned int>(aLongAaaPeriod)), 0),
  longAaaDownCalc(EMACalc::Period(static_cast<unsigned int>(aLongAaaPeriod)), 0), 
  longAaaDiffCalc(EMACalc::Period(static_cast<unsigned int>(aLongAaaPeriod)), 0),
  longAaaAtrCalc(aLongAaaPeriod),
  
  shortAaaUpCalc(EMACalc::Period(static_cast<unsigned int>(aShortAaaPeriod)), 0),
  shortAaaDownCalc(EMACalc::Period(static_cast<unsigned int>(aShortAaaPeriod)), 0), 
  shortAaaDiffCalc(EMACalc::Period(static_cast<unsigned int>(aShortAaaPeriod)), 0),
  shortAaaAtrCalc(aShortAaaPeriod),
  
  ricciHiCalc(aRicciPeriod),
  ricciLoCalc(aRicciPeriod),
  ricciAtrCalc(aRicciPeriod), 
  
  volumeMACalc(4), 
  allocAtrCalc(aAllocAtrPeriod),
  refAtrCalc(aRefAtrPeriod),
  
  ind1LBCalc(aPctPeriod, aInd1LBPct),
  ind1UBCalc(aPctPeriod, aInd1UBPct),
  ind2LBCalc(aPctPeriod, aInd2LBPct),
  ind2UBCalc(aPctPeriod, aInd2UBPct),
  ind3LBCalc(aPctPeriod, aInd3LBPct),
  ind3UBCalc(aPctPeriod, aInd3UBPct),
  
  hiNear(),
  loNear(),
  hiMed(),
  loMed(),
  hiFar(),
  loFar(),
  
  longPosAge(0),
  shortPosAge(0),
  longTimer(0),
  shortTimer(0),
  
  longAaa(doubleNA),
  longAaaAtr(doubleNA),
  shortAaa(doubleNA),
  shortAaaAtr(doubleNA),
  ricch(),
  riccl(),
  ricca(doubleNA),
  
  allocAtr(doubleNA),
  refAtr(doubleNA),
  
  maxPosAge(aMaxPosAge),
  ind1LBPct(aInd1LBPct),
  ind1UBPct(aInd1UBPct),
  ind2LBPct(aInd2LBPct),
  ind2UBPct(aInd2UBPct),
  ind3LBPct(aInd3LBPct),
  ind3UBPct(aInd3UBPct),
  
  ind1Value(doubleNA),
  ind2Value(doubleNA),
  ind1LBValue(doubleNA),
  ind1UBValue(doubleNA),
  ind2LBValue(doubleNA),
  ind2UBValue(doubleNA),
  ind3LBValue(doubleNA),
  ind3UBValue(doubleNA),
  
  minTickSz(0),
  //      slpgMult(aSlpgMult),
  numOfUnits(aNumOfUnits),
  riskFactor(aRiskFactor),
  tickMult(aTickMult),
  tpfMult(aTpfMult),
  alpha(aAlpha),
  beta(aBeta),
  
  DELTA(0),
  
  m_pxs(500),
  m_dtms(500),
  m_vols(500),
  m_prevClose(naValue<float>()),
  m_adjCtID(0),
  
  m_prevDtm(),
  m_adjDtm(),
  
  m_prevDt(0),
  m_adjDt(0),
  m_prevTm(0),
  m_adjTm(0)
{
    pxHistAdj.swap(*(new BarBuff(2)));
    pxHistRaw.swap(*(new BarBuff(300)));
    enLongFilter.swap(*(new BoolBuff(5)));
    enShortFilter.swap(*(new BoolBuff(5)));
    
    hhHist.swap(*(new DoubleBuff(3)));
    llHist.swap(*(new DoubleBuff(3)));
}

/* virtual */ void Strat::begin() {
   
    minTickSz = minTickSize();
    retPeriod = 3;

#if 0
    char *endptr;
    //DELTA = minTickSz * tickMult;

    const MktDef& mktDef = m_traderPtr->mktDef();
    // settlement: first column after CtCnt
    MktDef::const_iterator mktDefIter = mktDef.beginArgs();
    const std::string& settleString = *mktDefIter;
    std::string::size_type delimPos = settleString.find(":");
    int hh = static_cast<int>(strtol(settleString.substr(0, delimPos).c_str(),
                                     &endptr, 10));
    char *endptr0 = endptr;
    int errno0 = errno;
    int mm = static_cast<int>(strtol(settleString.substr(delimPos+1,
                                                         std::string::npos).c_str(),
                                     &endptr, 10));
    if (*endptr == '\0' && *endptr0 == '\0' && errno == 0 && errno0 == 0) {
        settleTm = time_duration(hh, mm, 0);
        settleTmInt = hh*100 + mm;
    }

    // TradeDir: 12 columns after settle (13th after ctcnt)
    // we need to reserve and retain the order of colums before TradeDir column
    // for pirate.data.ibpofdata.py 
    mktDefIter += 12;
    /*
    const std::string& tradeDirString = *mktDefIter;
    if (!std::all_of(tradeDirString.begin(), tradeDirString.end(), isspace)) {
        long td = strtol(tradeDirString.c_str(), &endptr, 10);
        if (*endptr == '\0' && errno == 0) tradeDirection = static_cast<int>(td);
    }
    mktDefIter++;
    */
    
    // StpLossPeriod
    /*
    const std::string& stpLossPeriodString = *mktDefIter;
    if (!std::all_of(stpLossPeriodString.begin(), stpLossPeriodString.end(), isspace)) {
        int slp = static_cast<int>(strtol(stpLossPeriodString.c_str(), &endptr, 10));
        if (*endptr == '\0' && errno == 0) stpLossPeriod = slp;
    }

    if (stpLossPeriod > 0) {
        hiNearCalc.reset(stpLossPeriod);
        loNearCalc.reset(stpLossPeriod);
        hiMedCalc.reset(stpLossPeriod);
        loMedCalc.reset(stpLossPeriod);
    }
    //*/
#endif
    
    if (debugLevel > 0)
    {
        logHeader() 
            << sep << "TsDt"
            << sep << "spread"
            << sep << "adjOp"
            << sep << "adjHi"
            << sep << "adjLo"
            << sep << "adjCl"

            << sep << "hiNear"
            << sep << "loNear"
            << sep << "hiMed"
            << sep << "loMed"
            << sep << "hiFar"
            << sep << "loFar"

            << sep << "longAaa"
            << sep << "longAaaAtr"
            
            << sep << "shortAaa"
            << sep << "shortAaaAtr"
            
            << sep << "ricch"
            << sep << "riccl"
            << sep << "ricca"

            << sep << "allocAtr"
            << sep << "refAtr"
            << sep << "weight"
            << sep << "equity"
            << sep << "exchRt"
            << sep << "ptValue"
            << sep << "nc"
            << sep << "volCap"

            << sep << "enLongFilter[0]"
            << sep << "enLongFilter[1]"
            << sep << "enShortFilter[0]"
            << sep << "enShortFilter[1]"

            << sep << "ind1LB"
            << sep << "ind1UB"
            << sep << "ind2LB"
            << sep << "ind2UB"
            << sep << "ind3LB"
            << sep << "ind3UB"

            << sep << "\n";
    }
}

/* virtual */ void Strat::calc() {

    //std::cout << "dtm = " << dtm << std::endl;

    // MktSim is still spooling in new ct data before the last time 
    // of the previous contract (rollover time).  We will not use 
    // these data to do anything and hence we just return. 
    if (dtm < m_adjDtm && !m_adjDtm.is_special())
    {
        /*
          enter here ONLY IF
          we are spooling NEW CT BEFORE rollover time. 
        */
        
        hiNear = hiNearCalc.calc(dtm, high);
        loNear = loNearCalc.calc(dtm, low);
        hiMed = hiMedCalc.calc(dtm, high);
        loMed = loMedCalc.calc(dtm, low);
        hiFar = hiFarCalc.calc(dtm, high);
        loFar = loFarCalc.calc(dtm, low);
        pxHistRaw.push_front(OHLC(open, high, low, close));
        hhHist.push_front(hiFar.px);
        llHist.push_front(loFar.px);
        enLongFilter.push_front(testEnterLongFilter());
        enShortFilter.push_front(testEnterShortFilter());
        
        prevVolumeMA = volumeMA;
        if (volume > 0) {
            volumeMA = static_cast<float>(volumeMACalc.calc(volume));
        }
        return; 
    }
    // now, we have passed the rollover point and we will adjust the saved 
    // old contract data (rather than using current contract data before 
    // rollover time!) and use the adjusted data for indicator calculation. 
    // So, we need to recalculate all indicators. 
    
    //std::cout << "ctID = " << ctID << ", m_adjCtID = " << m_adjCtID 
    //          << ", m_adjDtm = " << to_iso_string(m_adjDtm)  
    //          << ", dtm = " << to_iso_string(dtm) << std::endl;

    if (ctID != m_adjCtID && !m_pxs.empty()) {

        /*
          enter here ONLY if 
          we are spooling NEW CT AT the rollover bar.
        */

        // remove rollover date data for old contract
        // so that we do not duplicate data points in the rollover date
        m_pxs.pop_back(); 
        m_dtms.pop_back();
        m_vols.pop_back();
        if (!m_pxs.empty())
        {
            const float clSpread = close - m_prevClose;
            BarBuff::iterator i = m_pxs.begin();
            BarBuff::iterator e = m_pxs.end();
            DtmsBuff::iterator j = m_dtms.begin();
            IntBuff::iterator k = m_vols.begin();

            // as we removed last data for old contract on rollover data,
            // this only calc before rollover date.
            for (; i != e; ++i, ++k, ++j) 
            {
                i->op += clSpread;
                i->cl += clSpread;
                i->hi += clSpread;
                i->lo += clSpread;
                /*
                std::cout << "... ctID = " << ctID << ", m_adjCtID = " << m_adjCtID 
                          << ", dtm = " << to_iso_string(*j) << std::endl;
                */
                // still before current bar
                doCalc(*j, i->op, i->hi, i->lo, i->cl, 
                       *k, clSpread, true);
            }
        }
        m_adjCtID = ctID;
    }

    hiNear = hiNearCalc.calc(dtm, high);
    loNear = loNearCalc.calc(dtm, low);
    hiMed = hiMedCalc.calc(dtm, high);
    loMed = loMedCalc.calc(dtm, low);
    hiFar = hiFarCalc.calc(dtm, high);
    loFar = loFarCalc.calc(dtm, low);
    
    hhHist.push_front(hiFar.px);
    llHist.push_front(loFar.px);

    prevVolumeMA = volumeMA;
    if (volume > 0) {
        volumeMA = static_cast<float>(volumeMACalc.calc(volume));
    }
    pxHistRaw.push_front(OHLC(open, high, low, close));
    enLongFilter.push_front(testEnterLongFilter());
    enShortFilter.push_front(testEnterShortFilter());

    
    // put doCalc at rear so that we print out the latest indicators
    doCalc(dtm, open, high, low, close, volume, 0, false);

    m_pxs.push_back(OHLC(open, high, low, close));
    m_dtms.push_back(dtm);
    m_vols.push_back(volume);

    m_prevClose = close;
    m_prevDtm = dtm;
    m_prevDt = dt;
    m_prevTm = tm;
}

void 
Strat::doCalc(const boost::posix_time::ptime dt_tm,
              const float openPx,
              const float highPx,
              const float lowPx,
              const float closePx,
              const int vol, 
              const float spread,
              const bool inAdj)
{
    
    DELTA = minTickSz * tickMult;

    /*
    hiNear = hiNearCalc.calc(dt_tm, highPx);
    loNear = loNearCalc.calc(dt_tm, lowPx);
    hiFar = hiFarCalc.calc(dt_tm, highPx);
    loFar = loFarCalc.calc(dt_tm, lowPx);
    */

    // always insert into hist buffer before calc
    pxHistAdj.push_front(OHLC(openPx, highPx, lowPx, closePx));

    allocAtr = allocAtrCalc.calc(openPx, highPx, lowPx, closePx);
    refAtr = refAtrCalc.calc(openPx, highPx, lowPx, closePx);
    longAaaAtr = longAaaAtrCalc.calc(openPx, highPx, lowPx, closePx);
    longAaa = doubleNA;
    if (!na(longAaaAtr) && longAaaAtr != 0)
    {
        double upMove = highPx - pxHistAdj[1].hi;
        double downMove = pxHistAdj[1].lo - lowPx;
        double pDM = (upMove > downMove && upMove > 0) ? upMove : 0;
        double nDM = (downMove > upMove && downMove > 0) ? downMove : 0; 
        double pDI = longAaaUpCalc.calc(pDM / longAaaAtr);
        double nDI = longAaaDownCalc.calc(nDM / longAaaAtr);

        if (pDI + nDI != 0)
        {
            longAaa = longAaaDiffCalc.calc(std::fabs((pDI - nDI) / (pDI + nDI)));
        } else 
        {
            longAaa = doubleNA;
        }

        /*
        logValues() << "\thighPx" << "\t" << highPx << "\t"
                    << "pxHistRawHi" << "\t" << pxHistAdj[1].hi << "\t"
                    << "pxHistRawLo" << "\t" << pxHistAdj[1].lo << "\t"
                    << "lowPx" << "\t  " << lowPx << "\t"  
                    << "upMove" << "\t" << upMove << "\t"
                    << "downMove" << "\t" << downMove << "\t"
                    << "pDM" << "\t" << pDM << "   \t" 
                    << "nDM" << "\t" << nDM << "\t"
                    << "pDI" << "\t" << pDI << "\t"
                    << "nDI" << "\t" << nDI << "\t" 
                    << "longAaa" << "\t" << longAaa << "\n";
        */
    } 

    shortAaaAtr = shortAaaAtrCalc.calc(openPx, highPx, lowPx, closePx);
    if (!na(shortAaaAtr) && shortAaaAtr != 0)
    {
        double upMove = highPx - pxHistAdj[1].hi;
        double downMove = pxHistAdj[1].lo - lowPx;
        double pDM = (upMove > downMove && upMove > 0) ? upMove : 0;
        double nDM = (downMove > upMove && downMove > 0) ? downMove : 0; 
        double pDI = shortAaaUpCalc.calc(pDM / shortAaaAtr);
        double nDI = shortAaaDownCalc.calc(nDM / shortAaaAtr);
        if (pDI + nDI != 0)
        {
            shortAaa = shortAaaDiffCalc.calc(std::fabs((pDI - nDI) / (pDI + nDI)));
        } else 
        {
            shortAaa = doubleNA;
        }
    } else 
    {
        shortAaa = doubleNA;
    }

    ricch = ricciHiCalc.calc(dt_tm, highPx);
    riccl = ricciLoCalc.calc(dt_tm, lowPx);
    ricca = ricciAtrCalc.calc(openPx, highPx, lowPx, closePx); 

    ind1Value = longAaa;
    ind2Value = (pxHistRaw[retPeriod].cl - pxHistRaw[0].lo) / pxHistRaw[retPeriod].cl;
    ind3Value = (pxHistRaw[0].hi - pxHistRaw[retPeriod].cl) / pxHistRaw[retPeriod].cl;

    if (!na(ind1Value))
    {
        //ind1LBValue = ind1LBCalc.calc(ind1Value);
        //ind1UBValue = ind1UBCalc.calc(ind1Value);
    }
    if (!na(ind2Value))
    {
        ind2LBValue = ind2LBCalc.calc(ind2Value);
        ind2UBValue = ind2UBCalc.calc(ind2Value);
    }
    if (!na(ind3Value))
    {
        ind3LBValue = ind3LBCalc.calc(ind3Value);
        ind3UBValue = ind3UBCalc.calc(ind3Value);
    }

    if (debugLevel >= 2 ||
        (debugLevel == 1  &&
         !inAdj &&
         dt_tm.date() >= boost::gregorian::from_string("2022/01/01")))
    {
        logValues() 
            << sep << to_iso_string(dt_tm)
            << sep << spread
            << sep << openPx
            << sep << highPx
            << sep << lowPx
            << sep << closePx

            << sep << hiNear.px
            << sep << loNear.px
            << sep << hiMed.px
            << sep << loMed.px
            << sep << hiFar.px
            << sep << loFar.px

            << sep << longAaa
            << sep << longAaaAtr

            << sep << shortAaa
            << sep << shortAaaAtr

            << sep << ricch.px
            << sep << riccl.px
            << sep << ricca

            << sep << allocAtr
            << sep << refAtr
            << sep << weight()
            << sep << equity()
            << sep << exchRate
            << sep << pointValue()
            << sep << static_cast<unsigned int>(equity()*riskFactor*weight() / (allocAtr*exchRate*pointValue()))
            << sep << static_cast<unsigned int>(floor(volumeMA*0.005))

            << sep << enLongFilter[0]
            << sep << enLongFilter[1]
            << sep << enShortFilter[0]
            << sep << enShortFilter[1]
            
            << sep << ind1LBValue
            << sep << ind1UBValue
            << sep << ind2LBValue
            << sep << ind2UBValue
            << sep << ind3LBValue
            << sep << ind3UBValue

            << sep << "\n"; 
    }                     
}

bool Strat::isNthBusDaysFromEOM(int n)
{
    date currDt = dtm.date();
    date eom = currDt.end_of_month();

    date prevNBD = eom;
    date_duration dd(1);

    if (eom.day_of_week() == 6) {
        // Saturday
        prevNBD = eom - dd;
    } else if (eom.day_of_week() == 0) {
        // Sunday
        prevNBD = eom - dd - dd;
    }
            
    while(n > 1) {
        prevNBD = prevNBD - dd;
        if(prevNBD.day_of_week() == 0 || prevNBD.day_of_week() == 6) {
            // weekends
            continue;
        } 
        n--;
    }

    return (currDt == prevNBD);
}


// return true iff this is n-th weekday counting forward from eod of month (not include eom).
//           n = 1:   if today is weekday, return today, otherwise, the first weekday before today
//              

bool Strat::isNthBusDaysSinceBOM(int n)
{
    date currDt = dtm.date();
    date bom = date(currDt.year(), currDt.month(), 1);

    date_duration dd(1);
    date nextNBD = bom + dd;

    if (bom.day_of_week() == 6) {
        // Saturday
        nextNBD = bom + dd + dd;
    } else if (bom.day_of_week() == 0) {
        // Sunday
        nextNBD = bom + dd;
    }
            
    while(n > 1) {
        nextNBD = nextNBD + dd;
        if(nextNBD.day_of_week() == 0 || nextNBD.day_of_week() == 6) {
            // weekends
            continue;
        } 
        n--;
    }

    return (currDt == nextNBD);
}

int Strat::numBusDaysBetween(date beginDate, date endDate) {
    if (beginDate > endDate) return 0;
    int ret = 0;
    date dt = beginDate;
    date_duration dd(1);
    while (dt <= endDate) {
        if (dt.day_of_week() >= 1 && dt.day_of_week() <= 5) {
            ret++;
        }
        dt += dd;
    }
    return ret;
}

/* virtual */ 
void 
Strat::trade() 
{
    double dnc = equity()*riskFactor*weight() / (allocAtr*exchRate*pointValue());
    unsigned int nc = static_cast<unsigned int>(dnc);

    /*
    date currDt = dtm.date();
    int day = currDt.day();
    date bom = date(currDt.year(), currDt.month(), 1);
    //if (isNthBusDaysFromEOM(1) || numBusDaysBetween(bom, dtm.date()) < 9) {
    if (isNthBusDaysFromEOM(1) || day < 12) {
        if (mktID >= 3100 && mktID <= 3600) {
            // so for financial
            nc = int(nc * 1.25);
        } else if (mktID >= 4100) {
            // for metal 
            nc = int(nc * 1.25);
        }
    } else {
        nc = int(nc * 0.75);
    }
    //*/
    
    unsigned int volumeCap = static_cast<unsigned int>(floor(0.01 * (volumeMA)));

    //*
    if (volumeCap == 0) {
        nc = 1;
    } else if (nc > volumeCap) {
        nc = volumeCap;
    }
    //*/    

    if (nc <= 0)
    {
        nc = 1;
    }

    // TODO -- for test
    //nc = 1;

    /*
    bool longCond = testEnterLongFilter();    
    bool shortCond = testEnterShortFilter();
    */

    float enLongPx = roundToTic(static_cast<float>(hiFar.px + DELTA));
    float enShortPx = roundToTic(static_cast<float>(loFar.px - DELTA));

    float exLongPx = roundToTic(static_cast<float>(loNear.px - DELTA));
    float exShortPx = roundToTic(static_cast<float>(hiNear.px + DELTA));

    // ind2 for buy to exit short, ind3 for sell to exit long
    float shortPftLevel = roundToTic(static_cast<float>(pxHistRaw[0].cl * (1 - ind2LBValue)));
    float longPftLevel = roundToTic(static_cast<float>(pxHistRaw[0].cl * (1 + ind3LBValue)));

    /*
    float exLongPx = (mktID / 1000 == 3 || mktID == 4303) ?
        roundToTic(static_cast<float>(loMed.px - DELTA))
        : roundToTic(static_cast<float>(loNear.px - DELTA));
    float exShortPx = (mktID / 1000 == 3 || mktID == 4303) ?
        roundToTic(static_cast<float>(hiMed.px + DELTA))
        : roundToTic(static_cast<float>(hiNear.px + DELTA));
    //*/

    // ---alt0---
    // turn off opptunistic limit by setting this up very high
    // turn tis one by setting it to 1
    int lmtMult = 1;
    if (isLong())
    {
        longPosAge++;
        longTimer++;
        // ---alt2---: restart timer once detecting strong trend
        //       turn off if not restart timer. 
        if (low > pxHistRaw[1].lo && high > pxHistRaw[1].hi) {
            // market moving up and we go with trend 
            //longTimer = 0;
        }
        
        unsigned long longSize = static_cast<unsigned long>(longPosSize());
        //logValues() << sep << "longPosSz = " << longSize << "\n";
        assert(longSize != 0);

        // ---alt1--- close position on failed breakout
        // alt1.1: no close < open condition
        // alt1.2: with close < open (exit only when brk bar in wrong direction)
        /*
        if (close < hhHist[1] + DELTA && longPosAge <= 1) { // && close < open) {
            exitLong("SX_MCC", longSize, otMOC(), tifCurrBar());
        } else
        //*/
            
        // ---alt2---
        // not forcing to exit if entry filter is still true
        if (longPosAge >= maxPosAge - 1 && longSize != 0) 
        {

            /*
            oco(
                orderOCO(
                    orderExitLong("SX_STP2", longSize, otSTP(exLongPx)), 
                    orderExitLong("SX_LMT2", longSize, 
                                  otLMT(roundToTic(
                                            static_cast<float>(close + lmtMult*tpfMult*allocAtr))))
                    ),
                orderExitLong("SX_MOC", longSize, otMOC()));
            //*/
            //*
            oco(orderExitLong("SX_LOF", static_cast<unsigned int>(longSize),
                              otSTP(exLongPx)),
                orderExitLong("SX_MOC", static_cast<unsigned int>(longSize),
                              otMOC()));
            //*/

            
        } else {
            // timer not fired yet *OR* longCond still holds, do NOT force to close.
            //*
            //if (longPosAge == 1)
            {
                oco(
                    orderExitLong("SX_STP1", longSize, otSTP(exLongPx)), 
                    orderExitLong("SX_LMT1", longSize, otLMT(longPftLevel)));
            }
            //else
            //*/
            {
                //    exitLong("SX_STP1", longSize, otSTP(exLongPx)); 
            }
        }
    }
    else if (enLongFilter[0])
        //else if (enLongFilter[1] && (low >= pxHistRaw[1].lo || open < close) && high > hhHist[1])
    {
        // we are not in long position
        /*
        if (!isShort()) {
            // simulator seems not like oto when we have a short position
            // (with a stop loss buy order)
            oto(
                orderBuy("LE_HIF", nc, otSTP(enLongPx)),
                orderExitLong("SX_LMT0", nc, otLMT(
                                  roundToTic(
                                      static_cast<float>(
                                          enLongPx + lmtMult*3*tpfMult*refAtr))))
            );
        }
        //else
        /*/ 
        {
            buy("LE_HIF", nc, otSTP(enLongPx));
            /*
            oto(orderBuy("LE_MOC", nc, otMOC(), tifCurrBar()),
                orderExitLong("SX_STP1", nc, otSTP(exLongPx)));
            //*/
        }
        //*/
    }

    if (isShort())
    {
        shortPosAge++;
        shortTimer++;
        
        // ---alt2--- (turn off if not update timer)
        if (high < pxHistRaw[1].hi && low < pxHistRaw[1].lo) 
        {
            // move lower and go with trend
            //shortTimer = 1;
        }
        
        unsigned long shortSize = static_cast<unsigned long>(shortPosSize());
        //logValues() << sep << "shortPosSz = " << sep << shortSize << "\n"; 
        assert(shortSize != 0);

        // ---alt1---
        // alt1.1: no breakout bar direction test (no close > open)
        // alt1.2: exit only when breakout bar in wrong direction (keep close > open test)
        /*
        if (close > llHist[1] - DELTA && shortPosAge <= 1) { // && close > open) {
            exitShort("LX_MCC", shortSize, otMOC(), tifCurrBar());
        } else
        //*/

        // ---alt2---
        // not forcing exit if entry filter is still true
        if (shortPosAge >= maxPosAge - 1 && shortSize != 0)
        {
            /*
            oco(
                orderOCO(
                    orderExitShort("LX_STP2", shortSize, otSTP(exShortPx)),
                    orderExitShort("LX_LMT2", static_cast<unsigned int>(shortSize), 
                                   otLMT(roundToTic(static_cast<float>(close - lmtMult*tpfMult*allocAtr))))
                    //orderExitShort("LX_STP2", shortSize, otSTP(exShortPx))
                    ),
                orderExitShort("LX_MOC", shortSize, otMOC()));
            //*/
            //*
            oco(
                orderExitShort("LX_HOF", static_cast<unsigned int>(shortSize),
                               otSTP(exShortPx)),
                orderExitShort("LX_MOC", static_cast<unsigned int>(shortSize),
                               otMOC()));
            //*/

        }
        else {
            //*
            //if (shortPosAge == 1)
            {
                oco(
                    orderExitShort("LX_STP1", shortSize, otSTP(exShortPx)),
                    orderExitShort("LX_LMT1", static_cast<unsigned int>(shortSize), 
                                   otLMT(shortPftLevel)));

            }
            //else {
            //*/
            //*
            //exitShort("LX_STP1", shortSize, otSTP(exShortPx)); 
            //*/
            //}
        }
        
    }
    else if (enShortFilter[0])  
        //else if (enShortFilter[1] && (high <= pxHistRaw[1].hi || open > close) && low < llHist[1])
    {
        // we are not in long position
        /*
        if (!isLong()) {
            oto(orderSell("SE_LOF", nc, otSTP(enShortPx)),
                orderExitShort("LX_LMT0", static_cast<unsigned int>(nc), 
                               otLMT(roundToTic(
                                         static_cast<float>(
                                             enShortPx - lmtMult*3*tpfMult*refAtr)))));
        }
        //*/
        //else
        //*/
        {
            //logValues() << "SE_LOF at " << enShortPx << endl;
            sell("SE_LOF", nc, otSTP(enShortPx));
            /*
            oto(orderSell("SE_MOC", nc, otMOC(), tifCurrBar()),
                orderExitShort("LX_STP1", nc, otSTP(exShortPx)));
            //*/
        }
        //*/
                    
    }

    /*
    logValues() << sep << "pxHistRaw[0].hi = " << pxHistRaw[0].hi << sep 
                << "hhHist[1] = " << hhHist[1] << "\n";
    logValues() << sep << "pxHistRaw[0].lo = " << pxHistRaw[0].lo << sep 
                << "llHist[1] = " << llHist[1] << "\n";
    */
}


// from Patrev
bool Strat::testEnterShortFilter()
{
    date currDt = dtm.date();
    int day = currDt.day();
    date bom = date(currDt.year(), currDt.month(), 1);
    
    bool ind1Cond =  
        ((ind1UBPct >= 100 || ind1Value < ind1UBValue) && 
         (ind1LBPct <= 0 || ind1Value >= ind1LBValue));
    bool ind2Cond =  
        ((ind2UBPct >= 100 || ind2Value < ind2UBValue) && 
         (ind2LBPct <= 0 || ind2Value >= ind2LBValue));
    bool ind3Cond =  
        ((ind3UBPct >= 100 || ind3Value < ind3UBValue) && 
         (ind3LBPct <= 0 || ind3Value >= ind3LBValue));

    bool fresh = (pxHistRaw.size() >= 1 && hhHist.size() >= 2 &&
                  pxHistRaw[0].hi < hhHist[1] + DELTA);

    bool cheapBreak = (hiFar.px - close) <= alpha * refAtr;

    // if current bar is hiFar, extrPxCnt = pxCnt - 1
    // minus 1 to make it zero based, hiFarAge == 0 if curr bar is hiFar. 
    int hiFarAge = hiFarCalc.pxCnt() - hiFarCalc.extrPxCnt() - 1;

    //fresh1 => pxHistRaw[0].hi < hhHist[1] 
    bool sfresh1 =  (pxHistRaw.size() >= hiFarCalc.period() + 1 &&
                     hiFarAge >= 1 &&
                     hiFar.px > pxHistRaw[hiFarAge+1].hi &&
                     hiFar.px > pxHistRaw[hiFarAge-1].hi
        );
    
    bool sfresh2 =  (pxHistRaw.size() >= hiFarCalc.period() + 2 &&
                     hiFarAge >= 2 &&
                     hiFar.px > pxHistRaw[hiFarAge+1].hi &&
                     hiFar.px > pxHistRaw[hiFarAge+2].hi &&
                     hiFar.px > pxHistRaw[hiFarAge-1].hi &&
                     hiFar.px > pxHistRaw[hiFarAge-2].hi
        );

    bool sfresh3 =  (pxHistRaw.size() >= hiFarCalc.period() + 3 &&
                     hiFarAge >= 3 &&
                     hiFar.px > pxHistRaw[hiFarAge+1].hi &&
                     hiFar.px > pxHistRaw[hiFarAge+2].hi &&
                     hiFar.px > pxHistRaw[hiFarAge+3].hi &&
                     hiFar.px > pxHistRaw[hiFarAge-1].hi &&
                     hiFar.px > pxHistRaw[hiFarAge-2].hi && 
                     hiFar.px > pxHistRaw[hiFarAge-3].hi 
        );
    
    bool sfresh4 =  (pxHistRaw.size() >= hiFarCalc.period() + 4 &&
                     hiFarAge >= 4 &&
                     hiFar.px > pxHistRaw[hiFarAge+1].hi &&
                     hiFar.px > pxHistRaw[hiFarAge+2].hi &&
                     hiFar.px > pxHistRaw[hiFarAge+3].hi &&
                     hiFar.px > pxHistRaw[hiFarAge+4].hi &&
                     hiFar.px > pxHistRaw[hiFarAge-1].hi &&
                     hiFar.px > pxHistRaw[hiFarAge-2].hi && 
                     hiFar.px > pxHistRaw[hiFarAge-3].hi &&
                     hiFar.px > pxHistRaw[hiFarAge-3].hi
        );

    bool fresh1 =  (pxHistRaw.size() >= hiFarCalc.period() + 1 &&
                    hiFarAge >= 1 &&
                    hiFar.px >= pxHistRaw[hiFarAge+1].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge-1].hi
        );
    
    bool fresh2 =  (pxHistRaw.size() >= hiFarCalc.period() + 2 &&
                    hiFarAge >= 2 &&
                    hiFar.px >= pxHistRaw[hiFarAge+1].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge+2].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge-1].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge-2].hi
        );

    bool lfresh2 = (pxHistRaw.size() >= hiFarCalc.period() + 2 &&
                    hiFar.px >= pxHistRaw[hiFarAge+1].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge+2].hi
        );
    
    bool fresh3 =  (pxHistRaw.size() >= hiFarCalc.period() + 3 &&
                    hiFarAge >= 3 &&
                    hiFar.px >= pxHistRaw[hiFarAge+1].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge+2].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge+3].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge-1].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge-2].hi && 
                    hiFar.px >= pxHistRaw[hiFarAge-3].hi 
        );
    bool lfresh3 =  (pxHistRaw.size() >= hiFarCalc.period() + 3 &&
                    hiFar.px >= pxHistRaw[hiFarAge+1].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge+2].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge+3].hi 
        );
    
    bool fresh4 =  (pxHistRaw.size() >= hiFarCalc.period() + 4 &&
                    hiFarAge >= 4 &&
                    hiFar.px >= pxHistRaw[hiFarAge+1].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge+2].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge+3].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge+4].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge-1].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge-2].hi && 
                    hiFar.px >= pxHistRaw[hiFarAge-3].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge-3].hi
        );
    bool lfresh4 =  (pxHistRaw.size() >= hiFarCalc.period() + 4 &&
                    hiFar.px >= pxHistRaw[hiFarAge+1].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge+2].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge+3].hi &&
                    hiFar.px >= pxHistRaw[hiFarAge+4].hi
        );

    // 0=Sunday, 1=Monday, 2=Tuesday
    //bool dowCond = (currDt.day_of_week() >= 1 && currDt.day_of_week() <= 4);
    bool retValue = (hiFarAge <= extrDistance && lfresh3 && ind1Cond); 
    return retValue;
}


// Patrev
bool Strat::testEnterLongFilter()
{
    date currDt = dtm.date();
    int day = currDt.day();
    date bom = date(currDt.year(), currDt.month(), 1);
    
    bool ind1Cond =  
        ((ind1UBPct >= 100 || ind1Value < ind1UBValue) && 
         (ind1LBPct <= 0 || ind1Value >= ind1LBValue));
    bool ind2Cond =  
        ((ind2UBPct >= 100 || ind2Value < ind2UBValue) && 
         (ind2LBPct <= 0 || ind2Value >= ind2LBValue));
    bool ind3Cond =  
        ((ind3UBPct >= 100 || ind3Value < ind3UBValue) && 
         (ind3LBPct <= 0 || ind3Value >= ind3LBValue));
    
    bool fresh = (pxHistRaw.size() >= 2 && llHist.size() > 2 &&
                  pxHistRaw[0].lo > llHist[1] - DELTA);

    bool cheapBreak = (close - loFar.px) <= beta * refAtr;
    
    // extrPxCnt <= pxCnt - 1, if current bar is loFar,
    // extrPxCnt = pxCnt - 1
    int loFarAge = loFarCalc.pxCnt() - loFarCalc.extrPxCnt() - 1;

    // fresh1 => pxHistRaw[0].lo > llHist[1]  
    bool sfresh1 = (pxHistRaw.size() >= loFarCalc.period() + 1 &&
              loFarAge >= 1 &&
              loFar.px < pxHistRaw[loFarAge+1].lo &&
              loFar.px < pxHistRaw[loFarAge-1].lo
        );    
    
    bool sfresh2 = (pxHistRaw.size() >= loFarCalc.period() + 2 &&
                    loFarAge >= 2 &&
                    loFar.px < pxHistRaw[loFarAge+1].lo &&
                    loFar.px < pxHistRaw[loFarAge+2].lo); // &&
    //              loFar.px < pxHistRaw[loFarAge-1].lo &&
    //        loFar.px < pxHistRaw[loFarAge-2].lo
    //  );

    bool sfresh3 = (pxHistRaw.size() >= loFarCalc.period() + 2 &&
              loFarAge >= 3 &&
              loFar.px < pxHistRaw[loFarAge+1].lo &&                    
              loFar.px < pxHistRaw[loFarAge+2].lo &&
              loFar.px < pxHistRaw[loFarAge+3].lo &&
              loFar.px < pxHistRaw[loFarAge-1].lo &&
              loFar.px < pxHistRaw[loFarAge-2].lo &&
              loFar.px < pxHistRaw[loFarAge-3].lo
        );

    bool sfresh4 = (pxHistRaw.size() >= loFarCalc.period() + 2 &&
                    loFarAge >= 4 &&                    
                    loFar.px < pxHistRaw[loFarAge+1].lo &&                    
                    loFar.px < pxHistRaw[loFarAge+2].lo &&                    
                    loFar.px < pxHistRaw[loFarAge+3].lo &&
                    loFar.px < pxHistRaw[loFarAge+4].lo &&
                    loFar.px < pxHistRaw[loFarAge-1].lo &&
                    loFar.px < pxHistRaw[loFarAge-2].lo &&
                    loFar.px < pxHistRaw[loFarAge-3].lo &&
                    loFar.px < pxHistRaw[loFarAge-4].lo
        );

    bool fresh1 = (pxHistRaw.size() >= loFarCalc.period() + 1 &&
              loFarAge >= 1 &&
              loFar.px <= pxHistRaw[loFarAge+1].lo &&
              loFar.px <= pxHistRaw[loFarAge-1].lo
        );    
    
    bool fresh2 = (pxHistRaw.size() >= loFarCalc.period() + 2 &&
              loFarAge >= 2 &&
              loFar.px <= pxHistRaw[loFarAge+1].lo &&
              loFar.px <= pxHistRaw[loFarAge+2].lo &&
              loFar.px <= pxHistRaw[loFarAge-1].lo &&
              loFar.px <= pxHistRaw[loFarAge-2].lo
        );

    bool lfresh2 = (pxHistRaw.size() >= loFarCalc.period() + 2 &&
              loFar.px <= pxHistRaw[loFarAge+1].lo &&
              loFar.px <= pxHistRaw[loFarAge+2].lo
        );
    
    bool fresh3 = (pxHistRaw.size() >= loFarCalc.period() + 3 &&
              loFarAge >= 3 &&
              loFar.px <= pxHistRaw[loFarAge+1].lo &&                    
              loFar.px <= pxHistRaw[loFarAge+2].lo &&
              loFar.px <= pxHistRaw[loFarAge+3].lo &&
              loFar.px <= pxHistRaw[loFarAge-1].lo &&
              loFar.px <= pxHistRaw[loFarAge-2].lo &&
              loFar.px <= pxHistRaw[loFarAge-3].lo 
        );

    bool lfresh3 = (pxHistRaw.size() >= loFarCalc.period() + 3 &&
              loFar.px <= pxHistRaw[loFarAge+1].lo &&                    
              loFar.px <= pxHistRaw[loFarAge+2].lo &&
              loFar.px <= pxHistRaw[loFarAge+3].lo 
        );

    bool fresh4 = (pxHistRaw.size() >= loFarCalc.period() + 4 &&
                    loFarAge >= 4 &&                    
                    loFar.px <= pxHistRaw[loFarAge+1].lo &&                    
                    loFar.px <= pxHistRaw[loFarAge+2].lo &&                    
                    loFar.px <= pxHistRaw[loFarAge+3].lo &&
                    loFar.px <= pxHistRaw[loFarAge+4].lo &&
                    loFar.px <= pxHistRaw[loFarAge-1].lo &&
                    loFar.px <= pxHistRaw[loFarAge-2].lo &&
                    loFar.px <= pxHistRaw[loFarAge-3].lo &&
                    loFar.px <= pxHistRaw[loFarAge-4].lo 
        );
    bool lfresh4 = (pxHistRaw.size() >= loFarCalc.period() + 4 &&
                    loFar.px <= pxHistRaw[loFarAge+1].lo &&                    
                    loFar.px <= pxHistRaw[loFarAge+2].lo &&                    
                    loFar.px <= pxHistRaw[loFarAge+3].lo &&
                    loFar.px <= pxHistRaw[loFarAge+4].lo 
        );
    // 0=Sunday, 1=Monday, 2=Tuesday
    bool dowCond = (currDt.day_of_week() >= 1 && currDt.day_of_week() <= 4);
    bool retValue = (loFarAge >= extrDistance && lfresh3 && ind1Cond);
    return retValue;
}


void Strat::filled(const pirate::tr::to::Trade &aTrade)
{
    const TimeInForce * const ordTifPtr = aTrade.order()->tif();
    if (typeid(*ordTifPtr) == typeid(TifRoll)) return; 

    if(aTrade.order()->side() == pirate::tr::to::Order::Buy)
    {
        if (aTrade.order()->entryExit() == pirate::tr::to::Order::Entry)
        {
            longPosAge = 0;
            longTimer = 0;
            //highestSinceLong = high;
            //lowestSinceLong = low;
        } else if (aTrade.order()->entryExit() == pirate::tr::to::Order::Exit)
        {
            shortPosAge = intNA;
            shortTimer = intNA;
            //highestSinceShort = naValue<float>();
            //lowestSinceShort = naValue<float>(); 
        }
    } else if (aTrade.order()->side() == pirate::tr::to::Order::Sell)
    {
        if (aTrade.order()->entryExit() == pirate::tr::to::Order::Entry)
        {
            shortPosAge = 0;
            shortTimer = 0;
            //highestSinceShort = high;
            //lowestSinceShort = low;
        } else if (aTrade.order()->entryExit() == pirate::tr::to::Order::Exit)
        {
            longPosAge = intNA;
            longTimer = intNA;
            //highestSinceLong = naValue<float>();
            //lowestSinceLong = naValue<float>(); 
        }
    }       
}


/* virtual */ 
void Strat::rollCleanUp() {
    m_adjDtm = m_prevDtm;
    m_adjDt = m_prevDt;
    m_adjTm = m_prevTm;

    hiNear = 
        loNear =
        hiMed =
        loMed = 
        hiFar = 
        loFar = 
        ricch = 
        riccl = TmPx();
    ricca = 
        longAaa = 
        longAaaAtr = 
        shortAaa = 
        shortAaaAtr = 
        ind1Value = 
        ind2Value =
        ind3Value = doubleNA;

    /*
        ind1LBValue = 
        ind1UBValue = 
        ind2LBValue = 
        ind2UBValue = 
        ind3LBValue = 
        ind3UBValue = doubleNA;
    */
    
    hiNearCalc.cleanup();
    loNearCalc.cleanup();
    hiMedCalc.cleanup();
    loMedCalc.cleanup();
    hiFarCalc.cleanup();
    loFarCalc.cleanup();

    longAaaUpCalc.cleanup();
    longAaaDownCalc.cleanup();
    longAaaDiffCalc.cleanup();
    longAaaAtrCalc.cleanup();

    shortAaaUpCalc.cleanup();
    shortAaaDownCalc.cleanup();
    shortAaaDiffCalc.cleanup();
    shortAaaAtrCalc.cleanup();

    ricciHiCalc.cleanup();
    ricciLoCalc.cleanup();
    ricciAtrCalc.cleanup();

    allocAtrCalc.cleanup();

    //ind1LBCalc.cleanup();
    //ind1UBCalc.cleanup();
    //ind2LBCalc.cleanup();
    //ind2UBCalc.cleanup();
    //ind3LBCalc.cleanup();
    //ind3UBCalc.cleanup();

    pxHistAdj.clear();
    pxHistRaw.clear();
    hhHist.clear();
    llHist.clear();
}

