

#include "constantblackscholesprocess.hpp"
#include <ql/settings.hpp>
#include <cmath>

namespace QuantLib {

    ConstantBlackScholesProcess::ConstantBlackScholesProcess(
        Handle<Quote> x0, 
        Handle<YieldTermStructure> riskFreeRate,
        Handle<YieldTermStructure> dividendYield, 
        Handle<BlackVolTermStructure> blackVolatility)
    : GeneralizedBlackScholesProcess(x0, dividendYield, riskFreeRate, blackVolatility),
      constantX0_(x0->value()),
      // Utilisation de Settings::instance().evaluationDate(), du day counter et des options d'extrapolation.
      constantRiskFree_(riskFreeRate->zeroRate(Settings::instance().evaluationDate(),
                                                  riskFreeRate->dayCounter(),
                                                  Continuous,
                                                  NoFrequency,
                                                  true).rate()),
      constantDividendYield_(dividendYield->zeroRate(Settings::instance().evaluationDate(),
                                                      dividendYield->dayCounter(),
                                                      Continuous,
                                                      NoFrequency,
                                                      true).rate()),
      constantVolatility_(blackVolatility->blackVol(Settings::instance().evaluationDate(),
                                                     x0->value(),
                                                     true))
    {}

    Real ConstantBlackScholesProcess::x0() const {
        return constantX0_;
    }

    Real ConstantBlackScholesProcess::drift(Time t, Real /*x*/) const {
        return constantRiskFree_ - constantDividendYield_ - 0.5 * constantVolatility_ * constantVolatility_;
    }

    Real ConstantBlackScholesProcess::diffusion(Time /*t*/, Real /*x*/) const {
        return constantVolatility_;
    }

    Real ConstantBlackScholesProcess::expectation(Time t0, Real x0, Time dt) const {
        return x0 * std::exp(drift(t0, x0) * dt);
    }

    Real ConstantBlackScholesProcess::stdDeviation(Time /*t0*/, Real /*x0*/, Time dt) const {
        return constantVolatility_ * std::sqrt(dt);
    }

    Real ConstantBlackScholesProcess::variance(Time /*t0*/, Real /*x0*/, Time dt) const {
        return constantVolatility_ * constantVolatility_ * dt;
    }

    Real ConstantBlackScholesProcess::evolve(Time t0, Real x0, Time dt, Real dw) const {
        return x0 * std::exp(drift(t0, x0) * dt + constantVolatility_ * std::sqrt(dt) * dw);
    }

} 
