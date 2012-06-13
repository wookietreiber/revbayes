/**
 * @file
 * This file contains the declaration and implementation
 * of the templated Func_set, which is used to create
 * value sets.
 *
 * @brief Declaration and implementation of Func_set
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2012-06-12 10:25:58 +0200 (Tue, 12 Jun 2012) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 *
 * $Id: Func_set.h 1626 2012-06-12 08:25:58Z hoehna $
 */

#ifndef Func_set_H
#define Func_set_H

#include "RbFunction.h"
#include <map>
#include <string>

template <typename valType>
class Func_set :  public RbFunction {
    
public:
    Func_set();
    
    // Basic utility functions
    Func_set*                   clone(void) const;                                          //!< Clone the object
    static const std::string&   getClassName(void);                                         //!< Get class name
    static const TypeSpec&      getClassTypeSpec(void);                                     //!< Get class type spec
    const TypeSpec&             getTypeSpec(void) const;                                    //!< Get language type of the object
    
    // Regular functions
    const ArgumentRules&        getArgumentRules(void) const;                               //!< Get argument rules
    const TypeSpec&             getReturnType(void) const;                                  //!< Get type of return value
    
protected:
    RbPtr<RbLanguageObject>     executeFunction(const std::vector<const RbObject*>& args);  //!< Execute function
    
};


#include "ArgumentRule.h"
#include "Ellipsis.h"
#include "RbUtil.h"
#include "RlSet.h"
#include "TypeSpec.h"



template <typename valType>
Func_set<valType>::Func_set() : RbFunction() {
    
}

/* Clone object */
template <typename valType>
Func_set<valType>* Func_set<valType>::clone( void ) const {
    
    return new Func_set( *this );
}


/** Execute function: We rely on getValue and overloaded push_back to provide functionality */
template <typename valType>
RbPtr<RbLanguageObject> Func_set<valType>::executeFunction( const std::vector<const RbObject*>& args ) {
    
    RlSet<valType> *theSet = new RlSet<valType>();
    for ( size_t i = 0; i < args.size(); i++ )
        theSet->push_back( *static_cast<const valType*>( args[i] ) );
    
    return theSet;
}


/** Get argument rules */
template <typename valType>
const ArgumentRules& Func_set<valType>::getArgumentRules( void ) const {
    
    static ArgumentRules argumentRules = ArgumentRules();
    static bool          rulesSet = false;
    
    if ( !rulesSet ) 
    {
        argumentRules.push_back( new ArgumentRule( "", true, valType::getClassTypeSpec() ) );
        argumentRules.push_back( new Ellipsis (     valType::getClassTypeSpec() ) );
        rulesSet = true;
    }
    
    return argumentRules;
}


/** Get class name of object */
template <typename valType>
const std::string& Func_set<valType>::getClassName(void) { 
    
    static std::string rbClassName = "Func_set<" + valType::getClassTypeSpec() + ">";
    
	return rbClassName; 
}


/** Get class type spec describing type of object */
template <typename valType>
const TypeSpec& Func_set<valType>::getClassTypeSpec(void) { 
    
    static TypeSpec rbClass = TypeSpec( "Func_set", new TypeSpec( RbFunction::getClassTypeSpec() ) );
    
	return rbClass; 
}


/** Get type spec */
template <typename valType>
const TypeSpec& Func_set<valType>::getTypeSpec( void ) const {
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}


/** Get return type */
template <typename valType>
const TypeSpec& Func_set<valType>::getReturnType( void ) const {
    
    return RlSet<valType>::getClassTypeSpec();
}


#endif

