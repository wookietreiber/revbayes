#include "ArgumentRule.h"
#include "ArgumentRules.h"
#include "Dist_exponentialOffset.h"
#include "ExponentialWithOffsetDistribution.h"
#include "Real.h"
#include "RealPos.h"
#include "ContinuousStochasticNode.h"

using namespace RevLanguage;

Dist_exponentialOffset::Dist_exponentialOffset() : ContinuousDistribution() {
    
}


Dist_exponentialOffset::~Dist_exponentialOffset() {
    
}



Dist_exponentialOffset* Dist_exponentialOffset::clone( void ) const {
    return new Dist_exponentialOffset(*this);
}


RevBayesCore::ExponentialWithOffsetDistribution* Dist_exponentialOffset::createDistribution( void ) const {
    // get the parameters
    RevBayesCore::TypedDagNode<double>* l     = static_cast<const RealPos &>( lambda->getRevObject() ).getDagNode();
    RevBayesCore::TypedDagNode<double>* o     = static_cast<const Real    &>( offset->getRevObject() ).getDagNode();
    RevBayesCore::ExponentialWithOffsetDistribution* d  = new RevBayesCore::ExponentialWithOffsetDistribution( l, o );
    
    return d;
}



/* Get Rev type of object */
const std::string& Dist_exponentialOffset::getClassType(void) { 
    
    static std::string revType = "Dist_exponentialOffset";
    
	return revType; 
}

/* Get class type spec describing type of object */
const TypeSpec& Dist_exponentialOffset::getClassTypeSpec(void) { 
    
    static TypeSpec revTypeSpec = TypeSpec( getClassType(), new TypeSpec( ContinuousDistribution::getClassTypeSpec() ) );
    
	return revTypeSpec; 
}




/** Return member rules (no members) */
const MemberRules& Dist_exponentialOffset::getParameterRules(void) const {
    
    static MemberRules distExpMemberRules;
    static bool rulesSet = false;
    
    if ( !rulesSet ) {
        distExpMemberRules.push_back( new ArgumentRule( "lambda", RealPos::getClassTypeSpec(), ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
        distExpMemberRules.push_back( new ArgumentRule( "offset", Real::getClassTypeSpec()   , ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
        
        rulesSet = true;
    }
    
    return distExpMemberRules;
}


const TypeSpec& Dist_exponentialOffset::getTypeSpec( void ) const {
    
    static TypeSpec ts = getClassTypeSpec();
    
    return ts;
}


/** Print value for user */
void Dist_exponentialOffset::printValue(std::ostream& o) const {
    
    o << " exponential(lambda=";
    if ( lambda != NULL ) {
        o << lambda->getName();
    } else {
        o << "?";
    }
    o << ", offset=";
    if ( offset != NULL ) {
        o << offset->getName();
    } else {
        o << "?";
    }
    o << ")";
}


/** Set a member variable */
void Dist_exponentialOffset::setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var) {
    
    if ( name == "lambda" ) 
    {
        lambda = var;
    }
    else if ( name == "offset" ) 
    {
        offset = var;
    }
    else 
    {
        Distribution::setConstParameter(name, var);
    }
}
