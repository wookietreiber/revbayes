#include "ParallelMcmcmc.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include <iostream>
#include <vector>
#include <cmath>

#if defined (USE_LIB_OPENMP)
    #include <omp.h>
#endif

using namespace RevBayesCore;

ParallelMcmcmc::ParallelMcmcmc(const Model& m, const std::vector<Move*> &moves, const std::vector<Monitor*> &mons, int nc, int np, int si, double dt, double sh) : delta(dt), numChains(nc), numProcesses(np), swapInterval(si), gen(0), startingHeat(sh)
{
    activeIndex = 0;
    
    for (size_t i = 0; i < numChains; i++)
    {
        // get chain heat
        double b = computeBeta(delta,int(i)) * startingHeat;
        
        // create chains
        bool a = (i == 0 ? true : false);
        Mcmc* oneChain = new Mcmc(m, moves, mons, a, b);
        oneChain->setChainIndex(i);
        oneChain->startMonitors();
        
        // add chain to team
        chains.push_back(oneChain);
        chainIdxByHeat.push_back(i);
    }
    //std::cout << "\n";
    
    
    // assign chains to processors
#if defined (USE_LIB_OPENMP)
    omp_set_num_threads(numProcesses);
#endif
    
    if (numChains < numProcesses)
        numProcesses = numChains;
    
    chainsPerProcess.resize(numProcesses);
    for (size_t i = 0, j = 0; i < numChains; i++, j++)
    {
        if (j >= numProcesses)
            j = 0;
        chainsPerProcess[j].push_back(i);
    }
    
    for (size_t i = 0; i < numChains; i++)
    {
        
        ;//std::cout << i << ": " << chains[i]->getChainHeat() << " ";
        ;//std::cout << chains[i]->getLnPosterior() << "\n";
    }
    
}

ParallelMcmcmc::ParallelMcmcmc(const ParallelMcmcmc &m)
{   
    // MJL: Do I need a copy ctor for Analysis objects (e.g. non-DAG)?
}

ParallelMcmcmc::~ParallelMcmcmc(void)
{
    
}

void ParallelMcmcmc::initialize(void)
{
    
}

double ParallelMcmcmc::computeBeta(double delta, int idx)
{
    // MJL: May want other distributions of beta in the future
    return pow(1.0 + delta, -idx);
}

void ParallelMcmcmc::burnin(int g, int ti)
{
    
}

ParallelMcmcmc* ParallelMcmcmc::clone(void) const
{
    // MJL: I think this is handled through Mcmc, though may want to derive ParallelMCMCMC from Cloneable
    return NULL;
}

void ParallelMcmcmc::printOperatorSummary(void) const
{
    for (size_t i = 0; i < numChains; i++)
    {
        //std::cout << "\nChain " << i << ":\n";
        chains[i]->printOperatorSummary();
    }
}

void ParallelMcmcmc::run(size_t generations)
{
    // print file header
    if (gen == 0)
        chains[0]->monitor(0);
    
    // run chain
    for (size_t i = 1; i <= generations; i += swapInterval)
    {
        // start parallel job per block of swapInterval cycles
        size_t np = numProcesses; // in fact, used by the macro below
        int pid = 1;
        
        #pragma omp parallel default(shared) private(np, pid)
        {
            #if defined (USE_LIB_OPENMP)
                pid = omp_get_thread_num();
            #endif
            
            // Create process per chain
            for (size_t j = 0; j < chainsPerProcess[pid].size(); j++)
            {
                // get chain index from job vector
                size_t chainIdx = chainsPerProcess[pid][j];
                
                // Advance cycles in blocks of size swapInterval
                for (size_t k = 0; k < swapInterval && (i+k) <= generations; k++)
                {
                    // advance chain j by a single cycle
                    chains[chainIdx]->nextCycle(true);

                    // monitor chain activeIndex only
                    //if (chainIdx == activeIndex)
                    if (chains[chainIdx]->isChainActive() )
                    {
                        //std::cout << i + k << " only one\n";
                        //chains[activeIndex]->
                        chains[chainIdx]->monitor(i+k);
                    }
                    //std::cout << chainIdx << "    lnPosterior  " << chains[chainIdx]->getLnPosterior() << " chainHeat  " << chains[chainIdx]->getChainHeat() << "\n";
                    //chains[chainIdx]->monitor(i+k);
                }
            }
            
            // synchronize chains
            #pragma omp barrier
            
        } // processor job end
        
        // advance gen counter
        gen += swapInterval;
        
        // perform chain swap
        swapChains();
    }
}

// MJL: allow swapChains to take a swap function -- e.g. pairwise swap for 1..n-1
void ParallelMcmcmc::swapChains(void)
{
    size_t numChains = chains.size();
    double lnProposalRatio = 0.0;
    
    // exit if there is only one chain
    if (numChains < 2)
        return;
    
    int numAccepted = 0;
    
    //for (size_t i = 1; i < numChains; i++)
    for (size_t i = numChains-1; i > 0; i--)
    {
        
        // swap adjacent chains
        int j = chainIdxByHeat[i-1];
        int k = chainIdxByHeat[i];
        
        // compute exchange ratio
        double bj = chains[j]->getChainHeat();
        double bk = chains[k]->getChainHeat();
        double lnPj = chains[j]->getLnPosterior();
        double lnPk = chains[k]->getLnPosterior();
        double lnR = bj * (lnPk - lnPj) + bk * (lnPj - lnPk) + lnProposalRatio;
        
        // determine whether we accept or reject the chain swap
        bool accept = false;
        double u = GLOBAL_RNG->uniform01();
        if (lnR >= 0)
            accept = true;
        else if (lnR < -100)
            accept = false;
        else if (u < exp(lnR))
            accept = true;
        else
            accept = false;
        
        if (accept == true)
            numAccepted++;
        
        // test override
        //accept = true;
        //std::cout << "\nbj " << bj << "; bk " << bk << "; lnPj " << lnPj << "; lnPk " << lnPk << "\n";
        //std::cout << "bj*(lnPk-lnPj) " << bj*(lnPk-lnPj) << "; bk*(lnPj-lnPk) " << bk*(lnPj-lnPk) << "\n";
        //std::cout << "swapChains()\t" << j << " <--> " << k << "  " << lnR << "\n";
        //std::cout << u << "  " << exp(lnR) << "  " << (accept ? "accept\n" : "reject\n");
              
        // on accept, swap beta values and active chains
        if (accept)
        {
            // swap chain heat values
            chains[j]->setChainHeat(bk);
            chains[k]->setChainHeat(bj);
            
            //size_t tmpIdx = j;
            chainIdxByHeat[i-1] = k;
            chainIdxByHeat[i] = j;

            // swap active chain
            if (activeIndex == j)
            {
                activeIndex = (int)k;
                chains[j]->setChainActive(false);
                chains[k]->setChainActive(true);
            }
            else if (activeIndex == k)
            {
                activeIndex = (int)j;
                chains[j]->setChainActive(true);
                chains[k]->setChainActive(false);
            }
        }
        //std::cout << "activeIndex " << activeIndex << "\n";
    }
   
    /*
    for (int j = 0; j < numChains; j++)
    {
        int i = chainIdxByHeat[j];
        std::cout << i << " " << chains[i]->getChainHeat() << " " << chains[i]->getLnPosterior() << " == " << chains[i]->getModelLnProbability() << " " << (chains[i]->isChainActive() ? "*" : "") << (i == activeIndex ? "#" : "") << "\n";
    }
    std::cout << "freq accepted: " << (double)numAccepted/(numChains-1) << "\n";
    
    std::cout << "\n";
    */
    
}
