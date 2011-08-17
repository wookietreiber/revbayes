/**
 * @file
 * This file contains the declaration of Func_normalize, which 
 * normalizes a vector of positive real numbers.
 *
 * @brief Declaration of Func_normalize
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 *
 * $Id$
 */

#ifndef Func_normalize_H
#define Func_normalize_H

#include "RbFunction.h"

#include <map>
#include <string>
#include <vector>

class DAGNode;
class VectorString;

class Func_normalizeVector :  public RbFunction {

    public:
        // Basic utility functions
        Func_normalizeVector*       clone(void) const;                                          //!< Clone the object
    	const VectorString&         getClass(void) const;                                       //!< Get class vector

        // Regular functions
        bool                        addAsChildOfArguments(void) { return false; }               //!< We do not wish that this function is added as a child of the arguments
    	RbLanguageObject*           execute(void);                                              //!< Execute function
        const ArgumentRules&        getArgumentRules(void) const;                               //!< Get argument rules
        const TypeSpec              getReturnType(void) const;                                  //!< Get type of return value

};

#endif
