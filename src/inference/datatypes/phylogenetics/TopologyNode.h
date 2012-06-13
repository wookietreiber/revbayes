/**
 * @file
 * This file contains the declaration of a TopologyNode. Tree nodes are member objects and therefore can hold
 * all the variables one might to associate to the tree. The tree nodes are used to create the structure
 * of a tree. They provide access to their parent and children.
 *
 * The usage of tree nodes is to create and give easy access to parts of the dag, namely variables hold
 * by the nodes which need access to ancestral variables.
 *
 * We do not distinguish between branch parameter and node parameters. A branch parameter is simply set as
 * a node parameter of the descending node.
 *
 * A tree node can have a distribution associated with it, for instance in the situation when we condition
 * on a group of taxa being monophyletic.
 *
 *
 * NOTE: This class might be a temporary solution being the unified solution for all tree nodes. In the future
 * we might make this class abstract and implement at least the two types of tree nodes: bifurcating tree nodes
 * which are restricted two have exactly two descendants and multifurcating tree nodes which can have any number
 * of tree nodes. Perhaps there might be also tip nodes as a derived class and a root node, which is the 
 * correct OO design approach but comes with a lot of overhead. A fast conversion method would be needed.
 *
 * @brief Declaration of TreeNode
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes core development team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-12-04, version 1.0
 * @extends MemberObject
 *
 * $Id$
 */

#ifndef TopologyNode_H
#define TopologyNode_H

#include "MemberObject.h"
#include "RbBoolean.h"
#include "RbString.h"

#include <vector>


class TopologyNode : public MemberObject {
    
    public:
                                            TopologyNode(int indx=0);                                                   //!< Default constructor with optional index
                                            TopologyNode(const std::string& n, int indx=0);                             //!< Constructor with name and optional index
                                            TopologyNode(const TopologyNode &n);                                        //!< Copy constructor
        virtual                            ~TopologyNode(void);                                                         //!< Destructor
        
        TopologyNode&                       operator=(const TopologyNode& n);
    
        // Basic utility functions
        TopologyNode*                       clone(void) const;                                                          //!< Clone object
        bool                                equals(const TopologyNode& node) const;                                     //!< Test whether this is the same node
        static const std::string&           getClassName(void);                                                         //!< Get class name
        static const TypeSpec&              getClassTypeSpec(void);                                                     //!< Get class type spec
        const TypeSpec&                     getTypeSpec(void) const;                                                    //!< Get language type of the object
        void                                printValue(std::ostream& o) const;                                          //!< Print value for user

        // Member variable rules
        const MemberRules&                  getMemberRules(void) const;                                                 //!< Get member rules

        // Member method inits
        RbPtr<RbLanguageObject>             executeSimpleMethod(const std::string& name, const std::vector<const RbObject*>& args);         //!< Override to map member methods to internal functions
        const MethodTable&                  getMethods(void) const;                                                     //!< Get methods
            
        // TopologyNode functions
        void                                addChild(TopologyNode* c);                                                  //!< Adds a child node
        const TopologyNode&                 getChild(size_t i) const;                                                   //!< Returns the i-th child
        TopologyNode&                       getChild(size_t i);                                                         //!< Returns the i-th child (non-const to return non-const node)
        std::vector<int>                    getChildrenIndices(void) const;                                             //!< Return children indices
        int                                 getIndex(void) const;                                                       //!< Get index of node
        const std::string&                  getName(void) const;                                                        //!< Get name of node
        size_t                              getNumberOfChildren(void) const;                                            //!< Returns the number of children
        const TopologyNode&                 getParent(void) const;                                                      //!< Returns the node's parent
        int                                 getParentIndex(void) const;                                                 //!< Return parent index
        bool                                isTip(void) const;                                                          //!< Is node tip?
        bool                                isRoot(void) const;                                                         //!< Is node root?
        void                                setName(const std::string& n);                                              //!< Set the name of this node
        void                                setParent(TopologyNode* p);                                                 //!< Sets the node's parent
        void                                removeAllChildren(void);                                                    //!< Removes all of the children of the node
        void                                removeChild(TopologyNode* p);                                               //!< Removes a specific child
                 
    private:
        std::string                         buildNewickString(const TopologyNode& node) const;                          //!< compute the newick string for a tree rooting at this node
        void                                refreshNewickString(void);                                                  //!< recompute the newick string
        
        std::vector<TopologyNode*>          children;                                                                   //!< Vector holding the node's children. Note that the parent owns the children but not the other way around. 
        TopologyNode*                       parent;                                                                     //!< Pointer to the parent of the node. It is a regular pointer instead of a super smart pointer to avoid loops in the reference counting.
        std::string                         name;                                                                       //!< Name of the node, i.e. identifier/taxon name
        int                                 index;                                                                      //!< Node index
        bool                                isInteriorNode;
        bool                                isRootNode;
        bool                                isTipNode;
};

#endif