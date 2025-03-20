/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file mc_discr_arith_av_strike.hpp
    \brief Monte Carlo engine for discrete arithmetic average-strike Asian
*/

#ifndef mc_discrete_arithmetic_average_strike_asian_engine_hpp
#define mc_discrete_arithmetic_average_strike_asian_engine_hpp

#include <ql/exercise.hpp>
#include <ql/pricingengines/asian/mcdiscreteasianenginebase.hpp>
#include <ql/pricingengines/asian/mc_discr_arith_av_strike.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include "constantblackscholesprocess.hpp"
#include <utility>

namespace QuantLib {

    //! Monte Carlo pricing engine for discrete arithmetic average-strike Asian options
    /*! \ingroup asianengines */
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCDiscreteArithmeticASEngine_2
        : public MCDiscreteAveragingAsianEngineBase<SingleVariate,RNG,S> {
      public:
        typedef typename MCDiscreteAveragingAsianEngineBase<SingleVariate,RNG,S>::path_generator_type path_generator_type;
        typedef typename MCDiscreteAveragingAsianEngineBase<SingleVariate,RNG,S>::path_pricer_type   path_pricer_type;
        typedef typename MCDiscreteAveragingAsianEngineBase<SingleVariate,RNG,S>::stats_type          stats_type;
        // Constructor
        MCDiscreteArithmeticASEngine_2(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             bool brownianBridge,
             bool antitheticVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed)
        : MCDiscreteAveragingAsianEngineBase<SingleVariate,RNG,S>(process,
                                                              brownianBridge,
                                                              antitheticVariate,
                                                              false,
                                                              requiredSamples,
                                                              requiredTolerance,
                                                              maxSamples,
                                                              seed) {}
      protected:
        ext::shared_ptr<path_pricer_type> pathPricer() const override {
            ext::shared_ptr<PlainVanillaPayoff> payoff =
                ext::dynamic_pointer_cast<PlainVanillaPayoff>(this->arguments_.payoff);
            QL_REQUIRE(payoff, "non-plain payoff given");

            ext::shared_ptr<EuropeanExercise> exercise =
                ext::dynamic_pointer_cast<EuropeanExercise>(this->arguments_.exercise);
            QL_REQUIRE(exercise, "wrong exercise given");

            ext::shared_ptr<GeneralizedBlackScholesProcess> process =
                ext::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(this->process_);
            QL_REQUIRE(process, "Black-Scholes process required");

            // On utilise le process fourni pour obtenir le discount à la date d'exercice
            return ext::shared_ptr<path_pricer_type>(
                new ArithmeticASOPathPricer(
                    payoff->optionType(),
                    process->riskFreeRate()->discount(exercise->lastDate()),
                    this->arguments_.runningAccumulator,
                    this->arguments_.pastFixings));
        }
    };

    //! Factory for MCDiscreteArithmeticASEngine_2
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCDiscreteArithmeticASEngine_2 {
      public:
        explicit MakeMCDiscreteArithmeticASEngine_2(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process)
        : process_(std::move(process)),
          samples_(Null<Size>()), maxSamples_(Null<Size>()),
          tolerance_(Null<Real>()), brownianBridge_(true),
          seed_(0), useConstantProcess_(true) {}
        // Named parameters
        MakeMCDiscreteArithmeticASEngine_2& withBrownianBridge(bool b = true) {
            brownianBridge_ = b; return *this;
        }
        MakeMCDiscreteArithmeticASEngine_2& withSamples(Size samples) {
            QL_REQUIRE(tolerance_ == Null<Real>(), "tolerance already set");
            samples_ = samples; return *this;
        }
        MakeMCDiscreteArithmeticASEngine_2& withAbsoluteTolerance(Real tolerance) {
            QL_REQUIRE(samples_ == Null<Size>(), "number of samples already set");
            QL_REQUIRE(RNG::allowsErrorEstimate,
                       "chosen random generator policy does not allow an error estimate");
            tolerance_ = tolerance; return *this;
        }
        MakeMCDiscreteArithmeticASEngine_2& withMaxSamples(Size samples) {
            maxSamples_ = samples; return *this;
        }
        MakeMCDiscreteArithmeticASEngine_2& withSeed(BigNatural seed) {
            seed_ = seed; return *this;
        }
        MakeMCDiscreteArithmeticASEngine_2& withAntitheticVariate(bool b = true) {
            antithetic_ = b; return *this;
        }
        MakeMCDiscreteArithmeticASEngine_2& withConstantParameters(bool b = true) {
            useConstantProcess_ = b; return *this;
        }
        // Conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const {
            ext::shared_ptr<GeneralizedBlackScholesProcess> engineProcess;
            if (useConstantProcess_) {
                engineProcess = ext::make_shared<ConstantBlackScholesProcess>(
                                   process_->stateVariable(),
                                   process_->riskFreeRate(),
                                   process_->dividendYield(),
                                   process_->blackVolatility());
            } else {
                engineProcess = process_;
            }
            return ext::shared_ptr<PricingEngine>(
                new MCDiscreteArithmeticASEngine_2<RNG,S>(
                    engineProcess,
                    brownianBridge_,
                    antithetic_,
                    samples_, tolerance_,
                    maxSamples_,
                    seed_));
        }
      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        bool antithetic_ = false;
        Size samples_, maxSamples_;
        Real tolerance_;
        bool brownianBridge_ = true;
        BigNatural seed_ = 0;
        bool useConstantProcess_;
    };

} // namespace QuantLib

#endif
