// include guard
#ifndef BELIEF_H
#define BELIEF_H

#include <armadillo>
#include "config.h"

enum UpdateSource {Context, Target}; 
enum PriorType {Informative, Uniform}; 

/**
 * @brief Class implementing the core Belief update. 
 * @details Implements the update \f$ P_{\tau}(C,G\mid e^C, e^G) = \eta P(e^C, e^G\mid C,G)P_{\tau-1}(C,G) \f$
 * with normally-distributed evidence and no decay in the evidence distribution.
 * \todo make Belief a pure abstract class. 
 */
class Belief{
    public:
        virtual void updateFromTarget(double noise);
        virtual void updateFromContext(double noise);
        virtual void update(UpdateSource source, double noise); 
        virtual void setTrueStim(int trueContext, int trueTarget);
        virtual void reset(); 
        virtual arma::mat getBelief(); 
        Belief(const Config * c);
        arma::mat getLik(); // for testing
        
    protected: 
        int _trueContext; ///< true context we are sampling from
        int _trueTarget; ///< true target we are sampling from
        arma::mat _belief; ///< the current posterior
        arma::mat _urPrior; /**< the prior at the start of time (called 
                              * urPrior to disambiguate from the prior at each 
                              * timestep which is the previous posterior). */
        arma::mat _lik; ///< temporary holder for the likelihood of all hypotheses
        int _nContexts; ///< number of contexts, though 3+ not heavily tested
        int _nTargets; ///< number of targets, though 3+ not heavily tested
        double _contextMeanSpacing; ///< spacing of the context means on the number line. 
        double _targetMeanSpacing; ///< spacing of the target means on the number line. 
        void _computeLikelihoods(double samp, double noise, UpdateSource source); 
};

/**
 * @brief Class implementing the Belief update with decaying context. 
 * @details Implements the update \f$ P_{\tau}(C,G\mid e^C, e^G) = \eta P(e^C, e^G\mid C,G)P_{\tau-1}(C,G) \f$
 * with a probability of drawing the "true" context decaying at \f$e^{-\beta\tau}\f$
 */
class DecayBelief: public Belief{
    public:
        DecayBelief(const Config * c);
        // with default params, should be equivalent to parent class
        virtual int updateFromContext(double noise, double trialTime=0, PriorType decayTo=Informative);
        virtual void _computeLikelihoods(double samp, double noise, UpdateSource source, double pCorrectUpdate=1);
    protected:
        arma::vec _contextMarginals; ///< precomputed marginal probabilities of contexts, for the likelihood computation
        double _decayRate; ///< \f$\beta\f$, the decay rate. 
};

/**
 * @brief Class implementing a belief update where the context can be forgotten at any timestep. 
 * \warning NOT TESTED! This is equivalent on average to exponential decay so this is largely out of use. 
 * \todo finish and test ForgetBelief. 
 */
class ForgetBelief: public Belief{
    public:
        virtual void reset(); 
        ForgetBelief(const Config * c); ///< you know this is broken because there's no constructor
        int contextForgetUpdate(double noise, PriorType decayTo=Informative);
    protected:
        double _forgetProb; ///< probability of forgetting the context at each timepoint
        bool _forgot = false; ///< has the context been forgotten? 
};

#endif
