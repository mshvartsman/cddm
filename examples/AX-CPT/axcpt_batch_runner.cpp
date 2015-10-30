#include "axcpt.h"

#define BATCH_MODE

void populateDefaults(Config * c){
    vector<string> defaultKeyNames = {"timePerStep","retentionIntervalDur","maxTrials","maxSamps","contextNoise","targetNoise","decisionThresh","eblMean","motorPlanMean","motorExecMean","eblSd","motorSd","urPrior","trialDist","nContexts","nTargets","decayRate","pPrematureResp"}; 
    vector<string> defaultKeyVals = {"10", "200", "100", "1000", "3", "3", "0.95", "50", "150", "150", "20", "50", "0.4 0.3; 0.2 0.1", "0.4 0.3; 0.2 0.1", "2", "2","0.01","0"};
    for (unsigned i=0; i<defaultKeyNames.size(); ++i){
        if (!c->keyExists(defaultKeyNames[i])){
            c->set(defaultKeyNames[i], defaultKeyVals[i]); 
        }
    }
}



int main(int argc, const char * argv[]) {
    arma::arma_rng::set_seed_random();
    Config c; 
    Recorder r;
    IncrementalMeanVarianceDatum<double> d; 
    std::string in;
    populateDefaults(&c); 
    AxcptTask t(&c, &r); 
    vector<string> summaryDatumNames = t.getSummaryDatumNames(); 
    std::cout << "context,target,variable,mean,variance,n" << std::endl; 
    while(std::cin){
        getline(std::cin, in);
        if (in.empty()){
            std::cout << "Found empty input line, exiting!" << std::endl;
            return 0;
        }
        else {
            c.loadFromString(in); 
            populateDefaults(&c); // this has to happen AFTER config is loaded from string -- it writes defaults where there is nothing written
            AxcptTask t(&c, &r); 
            BatchExperiment be(&c, &t, &r); 
            be.run(); 
            arma::mat tdist; 
            tdist = c.get<arma::mat>("urPrior"); 
            for (unsigned i=0; i<summaryDatumNames.size(); ++i){
                for (unsigned c = 0; c < 2; c++){
                    for (unsigned t = 0; t< 2; t++){
                        d = r.getDatum<IncrementalMeanVarianceDatum<double> >("Context" + to_string(c) + "_Target" + to_string(t) + "_" + summaryDatumNames[i]); 
                        std::cout << c<< ","<<t  << ","<< summaryDatumNames[i] << "," << d.getMean() << "," << d.getVariance() << "," << d.getN() << std::endl; 
                    }
                }
            }
            r.reset(); 
        }
    }
}