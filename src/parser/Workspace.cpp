/**
 * @file
 * This file contains the implementation of Workspace, which is
 * used to hold the global workspace and the user workspace.
 *
 * @brief Implementation of Workspace
 *
 * (c) Copyright 2009-
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @extends Frame
 * @package parser
 * @version 1.0
 * @since version 1.0 2009-09-02
 *
 * $Id$
 */

// Regular include files
#include "ConstantNode.h"
#include "ConstructorFunction.h"
#include "Container.h"
#include "Distribution.h"
#include "DistributionReal.h"
#include "DistributionFunction.h"
#include "FunctionTable.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RbException.h"
#include "RbFunction.h"
#include "RbObject.h"
#include "RbOptions.h"         // For PRINTF
#include "UserInterface.h"
#include "ValueContainer.h"
#include "VariableContainer.h"
#include "Workspace.h"

// Objects added to the workspace in initializeGlobalWorkspace()
#include "Boolean.h"
#include "Dist_dirichlet.h"
#include "Dist_exp.h"
#include "Dist_multinomial.h"
#include "Dist_norm.h"
#include "Dist_unif.h"
#include "Func__add.h"
#include "Func__and.h"
#include "Func__div.h"
#include "Func__eq.h"
#include "Func__ge.h"
#include "Func__gt.h"
#include "Func__le.h"
#include "Func__lt.h"
#include "Func__mul.h"
#include "Func__ne.h"
#include "Func__or.h"
#include "Func__range.h"
#include "Func__sub.h"
#include "Func_clamp.h"
#include "Func_ls.h"
#include "Func_model.h"
#include "Func_normalize.h"
#include "Func_quit.h"
#include "Func_s_doublevec.h"
#include "Func_s_int.h"
#include "Func_s_realvec.h"
#include "Func_source.h"
#include "Func_sqrt.h"
#include "Func_transpose.h"
#include "Func_v_int.h"
#include "Func_v_double.h"
#include "Func_v_vecrealvec.h"
#include "MatrixReal.h"
#include "Mcmc.h"
#include "Move_mmultinomial.h"
#include "Move_mscale.h"
#include "Move_msimplex.h"
#include "Move_mslide.h"
#include "RealPos.h"
#include "Integer.h"
#include "Real.h"
#include "Simplex.h"
#include "VectorReal.h"
#include "VectorRealPos.h"

#include <sstream>
#include <vector>


/** Constructor of global workspace */
Workspace::Workspace() : Frame(), functionTable(new FunctionTable()) {

}


/** Constructor of user workspace */
Workspace::Workspace(Workspace* parentSpace) : Frame(parentSpace), functionTable(new FunctionTable(globalWorkspace().getFunctionTable())) {

}


/** Destructor */
Workspace::~Workspace() {

    delete functionTable;
    for (TypeTable::iterator i=typeTable.begin(); i!=typeTable.end(); i++)
        delete (*i).second;
}


/** Assignment operator */
Workspace& Workspace::operator=(const Workspace& x) {

    if (this != &x) {

        delete functionTable;
        functionTable = new FunctionTable(x.functionTable);
    }

    return (*this);
}


/** Add distribution to the workspace */
bool Workspace::addDistribution(const std::string& name, Distribution* dist) {

    PRINTF("Adding distribution to workspace\n");

    if (typeTable.find(name) != typeTable.end())
        throw RbException("There is already a type named '" + name + "' in the workspace");

    typeTable.insert(std::pair<std::string, RbObject*>(name, dist->clone()));

    functionTable->addFunction(name, new ConstructorFunction(dist));
    functionTable->addFunction("d" + name, new DistributionFunction(dist, DistributionFunction::DENSITY));
    functionTable->addFunction("r" + name, new DistributionFunction((Distribution*)(dist->clone()), DistributionFunction::RVALUE));

    return true;
}


/** Add real-valued distribution to the workspace */
bool Workspace::addDistribution(const std::string& name, DistributionReal* dist) {

    PRINTF("Adding real-valued distribution %s to workspace\n", name.c_str());

    if (typeTable.find(name) != typeTable.end())
        throw RbException("There is already a type named '" + name + "' in the workspace");

    typeTable.insert(std::pair<std::string, RbObject*>(name, dist->clone()));

    functionTable->addFunction(name, new ConstructorFunction(dist));
    functionTable->addFunction("d" + name, new DistributionFunction(dist, DistributionFunction::DENSITY));
    functionTable->addFunction("r" + name, new DistributionFunction((Distribution*)(dist->clone()), DistributionFunction::RVALUE));
    functionTable->addFunction("p" + name, new DistributionFunction((Distribution*)(dist->clone()), DistributionFunction::PROB));
    functionTable->addFunction("q" + name, new DistributionFunction((Distribution*)(dist->clone()), DistributionFunction::QUANTILE));

    return true;
}


/** Add function to the workspace */
bool Workspace::addFunction(const std::string& name, RbFunction* func) {

    PRINTF("Adding function %s to workspace\n", name.c_str());
    
    if (existsVariable(name))
        throw RbException("There is already a variable named '" + name + "' in the workspace");

    functionTable->addFunction(name, func);
    
    return true;
}


/** Add type to the workspace */
bool Workspace::addType(RbObject* exampleObj) {

    PRINTF("Adding type %s to workspace\n", exampleObj->getTypeSpec().toString().c_str());

    std::string name = exampleObj->getType();

    if (typeTable.find(name) != typeTable.end())
        throw RbException("There is already a type named '" + name + "' in the workspace");

    typeTable.insert(std::pair<std::string, RbObject*>(exampleObj->getType(), exampleObj));

    return true;
}


/** Add type with constructor to the workspace */
bool Workspace::addTypeWithConstructor(const std::string& name, MemberObject* templ) {

    PRINTF("Adding type %s with constructor to workspace\n", name.c_str());

    if (typeTable.find(name) != typeTable.end())
        throw RbException("There is already a type named '" + name + "' in the workspace");

    typeTable.insert(std::pair<std::string, RbObject*>(name, templ->clone()));

    functionTable->addFunction(name, new ConstructorFunction(templ));

    return true;
}


/** Execute function to get its value (workspaces only evaluate functions once) */
DAGNode* Workspace::executeFunction(const std::string& name, const std::vector<Argument>& args) const {

    /* Using this calling convention indicates that we are only interested in
       evaluating the function once */
    return functionTable->executeFunction(name, args);
}


/** Get function */
RbFunction* Workspace::getFunction(const std::string& name, const std::vector<Argument>& args) {

    return functionTable->getFunction(name, args);
}


/** Check and get reference to the type name of an object type */
const std::string& Workspace::getTypeNameRef( const std::string& name ) const {

    return getTypeSpec( name ).getType();
}


/** Check and get type specification for a named object type */
TypeSpec Workspace::getTypeSpec( const std::string& name ) const {

    if ( name == ValueContainer_name || name == VariableContainer_name ) {
        
        // We cannot rely on the type spec provided by generic container type dummies in the type table
        // because they do not know the relevant element dimensions. There should not be such dummies
        // in the type table anyway, but we add an extra check here just to make sure
        throw RbException( "Invalid attempt to convert a generic container type to valid type specification" );
    }

    std::map<std::string, RbObject*>::const_iterator it = typeTable.find( name );
    if ( it == typeTable.end() )
        throw RbException( "No object class with name " + name );
    return (*it).second->getTypeSpec();
}


/** Check and get type specification for a named object type */
TypeSpec Workspace::getTypeSpec( const TypeSpec& typeSp ) const {

    if ( typeSp.getType() == ValueContainer_name || typeSp.getType() == VariableContainer_name ) {
        
        // We cannot rely on the type spec provided by generic container type dummies in the type table
        // because they do not know the relevant element dimensions. There should not be such dummies
        // in the type table anyway, but we add an extra check here just to make sure
        throw RbException( "Invalid attempt to convert a generic container type to valid type specification" );
    }

    std::map<std::string, RbObject*>::const_iterator it = typeTable.find( typeSp.getType() );
    if ( it == typeTable.end() )
        throw RbException( "No object class with name " + typeSp.getType() );
    return (*it).second->getTypeSpec();
}


/** Convenient access to default random number generator */
RandomNumberGenerator* Workspace::get_rng(void) {

    return (RandomNumberGenerator*) getValue("_rng");
}


/** Initialize global workspace */
void Workspace::initializeGlobalWorkspace(void) {

    try 
        {
        /* Add types: add a dummy variable which we use for type checking, conversion checking and other tasks. */
        addType( new Boolean()             );
        addType( new Integer()             );
        addType( new MatrixReal()          );
        addType( new Real()                );
        addType( new RealPos()             );
        addType( new Simplex()             );
        addType( new VectorInteger()       );
        addType( new VectorReal()          );
        addType( new VectorRealPos()       );

        /* Add member object types with auto-generated constructors */
        addTypeWithConstructor( "mcmc",         new Mcmc()              );
        addTypeWithConstructor( "mmultinomial", new Move_mmultinomial() );
        addTypeWithConstructor( "mslide",       new Move_mslide()       );
        addTypeWithConstructor( "mscale",       new Move_mscale()       );
        addTypeWithConstructor( "msimplex",     new Move_msimplex()     );

        /* Add distributions with distribution constructors and distribution functions*/
        addDistribution( "dirichlet",   new Dist_dirichlet()   );
        addDistribution( "exp",         new Dist_exp()         );
        addDistribution( "multinomial", new Dist_multinomial() );
        addDistribution( "norm",        new Dist_norm()        );
        addDistribution( "unif",        new Dist_unif()        );

        /* Add basic internal functions */
        addFunction( "_range",    new Func__range()       );
       
        /* Add basic arithmetic/logic templated functions */
        addFunction( "_add",      new Func__add<            Integer,        Integer,    Integer >() );
        addFunction( "_add",      new Func__add<               Real,           Real,       Real >() );
        addFunction( "_add",      new Func__add<            Integer,           Real,       Real >() );
        addFunction( "_add",      new Func__add<               Real,        Integer,       Real >() );
        addFunction( "_add",      new Func__add<         MatrixReal,     MatrixReal, MatrixReal >() );
        addFunction( "_div",      new Func__div<            Integer,        Integer,       Real >() );
        addFunction( "_div",      new Func__div<               Real,           Real,       Real >() );
        addFunction( "_div",      new Func__div<            Integer,           Real,       Real >() );
        addFunction( "_div",      new Func__div<               Real,        Integer,       Real >() );
        addFunction( "_div",      new Func__div<         MatrixReal,     MatrixReal, MatrixReal >() );
        addFunction( "_mul",      new Func__mul<            Integer,        Integer,    Integer >() );
        addFunction( "_mul",      new Func__mul<               Real,           Real,       Real >() );
        addFunction( "_mul",      new Func__mul<            Integer,           Real,       Real >() );
        addFunction( "_mul",      new Func__mul<               Real,        Integer,       Real >() );
        addFunction( "_mul",      new Func__mul<         MatrixReal,     MatrixReal, MatrixReal >() );
        addFunction( "_mul",      new Func__mul<         MatrixReal,           Real, MatrixReal >() );
        addFunction( "_mul",      new Func__mul<               Real,     MatrixReal, MatrixReal >() );
#if 0
        addFunction( "_mul",      new Func__mul<          VectorReal,    VectorReal, MatrixReal >() );
        addFunction( "_mul",      new Func__mul<          VectorReal,    MatrixReal, MatrixReal >() );
        addFunction( "_mul",      new Func__mul<          MatrixReal,    VectorReal, MatrixReal >() );
        addFunction( "_mul",      new Func__mul<       VectorRealPos,    MatrixReal, MatrixReal >() );
        addFunction( "_mul",      new Func__mul<          MatrixReal, VectorRealPos, MatrixReal >() );
        addFunction( "_mul",      new Func__mul<       VectorRealPos, VectorRealPos, MatrixReal >() );
#endif
        addFunction( "_sub",      new Func__sub<            Integer,        Integer,    Integer >() );
        addFunction( "_sub",      new Func__sub<               Real,           Real,       Real >() );
        addFunction( "_sub",      new Func__sub<            Integer,           Real,       Real >() );
        addFunction( "_sub",      new Func__sub<               Real,        Integer,       Real >() );
        addFunction( "_sub",      new Func__sub<         MatrixReal,     MatrixReal, MatrixReal >() );
        addFunction( "_and",      new Func__and<            Integer,        Integer >()             );
        addFunction( "_and",      new Func__and<               Real,           Real >()             );
        addFunction( "_and",      new Func__and<            Integer,           Real >()             );
        addFunction( "_and",      new Func__and<               Real,        Integer >()             );
        addFunction( "_eq",       new Func__eq<             Integer,        Integer >()             );
        addFunction( "_eq",       new Func__eq<                Real,           Real >()             );
        addFunction( "_eq",       new Func__eq<             Integer,           Real >()             );
        addFunction( "_eq",       new Func__eq<                Real,        Integer >()             );
        addFunction( "_ge",       new Func__ge<             Integer,        Integer >()             );
        addFunction( "_ge",       new Func__ge<                Real,           Real >()             );
        addFunction( "_ge",       new Func__ge<             Integer,           Real >()             );
        addFunction( "_ge",       new Func__ge<                Real,        Integer >()             );
        addFunction( "_gt",       new Func__gt<             Integer,        Integer >()             );
        addFunction( "_gt",       new Func__gt<                Real,           Real >()             );
        addFunction( "_gt",       new Func__gt<             Integer,           Real >()             );
        addFunction( "_gt",       new Func__gt<                Real,        Integer >()             );
        addFunction( "_lt",       new Func__lt<             Integer,        Integer >()             );
        addFunction( "_lt",       new Func__lt<                Real,           Real >()             );
        addFunction( "_lt",       new Func__lt<             Integer,           Real >()             );
        addFunction( "_lt",       new Func__lt<                Real,        Integer >()             );
        addFunction( "_le",       new Func__le<             Integer,        Integer >()             );
        addFunction( "_le",       new Func__le<                Real,           Real >()             );
        addFunction( "_le",       new Func__le<             Integer,           Real >()             );
        addFunction( "_le",       new Func__le<                Real,        Integer >()             );
        addFunction( "_ne",       new Func__ne<             Integer,        Integer >()             );
        addFunction( "_ne",       new Func__ne<                Real,           Real >()             );
        addFunction( "_ne",       new Func__ne<             Integer,           Real >()             );
        addFunction( "_ne",       new Func__ne<                Real,        Integer >()             );
        addFunction( "_or",       new Func__or<             Integer,        Integer >()             );
        addFunction( "_or",       new Func__or<                Real,           Real >()             );
        addFunction( "_or",       new Func__or<             Integer,           Real >()             );
        addFunction( "_or",       new Func__or<                Real,        Integer >()             );
        addFunction( "transpose", new Func_transpose<    MatrixReal,     MatrixReal >()             );
#if 0
        addFunction( "transpose", new Func_transpose< VectorInteger,  VectorInteger >()             );
        addFunction( "transpose", new Func_transpose<    VectorReal,     VectorReal >()             );
        addFunction( "transpose", new Func_transpose< VectorRealPos,  VectorRealPos >()             );
#endif

        /* Add regular functions */
        addFunction( "clamp",     new Func_clamp()        ); 
        addFunction( "ls",        new Func_ls()           );
        addFunction( "model",     new Func_model()        );
        addFunction( "normalize", new Func_normalize()    );
        addFunction( "q",         new Func_quit()         );
        addFunction( "quit",      new Func_quit()         );
        addFunction( "source",    new Func_source()       );
        addFunction( "sqrt",      new Func_sqrt()         );
        addFunction( "v",         new Func_v_int()        );
        addFunction( "v",         new Func_v_double()     );
        addFunction( "v",         new Func_v_vecrealvec() );
        addFunction( "s",         new Func_s_doublevec()  );
        addFunction( "s",         new Func_s_int()        );
        addFunction( "s",         new Func_s_realvec()    );
        }
    catch(RbException& rbException) 
        {
        RBOUT("Caught an exception while initializing the workspace\n");
        std::ostringstream msg;
        rbException.printValue(msg);
        msg << std::endl;
        RBOUT(msg.str());

        RBOUT("Please report this bug to the RevBayes Development Core Team");

        RBOUT("Press any character to exit the program.");
        getchar();
        exit(0);
        }
}


/** Type checking using type table and full type spec */
bool Workspace::isXOfTypeY( const TypeSpec& xTypeSp, const TypeSpec& yTypeSp ) const {

    if ( xTypeSp.getDim() != yTypeSp.getDim() )
        return false;

    return isXOfTypeY( xTypeSp.getType(), yTypeSp.getType() );
}


/** Type checking using type table and type names, assuming same dim */
bool Workspace::isXOfTypeY( const std::string& xType, const std::string& yType ) const {

    /* Simplest case first */
    if ( xType == yType )
        return true;

    if ( typeTable.find( xType ) == typeTable.end() ) {
        if ( parentFrame == NULL )
            throw RbException( "Unknown type named '" + xType + "'" );
        else
            return ( (Workspace*)(parentFrame) )->isXOfTypeY( xType, yType );
    }

    const VectorString& xTypeVec = typeTable.find( xType )->second->getClass();
    size_t i;
    for ( i = 0; i < xTypeVec.size(); i++) {
        if ( xTypeVec[i] == yType )
            break;
    }

    if ( i == xTypeVec.size() )
        return false;
    else
        return true;
}


/** Is type x convertible to type y using type names, assuming dim = 0 */
bool Workspace::isXConvertibleToY( const std::string& xType, const std::string& yType ) const {

    TypeSpec    xTypeSpec   = TypeSpec( xType, 0 );
    TypeSpec    yTypeSpec   = TypeSpec( yType, 0 );

    return isXConvertibleToY( xTypeSpec, yTypeSpec );
}


/** Is type x convertible to type y? */
bool Workspace::isXConvertibleToY( const TypeSpec& xTypeSp, const TypeSpec& yTypeSp ) const {

    const std::string&  xType   = xTypeSp.getType();
    const std::string&  yType   = yTypeSp.getType();
    int                 xDim    = xTypeSp.getDim();
    int                 yDim    = yTypeSp.getDim();

    bool retVal = false;
    if ( xDim > 0 ) {
    
        VariableContainer* dummy    = new VariableContainer( xType );
        ValueContainer*    dummyVal = dummy->getConstValue();

        retVal = dummy->isConvertibleTo( yType, yDim, false );

        delete dummyVal;
        delete dummy;
    }
    else if (xDim == 0) {

        TypeTable::const_iterator i = typeTable.find( xType );
        if ( i == typeTable.end() )
            throw RbException("Unknown type named '" + xType + "'");
        
        retVal = (*i).second->isConvertibleTo( yType, yDim, false );
    }

    return retVal;
}


/** Print workspace */
void Workspace::printValue(std::ostream& o) const {

    o << "Variable table:" << std::endl;
    Frame::printValue(o);
    o << std::endl;

    o << "Function table:" << std::endl;
    functionTable->printValue(o);
    o << std::endl;

    o << "Type table:" << std::endl;
    std::map<std::string, RbObject*>::const_iterator i;
    for (i=typeTable.begin(); i!=typeTable.end(); i++) {
        o << (*i).first << " = " << (*i).second->getClass() << std::endl;
    }
}


