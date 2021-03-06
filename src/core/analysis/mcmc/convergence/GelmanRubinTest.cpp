#include "GelmanRubinTest.h"
#include "DistributionNormal.h"

#include <cmath>

using namespace RevBayesCore;
using namespace std;

GelmanRubinTest::GelmanRubinTest(double r, size_t n) : ConvergenceDiagnosticContinuous(),
    R( r ),
    nBatches( n )
{
    
}

bool GelmanRubinTest::assessConvergenceSingleChain(const std::vector<double>& values, size_t burnin)
{
    
    double  withinBatchVariance     = 0;
    double  betweenBatchVariance    = 0;
    
    size_t  totalSampleSize = values.size() - burnin;
    size_t  batchSize       = size_t(totalSampleSize / (double)nBatches);
    
    // get the mean between all traces
    analysis.analyseMean(values,burnin);
    double totalMean = analysis.getMean();
    
    // get a mean and standard error for each block
    std::vector<double> batchMeans  = std::vector<double>(nBatches,0.0);
    for (size_t i=0; i<nBatches; i++)
    {
        analysis.analyseMean(values,i*batchSize+burnin,(i+1)*batchSize+burnin);
        batchMeans[i] = analysis.getMean();
        
        // iterate over all samples from the chains
        for (size_t j=i*batchSize+burnin; j<(i+1)*batchSize+burnin; j++)
        {
            withinBatchVariance     += ( (values.at(j) - batchMeans[i])*(values.at(j) - batchMeans[i]) );
            betweenBatchVariance    += ( (values.at(j) - totalMean)*(values.at(i) - totalMean) );
        }
    }
    
    double psrf = ((totalSampleSize-nBatches) / (totalSampleSize-1.0)) * (betweenBatchVariance/withinBatchVariance);
    
    return psrf < R;
}

bool GelmanRubinTest::assessConvergenceMultipleChains(const std::vector<std::vector<double> >& values, const std::vector<size_t>& burnins)
{
    
    double  withinChainVariance     = 0;
    double  betweenChainVariance    = 0;
    size_t  totalSampleSize         = 0;
    
    // get number of chains
    size_t nChains = values.size();
    
    // get the mean between all traces
    analysis.analyseMean(values,burnins);
    double totalMean = analysis.getMean();
    
    // get a mean and standard error for each block
    std::vector<double> chainMeans  = std::vector< double >(nChains,0.0);
    for (size_t i=0; i<nChains; i++)
    {
        const std::vector<double>& chain    = values.at(i);
        size_t burnin                       = burnins.at(i);
        analysis.analyseMean(chain,burnin);
        chainMeans[i]                       = analysis.getMean();
    }
    
    // iterate over all chains
    for (size_t i=0; i<nChains; i++)
    {
        const std::vector<double>& chain    = values.at(i);
        size_t burnin                       = burnins.at(i);
        
        size_t chainSize                    = chain.size() - burnin;
        // add this chain size to the total sample size
        totalSampleSize                     += chainSize;
        // iterate over all samples from the chains
        for (size_t j=burnin; j<chainSize; j++)
        {
            withinChainVariance     += ( (chain.at(j) - chainMeans[i])*(chain.at(j) - chainMeans[i]) );
            betweenChainVariance    += ( (chain.at(j) - totalMean)*(chain.at(j) - totalMean) );
        }
    }
    
    double psrf = ((totalSampleSize-nChains) / (totalSampleSize-1.0)) * (betweenChainVariance/withinChainVariance);
    
    return psrf < R;
}
