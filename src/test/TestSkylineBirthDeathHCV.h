/**
 * @file
 * This file contains the declaration of the Birth-Death model test class. 
 * This test runs an MCMC analysis on a birth-death model when we have the tree observed and want to estimate the 
 * diversification rates.
 *
 *
 * @brief Declaration of the B-D model test class
 *
 * (c) Copyright 2009-
 * @date Last modified: $Date:$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @since Version 1.0, 2012-07-05
 *
 * $Id:$
 */


#ifndef TestSkylineBirthDeathHCV_H
#define TestSkylineBirthDeathHCV_H

#include "RbFileManager.h"

#include <string>
#include <vector>

namespace RevBayesCore {
    
    class TimeTree;

    class TestSkylineBirthDeathHCV {
    
    public:
        TestSkylineBirthDeathHCV(const std::string &aFn, const std::string &tFn, int gen);
        virtual                         ~TestSkylineBirthDeathHCV(void);                                                            //!< Virtual destructor
    
        bool                            run();
    
    private:    
        // members
		std::string                     alignmentFilename;
		std::string                     treeFilename;
        int                             mcmcGenerations;

    };

}

#endif
