//
//  AdmixtureEdgeMultiRemove.h
//  rb_mlandis
//
//  Created by Michael Landis on 1/13/14.
//  Copyright (c) 2014 Michael Landis. All rights reserved.
//

#ifndef __rb_mlandis__AdmixtureEdgeMultiRemove__
#define __rb_mlandis__AdmixtureEdgeMultiRemove__


#include <ostream>
#include <set>
#include <string>
#include <list>

#include "ConstantNode.h"
#include "DeterministicNode.h"
#include "Move.h"
#include "StochasticNode.h"
#include "AdmixtureNode.h"
#include "AdmixtureTree.h"
#include "Tree.h"

namespace RevBayesCore {
    /*
    struct AdmixtureEdgePosition
    {
        AdmixtureNode* admixtureParent;
        AdmixtureNode* admixtureChild;
        AdmixtureNode* admixtureParentChild;
        AdmixtureNode* admixtureChildChild;
        double age;
        double weight;
        AdmixtureEdgePosition(AdmixtureNode* p, AdmixtureNode* c, AdmixtureNode* pc, AdmixtureNode* cc, double a, double w) : admixtureParent(p), admixtureChild(c), admixtureParentChild(pc), admixtureChildChild(cc), age(a), weight(w)
        {
            ;//std::cout << "hello\n";
        }
    };
     */
    
    class AdmixtureEdgeMultiRemove : public Move {
        
    public:
        //AdmixtureEdgeMultiRemove( StochasticNode<AdmixtureTree> *n, StochasticNode<double>* r, DeterministicNode<std::vector<double> >* res, ConstantNode<int>* dt, double weight);                                            //!<  constructor
        AdmixtureEdgeMultiRemove( StochasticNode<AdmixtureTree> *n, StochasticNode<double>* r, StochasticNode<int>* ac, DeterministicNode<std::vector<double> >* res, double delta, double p, int ag, double weight);                                            //!<  constructor
        
        
        // Basic utility functions
        AdmixtureEdgeMultiRemove*       clone(void) const;                                                                  //!< Clone object
        void                            swapNode(DagNode *oldN, DagNode *newN);
        bool                            isActive(int g) const;
        
    protected:
        const std::string&              getMoveName(void) const;                                                            //!< Get the name of the move for summary printing
        double                          performSimpleMove(void);                                                            //!< Perform move
        void                            rejectSimpleMove(void);
        void                            acceptSimpleMove(void);
        
        void                            acceptMove(void);                                                                   //!< Accept the InferenceMoveSimple
        double                          performMove(double& probRatio);                                                     //!< Perform the InferenceMoveSimple
        void                            rejectMove(void);
        
    private:
        
        void                            findDescendantTips(std::set<AdmixtureNode*>& s, AdmixtureNode* p);
        
        // member variables
        StochasticNode<AdmixtureTree>*  variable;
        StochasticNode<double>*         rate;
        StochasticNode<int>*            admixtureCount;
        DeterministicNode<std::vector<double> >* residuals;
        //ConstantNode<int>*              delayTimer;
        bool changed;
        int activeGen;
        int numEvents;
        double delta;
        double pRemove;
        
        // stored objects to undo proposal
        bool                            failed;
        AdmixtureNode*                  storedAdmixtureChild;
        AdmixtureNode*                  storedAdmixtureParent;
        AdmixtureNode*                  storedAdmixtureChildChild;
        AdmixtureNode*                  storedAdmixtureParentChild;
        AdmixtureNode*                  storedAdmixtureChildParent;
        AdmixtureNode*                  storedAdmixtureParentParent;
        std::vector<double>             storedResiduals;
        
        std::list<AdmixtureEdgePosition> storedAdmixtureEdges;
        
    };
    
}

#endif /* defined(__rb_mlandis__AdmixtureEdgeMultiRemove__) */