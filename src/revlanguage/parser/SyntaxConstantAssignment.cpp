/**
 * @file
 * This file contains the implementation of SyntaxAssignExpr, which is
 * used to hold binary expressions in the syntax tree.
 *
 * @brief Implementation of SyntaxAssignExpr
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2012-09-07 12:47:31 +0200 (Fri, 07 Sep 2012) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 *
 * $Id: SyntaxAssignExpr.cpp 1801 2012-09-07 10:47:31Z hoehna $
 */

#include "ArgumentRule.h"
#include "RbException.h"
#include "RbUtil.h"
#include "RbOptions.h"
#include "RlDistribution.h"
#include "SyntaxFunctionCall.h"
#include "VariableSlot.h"
#include "SyntaxConstantAssignment.h"
#include "Workspace.h"

#include <iostream>
#include <list>
#include <sstream>

using namespace RevLanguage;

/** Construct from operator type, variable and expression */
SyntaxConstantAssignment::SyntaxConstantAssignment(SyntaxVariable* var, SyntaxElement* expr) : SyntaxElement(), variable(var), functionCall(NULL), expression(expr) {

}


/** Construct from operator type, function call and expression */
SyntaxConstantAssignment::SyntaxConstantAssignment(SyntaxFunctionCall* fxnCall, SyntaxElement* expr) : SyntaxElement(), variable(NULL), functionCall(fxnCall), expression(expr) {

}


/** Deep copy constructor */
SyntaxConstantAssignment::SyntaxConstantAssignment(const SyntaxConstantAssignment& x) : SyntaxElement(x) {
    
    if ( x.variable != NULL )
        variable   = x.variable->clone();
    
    if ( x.functionCall != NULL )
        functionCall = x.functionCall->clone();
    
    expression = x.expression->clone();
}


/** Destructor deletes operands */
SyntaxConstantAssignment::~SyntaxConstantAssignment() {
    
    delete variable;
    delete functionCall;
    delete expression;
    
}


/** Assignment operator */
SyntaxConstantAssignment& SyntaxConstantAssignment::operator=(const SyntaxConstantAssignment& x) {
    
    if ( this != &x ) {
        
        functionCall = NULL;
        variable = NULL;
        
        if ( x.variable != NULL )
            variable   = x.variable;
        
        if ( x.functionCall != NULL )
            functionCall = x.functionCall;
        
        expression = x.expression;
    }
    
    return (*this);
}


/** Clone syntax element */
SyntaxConstantAssignment* SyntaxConstantAssignment::clone () const {
    
    return new SyntaxConstantAssignment(*this);
}


/** Get semantic value: insert symbol and return the rhs value of the assignment */
RbPtr<Variable> SyntaxConstantAssignment::evaluateContent( Environment& env ) {
    
#ifdef DEBUG_PARSER
    printf( "Evaluating assign expression\n" );
#endif
    
    // Get variable info from lhs
    const RbPtr<Variable>& theSlot = variable->createVariable( env );
    
    // Declare variable storing the return value of the assignment expression
    RbPtr<Variable> theVariable = NULL;
        
#ifdef DEBUG_PARSER
    printf("Constant assignment\n");
#endif
        
    RbLanguageObject *newValue;
        
    // Calculate the value of the rhs expression
    theVariable = expression->evaluateContent( env );
    if ( theVariable == NULL )
        throw RbException( "Invalid NULL variable returned by rhs expression in assignment" );
        
    // fill the slot with the new variable
    const RbLanguageObject& value = theVariable->getValue();
        
        
    // check if the type is valid. This is necessary for reassignments
    if ( !value.getTypeSpec().isDerivedOf( theSlot->getValueTypeSpec() ) ) {
        // We are not of a derived type (or the same type)
        // since this will create a constant node we are allowed to type cast
        if (value.isConvertibleTo( theSlot->getValueTypeSpec() ) ) {
            newValue = value.convertTo( theSlot->getValueTypeSpec() );
                
        }
        else {
            std::ostringstream msg;
            msg << "Cannot reassign variable '" << theSlot->getName() << "' with type " << value.getTypeSpec() << " with value ";
            value.printValue(msg);
            msg << " because the variable requires type " << theSlot->getValueTypeSpec() << "." << std::endl;
            throw RbException( msg );
        }
    }
    else {
        newValue = value.clone();
        newValue->makeConstantValue();
            
    }
        
    // set the value of the variable
    theSlot->setValue( newValue );
        
    // set the name of the DAG node. This will ensure nicer outputs about the DAG.
    newValue->setName( theSlot->getName() );
        
#ifdef DEBUG_PARSER
    env.printValue(std::cerr);
#endif    
    
    return theVariable;
}


bool SyntaxConstantAssignment::isAssignment( void ) const {
    return true;
}



/** Print info about the syntax element */
void SyntaxConstantAssignment::printValue(std::ostream& o) const {
    
    o << "SyntaxConstantAssignment:" << std::endl;
    o << "variable      = ";
    variable->printValue(o);
    o << std::endl;
    o << "expression    = ";
    expression->printValue(o);
    o << std::endl;
}


/**
 * Replace the syntax variable with name by the constant value. Loops have to do that for their index variables.
 * We just delegate that to the element on our right-hand-side and also to the variable itself (lhs).
 */
void SyntaxConstantAssignment::replaceVariableWithConstant(const std::string& name, const RbLanguageObject& c) {
    expression->replaceVariableWithConstant(name, c);
    variable->replaceVariableWithConstant(name, c);
}

