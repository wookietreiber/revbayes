/* 
 * File:   DecomposedInverseWishartDistribution.cpp
 * Author: nl
 * 
 * Created on December 2, 2014, 10:08
 */

#include "DecomposedInverseWishartDistribution.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RbConstants.h"
#include "RbStatisticsHelper.h"
#include "DistributionNormal.h"
#include "DistributionInverseWishart.h"

using namespace RevBayesCore;

DecomposedInverseWishartDistribution::DecomposedInverseWishartDistribution(const TypedDagNode<MatrixReal> *insigma0, const TypedDagNode<int>* indf)  :
TypedDistribution<RevBayesCore::MatrixReal>(new MatrixReal(insigma0->getValue().getDim())),
sigma0(insigma0),
kappaVector(NULL),
kappa(NULL),
df(indf),
dim( NULL )  {
    
    // add the parameters to our set (in the base class)
    // in that way other class can easily access the set of our parameters
    // this will also ensure that the parameters are not getting deleted before we do
    addParameter( insigma0 );
    addParameter( df );
    
    redrawValue();
}

DecomposedInverseWishartDistribution::DecomposedInverseWishartDistribution(const TypedDagNode<RbVector<double> > *inkappaVector, const TypedDagNode<int>* indf)  :
TypedDistribution<RevBayesCore::MatrixReal>(new MatrixReal( inkappaVector->getValue().size()) ),
    sigma0(NULL),
    kappaVector(inkappaVector),
    kappa(NULL),
    df(indf),
    dim( NULL ) {
    
    // add the parameters to our set (in the base class)
    // in that way other class can easily access the set of our parameters
    // this will also ensure that the parameters are not getting deleted before we do
    addParameter( kappaVector );
    addParameter( df );
    
    redrawValue();
}

DecomposedInverseWishartDistribution::DecomposedInverseWishartDistribution(const TypedDagNode<int>* indim, const TypedDagNode<double> *inkappa, const TypedDagNode<int>* indf)  :
TypedDistribution<RevBayesCore::MatrixReal>(new MatrixReal( size_t(indim->getValue()) )),
    sigma0(NULL),
    kappaVector(NULL),
    kappa(inkappa),
    df(indf),
    dim(indim) {

    // add the parameters to our set (in the base class)
    // in that way other class can easily access the set of our parameters
    // this will also ensure that the parameters are not getting deleted before we do
    addParameter( inkappa );
    addParameter( indf );
    addParameter( indim );
    redrawValue();
}

DecomposedInverseWishartDistribution* DecomposedInverseWishartDistribution::clone(void) const   {

    return new DecomposedInverseWishartDistribution(*this);
}


void DecomposedInverseWishartDistribution::swapParameterInternal(const DagNode *oldP, const DagNode *newP) {

    if (oldP == sigma0)
        {
        sigma0 = static_cast<const TypedDagNode<MatrixReal>* >( newP );
        }
    if (oldP == kappaVector)
        {
        kappaVector = static_cast<const TypedDagNode<RbVector<double> >* >(newP);
        }
    if (oldP == kappa)
        {
        kappa = static_cast<const TypedDagNode<double>* >(newP);
        }
    if (oldP == dim)
        {
        dim = static_cast<const TypedDagNode<int>* >(newP);
        }
    if (oldP == df)
        {
        df = static_cast<const TypedDagNode<int>* >(newP);
        }
}


double DecomposedInverseWishartDistribution::computeLnProbability(void)  {
    
    double ret = 0.0;
    if (sigma0)
        {
        ret = RbStatistics::InverseWishart::lnPdf(sigma0->getValue(),df->getValue(),getValue());
        }
    else if (kappaVector)
        {
        ret = RbStatistics::InverseWishart::lnPdf(kappaVector->getValue(),df->getValue(),getValue());        
        }
    else if (kappa)
        {
        ret = RbStatistics::InverseWishart::lnPdf(kappa->getValue(),df->getValue(),getValue());        
        }
    else
        {
        std::cerr << "error in inverse wishart: no parameter\n";
        throw(0);
        }
    return ret;
}

void DecomposedInverseWishartDistribution::redrawValue(void)
{

    RandomNumberGenerator* rng = GLOBAL_RNG;
    if ( sigma0 != NULL )
        {
        setValue( RbStatistics::InverseWishart::rv(sigma0->getValue(),df->getValue(), *rng) );
        }
    else if ( kappaVector != NULL )
        {
        setValue( RbStatistics::InverseWishart::rv(kappaVector->getValue(),df->getValue(), *rng) );
        }
    else if ( kappa != NULL )
        {
        setValue( RbStatistics::InverseWishart::rv(kappa->getValue(),getValue().getDim(),df->getValue(), *rng) );
        }
    else
        {
        throw RbException("error in inverse wishart: no parameter\n");
        }

}
