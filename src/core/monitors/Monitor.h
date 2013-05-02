/**
 * @file
 * This file contains the declaration of the monitor base class, used to hold information
 * about the monitoring of a variable DAG node.
 *
 * @brief Declaration of the monitor base class
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2012-06-19, version 1.0
 *
 * $Id$
 */

#ifndef Monitor_H
#define Monitor_H

#include <ostream>
#include <set>

namespace RevBayesCore {

    class DagNode;
    class Model;

    class Monitor {
    
    public:
        // Constructors and Destructors
        Monitor(int printgen, DagNode *n);                                                                         //!< Default Constructor
        Monitor(int printgen, const std::set<DagNode *> &n);                                                       //!< Default Constructor
        Monitor(const Monitor &x);                                                                                          //!< Copy Constructor
        virtual ~Monitor(void);                                                                                             //!< Destructor

        // overloaded operators
        Monitor&                                    operator=(const Monitor &i);
            
        // pure virtual member functions
        virtual Monitor*                            clone(void) const = 0;
        virtual void                                monitor(long gen) = 0;                                                  //!< InferenceMonitor at generation gen

        // methods you may want to overwrite
        virtual void                                swapNode(DagNode *oldN, DagNode *newN);

        // getters and setters
        const std::set<DagNode *>&                  getDagNodes(void) const;                                                //!< Get the nodes vector
        void                                        setDagNodes(const std::set<DagNode *>& args);
        void                                        setModel(Model* m);

    protected:
    
        // parameters
        int                                         printgen;
        Model*                                      model;
        std::set<DagNode *>                         nodes;
    
    };
    
}



// Global functions using the class
std::ostream&                       operator<<(std::ostream& o, const RevBayesCore::Monitor& x);   

#endif
