/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2007, 2008 StatPro Italia srl

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

/*! \file mceuropeanengine.hpp
    \brief Monte Carlo European option engine
*/

#ifndef montecarlo_european_engine_hpp
#define montecarlo_european_engine_hpp

#include <ql/pricingengines/vanilla/mcvanillaengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include "constantblackscholesprocess.hpp"

namespace QuantLib {

    //! Path pricer pour l'option européenne
    class EuropeanPathPricer_2 : public PathPricer<Path> {
      public:
        EuropeanPathPricer_2(Option::Type type, Real strike, DiscountFactor discount)
        : payoff_(type, strike), discount_(discount) {}

        Real operator()(const Path& path) const {
            QL_REQUIRE(path.length() > 0, "Le chemin ne peut pas être vide");
            return payoff_(path.back()) * discount_;
        }
      private:
        PlainVanillaPayoff payoff_;
        DiscountFactor discount_;
    };

    //! Moteur Monte Carlo pour option européenne
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCEuropeanEngine_2 : public MCVanillaEngine<SingleVariate,RNG,S> {
      public:
        typedef typename MCVanillaEngine<SingleVariate,RNG,S>::path_generator_type path_generator_type;
        typedef typename MCVanillaEngine<SingleVariate,RNG,S>::path_pricer_type   path_pricer_type;
        typedef typename MCVanillaEngine<SingleVariate,RNG,S>::stats_type          stats_type;

        MCEuropeanEngine_2(
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
            bool useConstantProcess,
            Size timeSteps,
            Size timeStepsPerYear,
            bool brownianBridge,
            bool antitheticVariate,
            Size requiredSamples,
            Real requiredTolerance,
            Size maxSamples,
            BigNatural seed);

      protected:
        bool useConstantProcess_;
        boost::shared_ptr<path_pricer_type> pathPricer() const override;
    };

    template <class RNG, class S>
    inline MCEuropeanEngine_2<RNG,S>::MCEuropeanEngine_2(
        const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
        bool useConstantProcess,
        Size timeSteps,
        Size timeStepsPerYear,
        bool brownianBridge,
        bool antitheticVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed)
    : MCVanillaEngine<SingleVariate,RNG,S>(process,
                                           timeSteps,
                                           timeStepsPerYear,
                                           brownianBridge,
                                           antitheticVariate,
                                           false,
                                           requiredSamples,
                                           requiredTolerance,
                                           maxSamples,
                                           seed),
      useConstantProcess_(useConstantProcess) {}

    template <class RNG, class S>
    inline boost::shared_ptr<typename MCEuropeanEngine_2<RNG,S>::path_pricer_type>
    MCEuropeanEngine_2<RNG,S>::pathPricer() const {
        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(this->arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(this->process_);
        QL_REQUIRE(process, "Black-Scholes process required");

        DiscountFactor discount = process->riskFreeRate()->discount(this->timeGrid().back());
        return boost::make_shared<EuropeanPathPricer_2>(
                payoff->optionType(),
                payoff->strike(),
                discount);
    }

    //! Factory pour le moteur Monte Carlo européen
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCEuropeanEngine_2 {
      public:
        MakeMCEuropeanEngine_2(const boost::shared_ptr<GeneralizedBlackScholesProcess>& process);
        MakeMCEuropeanEngine_2& withConstantParameters(bool b = true);
        MakeMCEuropeanEngine_2& withSteps(Size steps);
        MakeMCEuropeanEngine_2& withSamples(Size samples);
        MakeMCEuropeanEngine_2& withSeed(BigNatural seed);
        operator boost::shared_ptr<PricingEngine>() const;
      private:
        boost::shared_ptr<GeneralizedBlackScholesProcess> process_;
        bool useConstantProcess_;
        Size steps_;
        Size samples_;
        BigNatural seed_;
    };

    template <class RNG, class S>
    inline MakeMCEuropeanEngine_2<RNG,S>::MakeMCEuropeanEngine_2(
        const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
    : process_(process), useConstantProcess_(true), steps_(0), samples_(0), seed_(0) {}

    template <class RNG, class S>
    inline MakeMCEuropeanEngine_2<RNG,S>&
    MakeMCEuropeanEngine_2<RNG,S>::withConstantParameters(bool b) {
        useConstantProcess_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanEngine_2<RNG,S>&
    MakeMCEuropeanEngine_2<RNG,S>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanEngine_2<RNG,S>&
    MakeMCEuropeanEngine_2<RNG,S>::withSamples(Size samples) {
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCEuropeanEngine_2<RNG,S>&
    MakeMCEuropeanEngine_2<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCEuropeanEngine_2<RNG,S>::operator boost::shared_ptr<PricingEngine>() const {
        // Si on utilise les paramètres constants, on crée une instance unique du ConstantBlackScholesProcess.
        boost::shared_ptr<GeneralizedBlackScholesProcess> engineProcess;
        if (useConstantProcess_) {
            engineProcess = boost::make_shared<ConstantBlackScholesProcess>(
                                process_->stateVariable(),
                                process_->riskFreeRate(),
                                process_->dividendYield(),
                                process_->blackVolatility());
        } else {
            engineProcess = process_;
        }
        return boost::make_shared<MCEuropeanEngine_2<RNG,S>>(
            engineProcess,
            useConstantProcess_,
            steps_,
            Null<Size>(),
            false,
            false,
            samples_,
            Null<Real>(),
            Null<Size>(),
            seed_);
    }

} 

#endif

