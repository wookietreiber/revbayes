/**
 * @file
 * This file contains the implementation of RateMatrix, which is
 * class that holds a rate matrix in RevBayes.
 *
 * @brief Implementation of RateMatrix
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-08-27, version 1.0
 * @interface Mcmc
 * @package distributions
 *
 * $Id$
 */

#include "EigenSystem.h"
#include "Matrix.h"
#include "RateMatrix.h"
#include "RbException.h"
#include "RbMathMatrix.h"
#include "Simplex.h"
#include "TransitionProbabilityMatrix.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>


/** Construct rate matrix with n states */
RateMatrix::RateMatrix(size_t n) {

    numStates            = n;
    eigensDirty          = true;
    reversibilityChecked = false;
    timeReversible       = false;
    theRateMatrix        = new Matrix<double>(numStates, numStates, 0.0 );
    theStationaryFreqs   = std::vector<double>(numStates);
    theEigenSystem       = new EigenSystem(theRateMatrix);
    c_ijk.resize(numStates * numStates * numStates);
    cc_ijk.resize(numStates * numStates * numStates);
}


/** Copy constructor */
RateMatrix::RateMatrix(const RateMatrix& m) {

    numStates            = m.numStates;
    eigensDirty          = m.eigensDirty;
    reversibilityChecked = m.reversibilityChecked;
    timeReversible       = m.timeReversible;
    theRateMatrix        = new Matrix<double>( *m.theRateMatrix );
    theStationaryFreqs   = m.theStationaryFreqs;
    theEigenSystem       = new EigenSystem( *m.theEigenSystem );
    c_ijk                = m.c_ijk;
    cc_ijk               = m.cc_ijk;
    avgRate              = m.avgRate;
    
    theEigenSystem->setRateMatrixPtr(theRateMatrix);
}


/** Destructor */
RateMatrix::~RateMatrix(void) {
    
    delete theRateMatrix;
    delete theEigenSystem;
}


RateMatrix& RateMatrix::operator=(const RateMatrix &r) {
    
    if (this != &r) {
        
        delete theRateMatrix;
        delete theEigenSystem;
        
        numStates            = r.numStates;
        eigensDirty          = r.eigensDirty;
        reversibilityChecked = r.reversibilityChecked;
        timeReversible       = r.timeReversible;
        theRateMatrix        = new Matrix<double>( *r.theRateMatrix );
        theStationaryFreqs   = r.theStationaryFreqs;
        theEigenSystem       = new EigenSystem( *r.theEigenSystem );
        c_ijk                = r.c_ijk;
        cc_ijk               = r.cc_ijk;
        avgRate              = r.avgRate;
        
        theEigenSystem->setRateMatrixPtr(theRateMatrix);
    }
    
    return *this;
}

/** Index operator (const) */
const std::vector<double>& RateMatrix::operator[]( const size_t i ) const {

    if ( i >= numStates )
        throw RbException( "Index to RateMatrix[][] out of bounds" );
    return (*theRateMatrix)[i];
}


/** Index operator */
std::vector<double>& RateMatrix::operator[]( const size_t i ) {

    if ( i >= numStates )
        throw RbException( "Index to RateMatrix[][] out of bounds" );
    return (*theRateMatrix)[i];
}


/** Calculate the average rate for the rate matrix */
double RateMatrix::averageRate(void) const {

    double ave = 0.0;
    for (size_t i=0; i<numStates; i++)
        ave += -theStationaryFreqs[i] * (*theRateMatrix)[i][i];
    return ave;
}


/** Do precalculations on eigenvectors */
void RateMatrix::calculateCijk(void) {

    if ( theEigenSystem->isComplex() == false )
        {
        // real case
        const Matrix<double>& ev  = theEigenSystem->getEigenvectors();
        const Matrix<double>& iev = theEigenSystem->getInverseEigenvectors();
        double* pc = &c_ijk[0];
        for (size_t i=0; i<numStates; i++)
            for (size_t j=0; j<numStates; j++)
                for (size_t k=0; k<numStates; k++)
                    *(pc++) = ev[i][k] * iev[k][j];   
        }
    else
        {
        // complex case
        const Matrix<std::complex<double> >& cev  = theEigenSystem->getComplexEigenvectors();
        const Matrix<std::complex<double> >& ciev = theEigenSystem->getComplexInverseEigenvectors();
        std::complex<double>* pc = &cc_ijk[0];
        for (size_t i=0; i<numStates; i++)
            for (size_t j=0; j<numStates; j++)
                for (size_t k=0; k<numStates; k++)
                    *(pc++) = cev[i][k] * ciev[k][j];
        }
}


/** This function calculates the stationary frequencies of the rate matrix. The
    rate matrix, Q, is the infinitesimal generator of the Markov chain. It is an
    n X n matrix whose off-diagonal elements are q_ij >= 0 and whose diagonal elements
    are specified such that each row sums to zero. The rate matrix is finite (has
    a fixed number of states) and we assume that the input matrix is irreducible, as
    is the usual case for substitution models. Because Q is irreducible and finite,
    it has a stationary distribution, pi, which is a row vector of n probabilities.
    The stationary probabilities can be calculated by solving the homogeneous system
    of equations, pi*Q = 0, where 0 is a vector of zeros.
 
    We do the following to calculate the stationary frequencies. 
 
    1. We perform an LU decomposition of the transpose of the matrix Q.
 
       Q' = LU
 
    2. Now we set Ux = z (x will eventually hold the stationary probabilities). 
       Because L is nonsingular, we have z = 0. We proceed to back substitute on
       Ux = z = 0. When u_nn = 0, we can put in any solution for x. Here, we put
       in x_n = 1. We then solve the other values of x through back substitution.
 
    3. The solution obtained in 2 is not a probability vector. We normalize the
       vector such that the sum of the elements is 1.
 
    Note that the only time we need to use this function is when we don't
    know the stationary frequencies of the rate matrix beforehand. For most
    substitution models used in molecular evolution, the stationary frequencies
    are built into the rate matrix itself. These models are time-reversible.
    This function is useful for the non-reversible models.
 
    For more information on the fascinating topic of calculating the stationary
    probabilities of a rate matrix, see:
  
    Stewart, W. J. 1999. Numerical methods for computing stationary distributions of 
       finite irreducible Markov chains. In "Advances in Computational
       Probability", W. Grassmann, ed. Kluwer Academic Publishers. */
void RateMatrix::calculateStationaryFrequencies(void) {

	// transpose the rate matrix and put into QT
    Matrix<double> QT(numStates, numStates);
    for (size_t i=0; i<numStates; i++)
        for (size_t j=0; j<numStates; j++)
            QT[i][j] = (*theRateMatrix)[j][i];

	// compute the LU decomposition of the transposed rate matrix
    Matrix<double> L(numStates, numStates);
    Matrix<double> U(numStates, numStates);
	RbMath::computeLandU(QT, L, U);
	
	// back substitute into z = 0 to find un-normalized stationary frequencies, starting with x_n = 1.0
    std::vector<double> pi(numStates, 0.0);
	pi[numStates-1] = 1.0;
	for (int i= int(numStates-2); i >= 0; i--)
		{
		double dotProduct = 0.0;
		for (size_t j=i+1; j<numStates; j++)
			dotProduct += U[i][j] * pi[j];
		pi[i] = (0.0 - dotProduct) / U[i][i];
		}
		
	// normalize the solution vector
	double sum = 0.0;
	for (size_t i=0; i<numStates; i++)
		sum += pi[i];
	for (size_t i=0; i<numStates; i++)
		pi[i] /= sum;
        
    // set the stationary frequencies
    theStationaryFreqs = pi;
}


/** Calculate the transition probabilities */
void RateMatrix::calculateTransitionProbabilities(double t, TransitionProbabilityMatrix& P) const {

	if ( theEigenSystem->isComplex() == false )
		tiProbsEigens(t, P);
	else
		tiProbsComplexEigens(t, P);
}


/** This function checks that the rate matrix is time reversible. It takes as
    input the rate matrix, a, and the stationary frequencies of the process, f. 
    It checks that f[i] * q[i][j] = f[j] * q[j][i] for all i != j. It does this
    by accumulating the difference | f[i] * q[i][j] - f[j] * q[j][i] | for all
    off-diagonal comparisons. If this difference is less than tolerance,
    it reports that the rate matrix is time-reversible. If the flag isRev
    is set to true, then we do not need to check because then we have determined
    previously that the rate matrix is reversible. */ 
bool RateMatrix::checkTimeReversibity(double tolerance) {
	
	double diff = 0.0;
	for (size_t i=0; i<numStates; i++)
		for (size_t j=i+1; j<numStates; j++)
			diff += fabs( theStationaryFreqs[i] * (*theRateMatrix)[i][j] - theStationaryFreqs[j] * (*theRateMatrix)[j][i] );
    reversibilityChecked = true;
	if (diff < tolerance)
        return true;
	return false;
}


size_t RateMatrix::getNumberOfStates( void ) const {
    return numStates;
}


const std::vector<double>& RateMatrix::getStationaryFrequencies( void ) const {
    return theStationaryFreqs;
}


/** Return whether or not the rate matrix is time reversible */
bool RateMatrix::isTimeReversible(void) {

    if (reversibilityChecked == false)
        return checkTimeReversibity(0.000001);
    return timeReversible;
}


/** Rescale the rates such that the average rate is r */
void RateMatrix::rescaleToAverageRate(const double r) {

    double curAve = averageRate();
    double scaleFactor = r / curAve;
    for (size_t i=0; i<numStates; i++)
        for (size_t j=0; j<numStates; j++)
            (*theRateMatrix)[i][j] *= scaleFactor;
}


/** Set the diagonal of the rate matrix such that each row sums to zero */
void RateMatrix::setDiagonal(void) {

    for (size_t i=0; i<numStates; i++)
        {
        double sum = 0.0;
        for (size_t j=0; j<numStates; j++)
            {
            if (i != j)
                sum += (*theRateMatrix)[i][j];
            }
        (*theRateMatrix)[i][i] = -sum;
        }
}


/** Directly set whether or not the rate matrix is time reversible */
void RateMatrix::setTimeReversible(const bool tf) {

    reversibilityChecked = true;
    timeReversible = tf;
}


/** Set the stationary frequencies directly. We assume that we know
    what the stationary frequencies are when this function is called. We
    would know the stationary frequencies for most phylogenetic models. */
void RateMatrix::setStationaryFrequencies(const std::vector<double>& f) {

    theStationaryFreqs = f;
}


/** Calculate the transition probabilities for the real case */
void RateMatrix::tiProbsEigens(const double t, TransitionProbabilityMatrix& P) const {

    // get a reference to the eigenvalues
    const std::vector<double>& eigenValue = theEigenSystem->getRealEigenvalues();
    
    // precalculate the product of the eigenvalue and the branch length
    std::vector<double> eigValExp(numStates);
	for (size_t s=0; s<numStates; s++)
		eigValExp[s] = exp(eigenValue[s] * t);
        
    // calculate the transition probabilities
	const double* ptr = &c_ijk[0];
	for (size_t i=0; i<numStates; i++) 
		{
		for (size_t j=0; j<numStates; j++) 
			{
			double sum = 0.0;
			for(size_t s=0; s<numStates; s++)
				sum += (*ptr++) * eigValExp[s];
			P[i][j] = (sum < 0.0) ? 0.0 : sum;
			}
		}
}


/** Calculate the transition probabilities for the complex case */
void RateMatrix::tiProbsComplexEigens(const double t, TransitionProbabilityMatrix& P) const {

    // get a reference to the eigenvalues
    const std::vector<double>& eigenValueReal = theEigenSystem->getRealEigenvalues();
    const std::vector<double>& eigenValueComp = theEigenSystem->getImagEigenvalues();

    // precalculate the product of the eigenvalue and the branch length
    std::vector<std::complex<double> > ceigValExp(numStates);
	for (size_t s=0; s<numStates; s++)
        {
        std::complex<double> ev = std::complex<double>(eigenValueReal[s], eigenValueComp[s]);
		ceigValExp[s] = exp(ev * t);
        }

    // calculate the transition probabilities
	const std::complex<double>* ptr = &cc_ijk[0];
	for (size_t i=0; i<numStates; i++) 
		{
		for (size_t j=0; j<numStates; j++) 
			{
			std::complex<double> sum = std::complex<double>(0.0, 0.0);
			for(size_t s=0; s<numStates; s++)
				sum += (*ptr++) * ceigValExp[s];
			P[i][j] = (sum.real() < 0.0) ? 0.0 : sum.real();
			}
		}
}


/** Update the eigen system */
void RateMatrix::updateEigenSystem(void) {

    if (areEigensDirty == false)
        return;
    theEigenSystem->update();
    calculateCijk();
    eigensDirty = false;
}
