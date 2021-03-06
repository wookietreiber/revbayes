/**
 * @file
 * This file contains the declaration and implementation
 * of the templated Func__add, which is used to add two
 * variables.
 *
 * @brief Declaration and implementation of Func__add
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2012-04-20 04:06:14 +0200 (Fri, 20 Apr 2012) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 *
 * $Id: Func__add.h 1406 2012-04-20 02:06:14Z hoehna $
 */

#ifndef Func__add_H
#define Func__add_H

#include "RlTypedFunction.h"

#include <map>
#include <string>

namespace RevLanguage {

template <typename firstValType, typename secondValType, typename retType>
class Func__add : public TypedFunction<retType> {
    
public:
    Func__add( void );
    
    // Basic utility functions
    Func__add*                                                      clone(void) const;                              //!< Clone the object
    static const std::string&                                       getClassType(void);                             //!< Get class name
    static const TypeSpec&                                          getClassTypeSpec(void);                         //!< Get class type spec
    const TypeSpec&                                                 getTypeSpec(void) const;                        //!< Get the type spec of the instance

    // Implementations of pure virtual functions of the base class(es)
    RevBayesCore::TypedFunction<typename retType::valueType>*       createFunction(void) const ;                    //!< Create a new internal function object
    const ArgumentRules&                                            getArgumentRules(void) const;                   //!< Get argument rules
    
private:
        
};
    
}

#include "BinaryAddition.h"
#include "RlDeterministicNode.h"
#include "StringUtilities.h"
#include "TypedDagNode.h"

/** default constructor */
template <typename firstValType, typename secondValType, typename retType>
RevLanguage::Func__add<firstValType, secondValType, retType>::Func__add( void ) : TypedFunction<retType>( )
{
    
}


/** Clone object */
template <typename firstValType, typename secondValType, typename retType>
RevLanguage::Func__add<firstValType, secondValType, retType>* RevLanguage::Func__add<firstValType, secondValType, retType>::clone( void ) const
{
    
    return new Func__add( *this );
}


template <typename firstValType, typename secondValType, typename retType>
RevBayesCore::TypedFunction<typename retType::valueType>* RevLanguage::Func__add<firstValType, secondValType, retType>::createFunction( void ) const
{
    RevBayesCore::TypedDagNode<typename firstValType::valueType>* firstArg = static_cast<const firstValType &>( this->args[0].getVariable()->getRevObject() ).getDagNode();
    RevBayesCore::TypedDagNode<typename secondValType::valueType>* secondArg = static_cast<const secondValType &>( this->args[1].getVariable()->getRevObject() ).getDagNode();
    RevBayesCore::BinaryAddition<typename firstValType::valueType, typename secondValType::valueType, typename retType::valueType> *func = new RevBayesCore::BinaryAddition<typename firstValType::valueType, typename secondValType::valueType, typename retType::valueType>(firstArg, secondArg);
    
    return func;
}


/* Get argument rules */
template <typename firstValType, typename secondValType, typename retType>
const RevLanguage::ArgumentRules& RevLanguage::Func__add<firstValType, secondValType, retType>::getArgumentRules( void ) const
{
    
    static ArgumentRules argumentRules = ArgumentRules();
    static bool          rulesSet = false;
    
    if ( !rulesSet )
    {
        
        argumentRules.push_back( new ArgumentRule( "first" , firstValType::getClassTypeSpec() , ArgumentRule::BY_CONSTANT_REFERENCE ) );
        argumentRules.push_back( new ArgumentRule( "second", secondValType::getClassTypeSpec(), ArgumentRule::BY_CONSTANT_REFERENCE ) );
        rulesSet = true;
    }
    
    return argumentRules;
}


template <typename firstValType, typename secondValType, typename retType>
const std::string& RevLanguage::Func__add<firstValType, secondValType, retType>::getClassType(void)
{
    static std::string revType = "Func__add<" + firstValType::getClassType() + "," + secondValType::getClassType() + "," + retType::getClassType() + ">";
    
	return revType; 
}

/* Get class type spec describing type of object */
template <typename firstValType, typename secondValType, typename retType>
const RevLanguage::TypeSpec& RevLanguage::Func__add<firstValType, secondValType, retType>::getClassTypeSpec(void)
{
    
    static TypeSpec revTypeSpec = TypeSpec( Func__add<firstValType, secondValType, retType>::getClassType(), new TypeSpec( Function::getClassTypeSpec() ) );
    
	return revTypeSpec; 
}


template <typename firstValType, typename secondValType, typename retType>
const RevLanguage::TypeSpec& RevLanguage::Func__add<firstValType, secondValType, retType>::getTypeSpec( void ) const
{
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}

#endif
