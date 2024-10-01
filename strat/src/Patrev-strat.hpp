#ifndef Patrev__STRAT_HPP
#define Patrev__STRAT_HPP

#include <pirate/tr/bar/BarStrat.hpp>

#include <pirate/ind/Indicator.hpp>
#include <pirate/ind/TmPx.hpp>
#include <pirate/ind/HighTm.hpp>
#include <pirate/ind/LowTm.hpp>
#include <pirate/ind/Var.hpp>
#include <pirate/ind/EMA.hpp>
#include <pirate/ind/SMA.hpp>
#include <pirate/ind/RSI.hpp>
#include <pirate/ind/ATR.hpp>
#include <pirate/ind/Pctile.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <pirate/tr/data/OHLC.hpp>
#include <pirate/data/CyclicBuffer.hpp>
#include <pirate/tr/Position.hpp>
#include <pirate/tr/OMS.hpp>
#include <pirate/tr/Orders.hpp>
#include <pirate/tr/to/Order.hpp>
#include <pirate/tr/to/OrderOCO.hpp>
#include <pirate/tr/to/OrderOTO.hpp>
#include <pirate/tr/to/Trade.hpp>
#include <pirate/tr/Trader.hpp>
#include <pirate/tr/to/LMT.hpp>
#include <pirate/tr/to/LMTT.hpp>
#include <pirate/tr/to/STP.hpp>
#include <pirate/tr/to/STPT.hpp>
#include <pirate/tr/to/TifGTB.hpp>
#include <vector>
#include <climits>
#include <cstddef>
#include <string>


/*! Sample high/low breakout strategy implementation.
  For more default on PIRATE's pirate::tr::bar::Strat
  interface please consult its header or API doc.
 */
struct Strat : pirate::tr::bar::BarStrat {
    Strat(const pirate::ind::period_type aNearPeriod,
          const pirate::ind::period_type aMedPeriod,
          const pirate::ind::period_type aFarPeriod,
          const pirate::ind::period_type aLongAaaPeriod,
          const pirate::ind::period_type aShortAaaPeriod,
          const pirate::ind::period_type aRicciPeriod,
          const pirate::ind::period_type aAllocAtrPeriod,
          const pirate::ind::period_type aRefAtrPeriod,
          const pirate::ind::period_type aPctPeriod,
          const pirate::ind::period_type aExtrDistance,
          
          const int maxPosAge,
          
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
          const float aBeta
          
        );
    
    /* virtual */ 
    void calc();
    void doCalc(const boost::posix_time::ptime dt_tm,
                const float openPx,
                const float highPx,
                const float lowPx,
                const float closePx,
                const int vol, 
                const float spread,
                const bool logOn);

    /* virtual */
    void begin();
    /* virtual */ 
    void trade();
    /* virtual */ 
    void rollCleanUp();
    /* virtual */ void filled(const pirate::tr::to::Trade &aTrade);
    bool testEnterLongFilter();
    bool testEnterShortFilter();

    float roundToTic(const float aPx) const 
    {
        return static_cast<float>(minTickSz * std::floor(aPx / minTickSz + 0.5));
    }

    float roundToTicUp(const float aPx) const 
    {
        return static_cast<float>(minTickSz * std::ceil(aPx / minTickSz));
    }
    
    float roundToTicDown(const float aPx) const 
    {
        return static_cast<float>(minTickSz * std::floor(aPx / minTickSz));
    }
    bool Strat::isNthBusDaysSinceBOM(int n);
    bool Strat::isNthBusDaysFromEOM(int n);
    int Strat::numBusDaysBetween(boost::gregorian::date beginDate,
                                 boost::gregorian::date endDate);
private:
    static const int OPEN = 0;
    static const int HIGH = 1;
    static const int LOW = 2;
    static const int CLOSE = 3;

    typedef pirate::data::CyclicBuffer<pirate::tr::data::OHLC> BarBuff;
    typedef pirate::data::CyclicBuffer<double> DoubleBuff; 
    typedef pirate::data::CyclicBuffer<float> FloatBuff;
    typedef pirate::data::CyclicBuffer<pirate::ind::TmPx> TmPxBuff;
    typedef pirate::data::CyclicBuffer<int> IntBuff;
    typedef pirate::data::CyclicBuffer<bool> BoolBuff;
    typedef pirate::data::CyclicBuffer<boost::posix_time::ptime> DtmsBuff;

    int intNA;
    float floatNA;
    double doubleNA;

    pirate::ind::HighTmCalc hiNearCalc;
    pirate::ind::HighTmCalc hiMedCalc;
    pirate::ind::HighTmCalc hiFarCalc;
    pirate::ind::LowTmCalc loNearCalc;
    pirate::ind::LowTmCalc loMedCalc;
    pirate::ind::LowTmCalc loFarCalc;

    pirate::ind::EMACalc longAaaUpCalc; // UpMove
    pirate::ind::EMACalc longAaaDownCalc; // DownMove
    pirate::ind::EMACalc longAaaDiffCalc; // difference of above two   
    pirate::ind::ATRCalc longAaaAtrCalc;

    pirate::ind::EMACalc shortAaaUpCalc; //     UpMove
    pirate::ind::EMACalc shortAaaDownCalc; // DownMove
    pirate::ind::EMACalc shortAaaDiffCalc; // difference of above two   
    pirate::ind::ATRCalc shortAaaAtrCalc;

    pirate::ind::HighTmCalc ricciHiCalc;
    pirate::ind::LowTmCalc ricciLoCalc;
    pirate::ind::ATRCalc ricciAtrCalc;

    pirate::ind::ATRCalc allocAtrCalc;
    pirate::ind::ATRCalc refAtrCalc;

    pirate::ind::SMACalc volumeMACalc;
    
    pirate::ind::PctileCalc ind1LBCalc;
    pirate::ind::PctileCalc ind1UBCalc;
    pirate::ind::PctileCalc ind2LBCalc;
    pirate::ind::PctileCalc ind2UBCalc;
    pirate::ind::PctileCalc ind3LBCalc;
    pirate::ind::PctileCalc ind3UBCalc;

    pirate::ind::TmPx hiNear;
    pirate::ind::TmPx loNear;
    pirate::ind::TmPx hiMed;
    pirate::ind::TmPx loMed;
    pirate::ind::TmPx hiFar;
    pirate::ind::TmPx loFar;
    
    boost::posix_time::time_duration settleTm;
    int settleTmInt;
    int stpLossPeriod;
    int retPeriod;
    
    int longPosAge;
    int shortPosAge;
    int longTimer;
    int shortTimer;

    int farPeriod;
    int medPeriod;
    int nearPeriod;
    int extrDistance;
    
    double longAaa;
    double longAaaAtr;
    double shortAaa;
    double shortAaaAtr;

    pirate::ind::TmPx ricch;
    pirate::ind::TmPx riccl;
    double ricca;
    double allocAtr;
    double refAtr;

    double riccaAtPeak;
    double riccaAtTrough;

    float prevVolumeMA;
    float volumeMA;

    int maxPosAge;

    double ind1LBPct;
    double ind1UBPct;
    double ind2LBPct;
    double ind2UBPct;
    double ind3LBPct;
    double ind3UBPct;
    double ind1Value;
    double ind2Value;
    double ind3Value;
    double ind1LBValue;
    double ind1UBValue;
    double ind2LBValue;
    double ind2UBValue;
    double ind3LBValue;
    double ind3UBValue;

    BarBuff pxHistRaw;   // history of unajusted contracts
    BarBuff pxHistAdj;  // history of ajusted front contracts only
    BoolBuff enLongFilter;
    BoolBuff enShortFilter;
    
    DoubleBuff hhHist;
    DoubleBuff llHist;

    double minTickSz;

    int numOfUnits;
    float riskFactor;
    float tickMult;
    float tpfMult;
    float alpha;
    float beta;

    float DELTA;

  //! CyBuf of closing prices
    BarBuff m_pxs;
    DtmsBuff m_dtms;
    IntBuff m_vols;
    //! Old front contract's prior closing price, used for spread calc.
    float m_prevClose;
    //! The last contract ID that we spre   ad adjusted.
    pirate::tr::ct_id_type m_adjCtID;

#ifdef _MSC_VER
#  pragma warning( push )
#    pragma warning( disable : 4251 ) // disable missing dll-interface warning
#endif
    //! The previous business day's date-time.
    boost::posix_time::ptime m_prevDtm;

    //! The adjustment date-time (which is one business day prior roll date).
    boost::posix_time::ptime m_adjDtm;
#ifdef _MSC_VER
#  pragma warning( pop ) 
#endif

    //! The previous date (just to make it more debug friendly).
    pirate::tr::bar::dt_type m_prevDt;

    //! The adjustment date (just to make it more debug friendly).
    pirate::tr::bar::dt_type m_adjDt;

    //! The previous time (just to make it more debug friendly).
    pirate::tr::bar::tm_type m_prevTm;

    //! The adjustment time (just to make it more debug friendly).
    pirate::tr::bar::tm_type m_adjTm;


};

#endif /* STRAT_HPP */
