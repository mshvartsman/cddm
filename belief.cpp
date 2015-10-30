#include <vector>
#include <armadillo>
#include <cmath>
#include "config.h" 
#include "rng.h" // for noisifying sample pre-update
#include "fatal_error.h"
#include "utils.h"
#include "belief.h"

using std::vector;
using arma::mat;

/**
 * @brief Constructor for Belief.
 * @details Constructor for Belief. Expects a Config with set 
 * \ref urPrior and optionally \ref contextMeanSpacing and 
 * \ref targetMeanSpacing (default 1 on both). Properly sizes the 
 * posterior and likelihood, and resets both. 
 */
Belief::Belief(const Config * c):_trueContext(-1),_trueTarget(-1){
    _urPrior = c->get<mat>("urPrior"); 
    #ifndef DISABLE_ERROR_CHECKS
        if (utils::kahanSum(_urPrior) != 1) throw fatal_error() << "urPrior is not proper! Actual sum: " << utils::kahanSum(_urPrior) << ", actual urPrior " << _urPrior; 
    #endif
    // see if we have nonstandard spacing for context,target means
    if (c->keyExists("contextMeanSpacing")){
        _contextMeanSpacing = c->get<double>("contextMeanSpacing"); 
    } else {
        _contextMeanSpacing = 1; 
    }
    if (c->keyExists("targetMeanSpacing")){
        _targetMeanSpacing = c->get<double>("targetMeanSpacing"); 
    } else {
        _targetMeanSpacing = 1; 
    }
    _nContexts = _urPrior.n_rows;
    _nTargets = _urPrior.n_cols;
    _belief.set_size(_nContexts, _nTargets); 
    _lik.set_size(_nContexts, _nTargets); 
    _lik.fill(-1);
    reset(); 
}

/**
 * @brief Constructor for DecayBelief. 
 * @details Constructor for Belief with a decaying sampling distribution. 
 * Expects a Config with set \ref urPrior and optionally 
 * \ref contextMeanSpacing and \ref targetMeanSpacing (default 1 on both) as
 * does its parent Belief. Also optinally accepts \ref decayRate for the 
 * rate of change of the sampling distribution (default 0 is equivalent to
 * the parent Belief)
 */
DecayBelief::DecayBelief(const Config * c): Belief(c){
    if (c->keyExists("decayRate")){
        _decayRate = c->get<double>("decayRate"); 
    } else {
        _decayRate = 0; 
    }
    _contextMarginals = sum(_urPrior, 1); 
}

/**
 * @brief Set the context and target being sampled from. 
 * @details Stores the current context and target being sampled from. 
 * 
 * @param trueContext the index of the context to sample from. Allowable 
 * values from 0 to \ref nContexts%-1
 * @param trueTarget the index of the target to sample from. Allowable 
 * values from 0 to \ref nTargets%-1
 */
void Belief::setTrueStim(int trueContext, int trueTarget){
    #ifndef DISABLE_ERROR_CHECKS
    if(trueContext >= _nContexts) throw fatal_error() << "ERROR: setting context to " << trueContext << " but only have " << _nContexts << " in representation (zero-indexed)!";
    if(trueContext<0) throw fatal_error() << "ERROR: context < 0?! Provided value of " << trueContext;
    if(trueTarget >= _nTargets) throw fatal_error() << "ERROR: setting target to " << trueTarget << " but only have " << _nTargets << " in representation (zero-indexed)!";
    if(trueTarget<0) throw fatal_error() << "ERROR: target < 0?! Provided value of " << trueTarget;
    #endif
    _trueContext = trueContext; 
    _trueTarget = trueTarget; 
}

/**
 * @brief Return the current belief posterior. 
 */
mat Belief::getBelief(){
    return _belief; 
}

/**
 * @brief Reset the current belief posterior to the trial-start prior. 
 */
void Belief::reset(){
    _belief = _urPrior; 
}


/**
 * @brief Update the posterior from context. 
 * @details Perform the update \f$ P_{\tau}(C,G\mid e^C) = \eta P(e^C \mid C,G)P_{\tau-1}(C,G) \f$  
 * 
 * @param noise standard deviation of the gaussian sampling distribution of the context evidence. 
 */
void Belief::updateFromContext(double noise){
    update(Context, noise);
}

/**
 * @brief Update the posterior from target. 
 * @details Perform the update \f$ P_{\tau}(C,G\mid e^T) = \eta P(e^T \mid C,G)P_{\tau-1}(C,G) \f$  
 * 
 * @param noise standard deviation of the gaussian sampling distribution of the target evidence. 
 */
void Belief::updateFromTarget(double noise){
    update(Target, noise);
}

/**
 * @brief Perform a single belief update. 
 * @details Performs a single update \f$ P_{\tau}(C,G\mid e^T) = \eta P(e^T \mid C,G)P_{\tau-1}(C,G) \f$ 
 * or \f$ P_{\tau}(C,G\mid e^C) = \eta P(e^C \mid C,G)P_{\tau-1}(C,G) \f$, including drawing the random
 * sample, computing and multiplying the likelihoods by the priors, and normalizing. Usually
 * the individual update functions updateFromContext updateFromTarget should be used. 
 * This method is mostly internal, and lets us avoid some duplicated code in sampling and normalization. 
 * * @param source where to update from -- Context or Target
 * @param noise the SD of the sampling distribution of the evidence
 * 
 * \todo consider making Belief::update() private
 
 */
void Belief::update(UpdateSource source, double noise){
    // set up
    double truth, normalizer, samp; 
    switch (source){
        case Context:
        truth = _trueContext; 
        break;
        case Target: 
        truth = _trueTarget; 
        break; 
        #ifndef DISABLE_ERROR_CHECKS
        default:
        fatal_error() << "ERROR: unknown update source"; 
        #endif
    }
    samp = RNG::rnorm(truth, noise); 
    _computeLikelihoods(samp, noise, source);
    _belief %= _lik; // armadillo elementwise multiply
    normalizer = utils::kahanSum(_belief); 
    _belief = _belief / normalizer; 
    // #ifndef DISABLE_ERROR_CHECKS
    // if (any(vectorise(_belief)==0)) throw fatal_error() << "BELIEF IS 0";
    // #endif
}


/**
 * @brief Compute likelihoods under decaying sampling distribution of the context. 
 * @details This assumes that the probability that the incoming sample came from the 
 * true context is pCorrectUpdate, and therefore the likelihood is 
 * \f$e^{-\beta\tau} P(e^C \mid C=c_i) + (1-e^{-\beta\tau}) \sum_j P(e^C \mid C=c_j)P_0(C=c_j)\f$. 
 * That is, the true likelihood plus all the likelihoods weighed by the prior distribution (assumed 
 * to be the probability of drawing a random sample). 
 * @param samp a draw from the evidence distribution
 * @param noise SD of the evidence distribution
 * @param source source of the sample (Context or Target)
 * @param pCorrectUpdate the probability of the correct update, \f$ e^{-\beta\tau} \f$. 
 */
void DecayBelief::_computeLikelihoods(double samp, double noise, UpdateSource source, double pCorrectUpdate){
    switch (source){
        case Context: {
            for (unsigned i=0; i<_nContexts; ++i){
                for (unsigned j=0; j<_nTargets; ++j){
                    // likelihood is (dnorm | correct) * pCorrect + (1-pCorrect) * (tDist[0]*(dnorm|0) + tdist[1]*(dnorm|1))
                    double correctLik = RNG::dnorm(samp, i*_contextMeanSpacing, noise); // true portion
                    double noisyLik = 0; 
                    for (unsigned k=0; k<_nContexts; ++k){ // noisy portion
                        noisyLik += _contextMarginals(k) * RNG::dnorm(samp, k*_contextMeanSpacing, noise); 
                    }
                    _lik(i,j) = pCorrectUpdate * correctLik + (1-pCorrectUpdate)* noisyLik;                     
                }
            }
            break;
        }
        case Target: {
            for (unsigned i=0; i<_nContexts; ++i){
                for (unsigned j=0; j<_nTargets; ++j){
                    _lik(i,j) = RNG::dnorm(samp, j*_targetMeanSpacing, noise);
                }
            }
            break; 
        }
        #ifndef DISABLE_ERROR_CHECKS
        default:
        fatal_error() << "ERROR: unknown update source"; 
        #endif
    }    
}
// Specifically, the sampling distribution is: 
/**
 * @brief Update from a decaying sampling distribution of the context. 
 * @details Perform an update from a decay context. The context decays 
 * exponentially such that at time \f$\tau\f$ the probability of drawing
 * the true context is \f$ e^{-\beta\tau}\f$, otherwise drawing either 
 * uniformly, or according to the trial-level prior ("urPrior"). 
 
 * @param noise SD of the sampling distribution of the evidence
 * @param trialTime current trial time (needed to compute the decay)
 * @param decayTo How to randomly draw a decayed sample? 
 * @return -1 for a correct sample; otherwise the index of the context sampled from. 
 */
int DecayBelief::updateFromContext(double noise, double trialTime, PriorType decayTo){
    if (_decayRate == 0 || trialTime == 0) { // don't waste computation if this is a no-decay model
        Belief::updateFromContext(noise);  // run the base class no-decay update
        return -1; // return -1 to signify we didn't decay here, for testing
    } else {
        double pCorrectUpdate = exp(-_decayRate*(trialTime)); ///< \todo can precompute pCorrectUpdate once for the whole sim as long as we set/can know maxTrialTime somewhere
        double truth, normalizer, samp; 
        arma::vec cumContextProb = arma::cumsum(arma::sum(_urPrior,1)); // sum rows, then cumsum so we can do categorical draw on it    
        int goodRetrieval = RNG::rbernoulli(pCorrectUpdate); 
        if (goodRetrieval == 0){ // if we did a bad retrieval 
            truth = -1; 
            if (decayTo == Uniform){
                truth = RNG::runif_int(_nContexts-1); // open interval
            } else if (decayTo == Informative){
                // do a categorical draw on marginal context prob
                // horribly non-idiomatic? We just count up until we cross cumulative prob threshold
                double p = RNG::runif(1); 
                for (truth=0; p >= cumContextProb[truth]; ++truth)
                    ; 
            }
            #ifndef DISABLE_ERROR_CHECKS
            else throw fatal_error() << "Unknown PriorType!";
            if (truth >= _nContexts || truth < 0) throw fatal_error() << "failed to draw any contexts? Do you have a proper distribution?";
            #endif    
            truth = truth; 
        } else { // we did not do a bad retrieval
            truth = _trueContext; 
        }
        // set up
        samp = RNG::rnorm(truth, noise); 
        _computeLikelihoods(samp, noise, Context, pCorrectUpdate);
        _belief %= _lik; // elementwise multiply
        normalizer = utils::kahanSum(_belief); 
        _belief = _belief / normalizer; 
        // #ifndef DISABLE_ERROR_CHECKS
        // if (any(vectorise(_belief)==0)) throw fatal_error() << "BELIEF IS 0";
        // #endif
        return goodRetrieval==1 ? -1: truth; // signal whether we retrieved correctly, otherwise what we retrieved
    }
}

/**
 * @brief Belief update with a context that can be forgotten. 
 * @details Perform an update with some probability (_forgetProb) of forgetting the 
 * true context at each update, and sampling from a random context after that point. 
 * 
 * @param noise SD of the sampling distribution of the evidence. 
 * @param forgetTo What to draw upon forgetting. Uniform means a uniformly random context; 
 * Prior means one drawn according to the trial-level prior. 
 * 
 * @return -1 if haven't forgotten, otherwise the index of the sampled contect. 
 */
int ForgetBelief::contextForgetUpdate(double noise, PriorType forgetTo) {
    if (_forgetProb == 0 || _forgot) { // don't waste computation if we forgot already or can't forget
        updateFromContext(noise);
        return -1; // signal that we drew the correct one
    }
    // check if we forgot
    if (RNG::rbernoulli(_forgetProb) == 1) { // forgot now
        _forgot = true; 
        int drawFrom = -1; 
        arma::vec cumContextProb = arma::cumsum(arma::sum(_urPrior,1)); // sum rows, then cumsum so we can do categorical draw on it
        if (forgetTo == Uniform){
            drawFrom = RNG::runif_int(_nContexts-1); // open interval
        } else if (forgetTo == Informative){
            // do a categorical draw on marginal context prob
            // horribly non-idiomatic? We just count up until we cross cumulative prob threshold
            double p = RNG::runif(1); 
            for (drawFrom=0; p >= cumContextProb[drawFrom]; ++drawFrom); 
        }
        #ifndef DISABLE_ERROR_CHECKS
    else throw fatal_error() << "Unknown forgetTo!";
    if (drawFrom >= _nContexts || drawFrom < 0) throw fatal_error() << "failed to draw any contexts? Do you have a proper distribution?";
        #endif    
    _trueContext = drawFrom; 
    updateFromContext(noise);
    return drawFrom; 
    } else { // did not forget
        updateFromContext(noise);
        return -1; // signal that we drew the correct one
    }
}

/**
 * @brief Reset the belief to the trial-level "urPrior" and unset the forgotten flag
 */
void ForgetBelief::reset(){
    _forgot = false; 
    Belief::reset(); 
}

/**
 * @brief Compute the likelihoods of the joint probabilities of all contexts
 * and targets from an incoming sample.
 * @param samp random sample from the gaussian evidence distribution. 
 * @param noise SD of the gaussian evidence distribution
 * @param source source of the sample (Context or Target)
 */
void Belief::_computeLikelihoods(double samp, double noise, UpdateSource source){
    for (unsigned i=0; i<_nContexts; ++i){
        for (unsigned j=0; j<_nTargets; ++j){
            if (source == Context){
                _lik(i,j) = RNG::dnorm(samp, i*_contextMeanSpacing, noise);
            } else if (source == Target){
                _lik(i,j) = RNG::dnorm(samp, j*_targetMeanSpacing, noise);
            }
        }
    }
} 

/**
 * @brief Return the current likelihood (mostly for testing). 
 * \todo consider making getLik() private or protected. 
 */
arma::mat Belief::getLik(){
    return _lik; // for testing
}