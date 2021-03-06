#ifndef SumFunction_H
#define SumFunction_H

#include "RbVector.h"
#include "TypedFunction.h"
#include "TypedDagNode.h"

namespace RevBayesCore {
    
    /**
     * \brief Function for computation of the sum of some numbers.
     *
     * This class is the function that computes the sum of some numbers.
     * The numbers are passed in as a DAG node whose value type is a std::vector<double>.
     *
     * \copyright (c) Copyright 2009-2013 (GPL version 3)
     * \author The RevBayes Development Core Team (Sebastian Hoehna)
     * \since Version 1.0, 2014-07-27
     *
     */
    class SumFunction : public TypedFunction<double> {
        
    public:
        SumFunction(const TypedDagNode<RbVector<double> > * v);
        virtual                                            ~SumFunction(void);                                                         //!< Virtual destructor
        
        // public member functions
        SumFunction*                                        clone(void) const;                                                          //!< Create an independent clone
        void                                                update(void);
        
    protected:
        void                                                swapParameterInternal(const DagNode *oldP, const DagNode *newP);            //!< Implementation of swaping parameters
        
    private:
        
        // members
        const TypedDagNode<RbVector<double> >*              vals;
        
    };
    
}


#endif
