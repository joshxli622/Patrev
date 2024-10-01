#include "Patrev-factory.hpp"
using pirate::ind::period_type;

#include "Patrev-strat.hpp"
#include "Patrev-version.hpp"
#include <pirate/ver/Versions.hpp>
#include <pirate/tr/Trader.hpp>
#include <pirate/ind/PirateToolsVersion.hpp>

#include <pirate/tr/cfg/MktDef.hpp>
using pirate::tr::cfg::MktDef;

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include <pirate/boost/program_options.hpp>
namespace po = boost::program_options;

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;

#include <stdexcept>
using std::runtime_error;

#include <iostream>
using std::cout;
using std::endl;
using pirate::ind::naValue;

Factory::Factory()
    : pirate::tr::bar::BarStratFactory(),
      m_nearPeriod(0),
      m_medPeriod(0),
      m_farPeriod(0), 
      m_longAaaPeriod(0),
      m_shortAaaPeriod(0),
      m_ricciPeriod(0),
      m_allocAtrPeriod(0),
      m_refAtrPeriod(0),
      m_pctPeriod(0),
      m_extrDistance(0),

      m_maxPosAge(naValue<int>()),

      m_ind1LBPct(0.0),
      m_ind1UBPct(100.0),
      m_ind2LBPct(0),
      m_ind2UBPct(100.0),
      m_ind3LBPct(0),
      m_ind3UBPct(100.0),

      m_numOfUnits(1),
      m_riskFactor(0.006),
      m_tickMult(0.0),
      m_tpfMult(1.0),
      m_alpha(1.0),
      m_beta(1.0)

{}

/* virtual */ void Factory::prep(po::options_description &args) {
    po::options_description argsMandCmdLn(" Strategy Mandatory:");
    argsMandCmdLn.add_options()
        ("strat.nearPeriod", 
         po::value<pirate::ind::period_type>(), 
         "Near Period Length for breakout.")
        
        ("strat.medPeriod", 
         po::value<pirate::ind::period_type>(), 
         "Med Period Length for breakout.")
        
        ("strat.farPeriod", 
         po::value<pirate::ind::period_type>(), 
         "Far Period Length for breakout.")

        ("strat.longAaaPeriod", 
         po::value<pirate::ind::period_type>(), 
         "Period Length for enter long ADX computation.")

        ("strat.shortAaaPeriod", 
         po::value<pirate::ind::period_type>(), 
         "Period Length for enter short ADX computation.")

        ("strat.ricciPeriod", 
         po::value<pirate::ind::period_type>(), 
         "Period Length for ricci computation.")

        ("strat.allocAtrPeriod", 
         po::value<pirate::ind::period_type>(), 
         "Period Length for allocation ATR computation.")

        ("strat.refAtrPeriod", 
         po::value<pirate::ind::period_type>(), 
         "Period Length for Long Term ATR computation.")

        ("strat.pctPeriod", 
         po::value<pirate::ind::period_type>(), 
         "Rolling Window Length for percentile computation.")

        ("strat.extrDistance", 
         po::value<pirate::ind::period_type>(), 
        "minimal distance between two extremes.")

        ("strat.maxPosAge", 
         po::value<int>(),
         "max position age")

        ("strat.ind1LBPct", 
         po::value<double>(),
         "Percentile Lower Bound for ind1")
        ("strat.ind1UBPct", 
         po::value<double>(),
         "Percentile Upper Bound for ind1")
        ("strat.ind2LBPct", 
         po::value<double>(),
         "Percentile Lower Bound for ind2")
        ("strat.ind2UBPct", 
         po::value<double>(),
         "Percentile Upper Bound for ind2")
        ("strat.ind3LBPct", 
         po::value<double>(),
         "Percentile Lower Bound for ind3")
        ("strat.ind3UBPct", 
         po::value<double>(),
         "Percentile Upper Bound for ind3")

        ("strat.numOfUnits", 
         po::value<int>(),
         "number of Trading Units.")
        ("strat.riskFactor", 
         po::value<float>(),
         "Risk factor to adjust number of contracts traded.")
        ("strat.tickMult",
         po::value<float>(),
         "Minimal tick multiplier")
        ("strat.tpfMult",
         po::value<float>(),
         "Taking profit multiplier")
        
        ("strat.alpha",
         po::value<float>(),
         "Alpha")
        ("strat.beta",
         po::value<float>(),
         "Beta")

        
        ;
    args.add(argsMandCmdLn);
}

/* virtual */ void Factory::init(const po::variables_map &argVals) {
    if (argVals.count("strat.nearPeriod") == 0) 
        throw po::error("Missing option value: strat.nearPeriod");
    if (argVals.count("strat.medPeriod") == 0) 
        throw po::error("Missing option value: strat.medPeriod");
    if (argVals.count("strat.farPeriod") == 0)
        throw po::error("Missing option value: strat.farPeriod");

    if (argVals.count("strat.longAaaPeriod") == 0) 
        throw po::error("Missing option value: strat.longAaaPeriod");
    if (argVals.count("strat.shortAaaPeriod") == 0) 
        throw po::error("Missing option value: strat.shortAaaPeriod");

    if (argVals.count("strat.ricciPeriod") == 0)
        throw po::error("Missing option value: strat.ricciPeriod");

    if (argVals.count("strat.allocAtrPeriod") == 0) 
        throw po::error("Missing option value: strat.allocAtrPeriod");

    if (argVals.count("strat.refAtrPeriod") == 0) 
        throw po::error("Missing option value: strat.refAtrPeriod");

    
    if (argVals.count("strat.pctPeriod") == 0)
        throw po::error("Missing option value: strat.pctPeriod");

    if (argVals.count("strat.extrDistance") == 0)
        throw po::error("Missing option value: strat.extrDistance");

    if (argVals.count("strat.maxPosAge") == 0) 
        throw po::error("Missing option value: strat.maxPosAge");

    if (argVals.count("strat.ind1LBPct") == 0) 
        throw po::error("Missing option value: strat.ind1LBPct");
    if (argVals.count("strat.ind1UBPct") == 0)
        throw po::error("Missing option value: strat.ind1UBPct");
    if (argVals.count("strat.ind2LBPct") == 0) 
        throw po::error("Missing option value: strat.ind2LBPct");
    if (argVals.count("strat.ind2UBPct") == 0)
        throw po::error("Missing option value: strat.ind2UBPct");
    if (argVals.count("strat.ind3LBPct") == 0) 
        throw po::error("Missing option value: strat.ind3LBPct");
    if (argVals.count("strat.ind3UBPct") == 0)
        throw po::error("Missing option value: strat.ind3UBPct");


    if (argVals.count("strat.numOfUnits") == 0)
        throw po::error("Missing option value: strat.numOfUnits");
    if (argVals.count("strat.riskFactor") == 0)
        throw po::error("Missing option value: strat.riskFactor");
    if (argVals.count("strat.tickMult") == 0)
        throw po::error("Missing option value: strat.tickMult");
    if (argVals.count("strat.tpfMult") == 0)
        throw po::error("Missing option value: strat.tpfMult");
     
    if (argVals.count("strat.alpha") == 0)
        throw po::error("Missing option value: strat.alpha");
     if (argVals.count("strat.beta") == 0)
        throw po::error("Missing option value: strat.beta");
    

    m_nearPeriod = argVals["strat.nearPeriod"].as<pirate::ind::period_type>();
    m_medPeriod = argVals["strat.medPeriod"].as<pirate::ind::period_type>();
    m_farPeriod = argVals["strat.farPeriod"].as<pirate::ind::period_type>();
    m_longAaaPeriod = argVals["strat.longAaaPeriod"].as<pirate::ind::period_type>();
    m_shortAaaPeriod = argVals["strat.shortAaaPeriod"].as<pirate::ind::period_type>();
    m_ricciPeriod = argVals["strat.ricciPeriod"].as<pirate::ind::period_type>();
    m_allocAtrPeriod = argVals["strat.allocAtrPeriod"].as<pirate::ind::period_type>();
    m_refAtrPeriod = argVals["strat.refAtrPeriod"].as<pirate::ind::period_type>();
    m_pctPeriod = argVals["strat.pctPeriod"].as<pirate::ind::period_type>();
    m_extrDistance = argVals["strat.extrDistance"].as<pirate::ind::period_type>();

    m_maxPosAge = argVals["strat.maxPosAge"].as<int>();

    m_ind1LBPct = argVals["strat.ind1LBPct"].as<double>();
    m_ind1UBPct = argVals["strat.ind1UBPct"].as<double>();
    m_ind2LBPct = argVals["strat.ind2LBPct"].as<double>();
    m_ind2UBPct = argVals["strat.ind2UBPct"].as<double>();
    m_ind3LBPct = argVals["strat.ind3LBPct"].as<double>();
    m_ind3UBPct = argVals["strat.ind3UBPct"].as<double>();

    m_numOfUnits = argVals["strat.numOfUnits"].as<int>();
    m_riskFactor = argVals["strat.riskFactor"].as<float>();
    m_tickMult = argVals["strat.tickMult"].as<float>();
    m_tpfMult = argVals["strat.tpfMult"].as<float>();
    m_alpha = argVals["strat.alpha"].as<float>();
    m_beta = argVals["strat.beta"].as<float>();
}

/* virtual */ Strat* Factory::create(const pirate::tr::Trader &trader) {

    /*
    const MktDef &mktDef = trader.mktDef();
    if (mktDef.argsSize() < 2) {
        ostringstream err;
        err << "Invalid number of market specific arguments in portfolio, mktID: " << trader.mktID();
        throw runtime_error(err.str());
    }
    const float arg0 = lexical_cast<float>(mktDef[0]);
    const string arg1 = lexical_cast<string>(mktDef[1]);
    cout << "Mkt: " << trader.mktID() << ", arg0: " << arg0 << ", arg1: " << arg1 << endl;
    */

    return new Strat(m_nearPeriod,
                     m_medPeriod,
                     m_farPeriod, 
                     m_longAaaPeriod,
                     m_shortAaaPeriod,
                     m_ricciPeriod,
                     m_allocAtrPeriod,
                     m_refAtrPeriod,
                     m_pctPeriod,
                     m_extrDistance,

                     m_maxPosAge,

                     m_ind1LBPct,
                     m_ind1UBPct,
                     m_ind2LBPct,
                     m_ind2UBPct,
                     m_ind3LBPct,
                     m_ind3UBPct,
                     m_numOfUnits,
                     m_riskFactor,
                     m_tickMult, 
                     m_tpfMult,
                     m_alpha,
                     m_beta
        );
}

/* virtual */ void Factory::version(pirate::ver::Versions *versionsPtr) {
    pirate::ind::version(versionsPtr);
    versionsPtr->append(stratName, stratVersion, __DATE__, __TIME__);
}
