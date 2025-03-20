

#ifndef quantlib_constant_black_scholes_process_hpp
#define quantlib_constant_black_scholes_process_hpp

#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! 
    /*!  Tous les paramètres sont constants.
        Les valeurs sont extraites une seule fois lors de la construction.
    */
    class ConstantBlackScholesProcess : public GeneralizedBlackScholesProcess {
      public:
        // Constructeur identique à l'original 
        ConstantBlackScholesProcess(Handle<Quote> x0, 
                                    Handle<YieldTermStructure> riskFreeRate,
                                    Handle<YieldTermStructure> dividendYield, 
                                    Handle<BlackVolTermStructure> blackVolatility);

        //! Implémentation des méthodes de GeneralizedBlackScholesProcess
        Real x0() const override;
        Real drift(Time t, Real x) const override;
        Real diffusion(Time t, Real x) const override;
        Real expectation(Time t0, Real x0, Time dt) const override;
        Real stdDeviation(Time t0, Real x0, Time dt) const override;
        Real variance(Time t0, Real x0, Time dt) const override;
        Real evolve(Time t0, Real x0, Time dt, Real dw) const override;
      private:
        // Valeurs constantes pré-calculées
        Real constantX0_;
        Real constantRiskFree_;
        Real constantDividendYield_;
        Real constantVolatility_;
    };

} // namespace QuantLib

#endif 


