//
//  GeographyRateModifier.cpp
//  rb_mlandis
//
//  Created by Michael Landis on 8/8/13.
//  Copyright (c) 2013 Michael Landis. All rights reserved.
//

#include <iomanip>
#include <cmath>
#include "CharacterEvent.h"
#include "GeographicArea.h"
#include "GeographyRateModifier.h"
#include "RbConstants.h"
#include "TimeAtlas.h"
#define EARTHRADIUSKM 6371

using namespace RevBayesCore;

GeographyRateModifier::GeographyRateModifier(const TimeAtlas* ta, bool uadj, bool uav, bool udd, int idx, double dp, double th, std::string dt )

{
    // initialize
    atlas = ta;
    index = idx;
    distancePower = dp;
    threshhold = th;
    distanceType = dt;
    
    useAreaAdjacency = uadj;
    useAreaAvailable = uav;
    useDistanceDependence = udd;

    // get geography dimensions
    const std::vector<std::vector<GeographicArea*> > tmpAreas = atlas->getAreas();
    numAreas = 0;
    epochs = atlas->getEpochs();
    numEpochs = (unsigned)epochs.size();
    if (numEpochs > 0)
        numAreas = (unsigned)tmpAreas[0].size();
    
    epochOffset = numAreas * numAreas;
    areaOffset = numAreas;
    
    // assign areas for timeslice
    for (size_t i = 0; i < numEpochs; i++)
    {
        for (size_t j = 0; j < numAreas; j++)
        {
            GeographicArea* g = new GeographicArea( *tmpAreas[i][j] );
            areas.push_back(g);
            
            geographicCoordinates.push_back(g->getLatitude());
            geographicCoordinates.push_back(g->getLongitude());
        }
    }
    
    // initialize adjacencies between areas
    std::set<size_t> tmpSet;
    for (size_t i = 0; i < numAreas; i++)
        tmpSet.insert(i);
    
    availableAreaVector.resize(numEpochs * numAreas, 1.0);
    adjacentAreaVector.resize(numEpochs * numAreas * numAreas, 1.0);
    availableAreaSet.resize(numEpochs, tmpSet);
    adjacentAreaSet.resize(numEpochs*numAreas, tmpSet);

    if (useAreaAdjacency || useAreaAvailable)
       initializeAdjacentAreas();
    
    // initialize distance dependence
    geographicDistances.resize(numEpochs * numAreas * numAreas, 0.0);
    geographicDistancePowers.resize(numEpochs * numAreas * numAreas, 1.0);
    if (useDistanceDependence)
        initializeDistances();
    
    // unimplemented, but helps for computations for large N
    //computeAllPairwiseDistanceOrder();
    
    setDistancePower(dp, true);
    update();
}

GeographyRateModifier::GeographyRateModifier(const GeographyRateModifier& g)
{
    
    if (&g != this)
    {
        atlas = g.atlas;
        index = g.index;
        epochs = g.epochs;
        areas = g.areas;
        
        distanceType = g.distanceType;
        geographicCoordinates = g.geographicCoordinates;
        geographicDistances = g.geographicDistances;
        geographicDistancePowers = g.geographicDistancePowers;
        geographicDistanceOrder = g.geographicDistanceOrder;
        
        useAreaAvailable = g.useAreaAvailable;
        useAreaAdjacency = g.useAreaAdjacency;
        useDistanceDependence = g.useDistanceDependence;
        
//        dispersalValues = g.dispersalValues;
//        extinctionValues = g.extinctionValues;
//        adjacentAreaSet.resize(g.adjacentAreaSet.size());
//        availableAreaSet.resize(g.availableAreaSet.size());
//        
//        for (size_t i = 0; i < adjacentAreaSet.size(); i++)
//        {
//            std::cout << i << " " << g.adjacentAreaSet[i].size() << "\n";
//            adjacentAreaSet[i] = g.adjacentAreaSet[i];
//            for (std::set<size_t>::iterator it = adjacentAreaSet[i].begin(); it != adjacentAreaSet[i].end(); it++)
//                std::cout << " " << *it;
//            std::cout << "\n";
//        }
//        
//        for (size_t i = 0; i < availableAreaSet.size(); i++)
//            availableAreaSet[i] = g.availableAreaSet[i];
        
        adjacentAreaSet = g.adjacentAreaSet;
        availableAreaSet = g.availableAreaSet;
        
        adjacentAreaVector = g.adjacentAreaVector;
        availableAreaVector = g.availableAreaVector;
        
        numAreas = g.numAreas;
        numEpochs = g.numEpochs;
        areaOffset = g.areaOffset;
        epochOffset = g.epochOffset;
        threshhold = g.threshhold;
        distancePower = g.distancePower;
        
        setDistancePower(1.0, true);
        update();
    }
}

double GeographyRateModifier::computeRateModifier(std::vector<CharacterEvent *> currState, CharacterEvent* newState, double age)
{
    unsigned epochIdx = getEpochIndex(age);
    
    double rate = 0.0;
    double sum = 0.0;
    double r = 0.0;
    
    double eps = 1e-4;
    
    unsigned s = newState->getState();
    size_t charIdx = newState->getIndex();
    bool areaAvailable = availableAreaVector[epochIdx*numAreas + charIdx] > 0.0;

    // area exists and is lost
    if (areaAvailable && s == 0)
    {
        r = 1.0;

//        if (false)
        if (useAreaAdjacency) // || useDistanceDependence)
        {
            // determine which areas are present and which are absent
            present.clear();
            absent.clear();
            
            // which areas are in the range?
            std::set<size_t>::iterator it;
            for (it = availableAreaSet[epochIdx].begin(); it != availableAreaSet[epochIdx].end(); it++)
            {
                if (currState[*it]->getState() == 1)
                {
                    present.insert(currState[*it]);
                }
            }
            
            // impossible state?
            if (present.size() == 0)
                return 10e6;
            
            std::set<CharacterEvent*>::iterator it_p1;
            std::set<CharacterEvent*>::iterator it_p2;
            
            // focal present area
            double sum = 0.0;
            for (it_p1 = present.begin(); it_p1 != present.end(); it_p1++)
            {
                size_t idx_p1 = (*it_p1)->getIndex();
                double d = 1.0;
//                std::cout << idx_p1 << " " <<  d << "\n";
                
                // adjacent present areas
                double n_adj = 0;
                for (it_p2 = present.begin(); it_p2 != present.end(); it_p2++)
                {
                    size_t idx_p2 = (*it_p2)->getIndex();
                    size_t idx_e = epochIdx * epochOffset + idx_p1 * areaOffset + idx_p2;
                    
                    double v = adjacentAreaVector[idx_e];
                    n_adj += v;
                    
                    // extinction depends on distance to adjacent areas?
                    if (useDistanceDependence && v != 0.0)
                    {
                        v *= geographicDistancePowers[idx_e];
                    }
//                    d += v;
                }

                d /= n_adj;
                sum += d;
//                std::cout << d << " " << n_adj << " " << sum << "\n";                
                if (idx_p1 == charIdx)
                    rate = d;

            }
         
            
//            for (it = availableAreaSet[epochIdx].begin(); it != availableAreaSet[epochIdx].end(); it++)
//            {
//                std::cout << currState[*it]->getState();
//                if (*it % 5 == 4)
//                    std::cout << "\n";
//            }
//            
//            std::cout << rate << " " << sum << " " << present.size() << "\n";
            size_t n_p = present.size();
            
//            r = (rate / sum) * n_p;
            r = rate / (sum / n_p);
//            std::cout << "\n";
        }
    }
    
    // area does not exist and is lostsource("/Users/mlandis/Documents/code/revbayes-code/tutorials/RB_Biogeography_tutorial/RB_biogeo_files/scripts/biogeography_inf.Rev")
    if (!areaAvailable && s == 0)
    {
        r = 1e10;
    }
    
    // area does not exist and is gained
    else if (!areaAvailable && s == 1)
    {
        r = 0.0;
    }

    // area exists and is gained
    if (areaAvailable && s == 1)
    {
        // determine which areas are present and which are absent
        present.clear();
        absent.clear();
        std::set<size_t>::iterator it;
        
        if (!useAreaAdjacency)
        {
            for (it = availableAreaSet[epochIdx].begin(); it != availableAreaSet[epochIdx].end(); it++)
            {
                if (currState[*it]->getState() == 0)
                {
                    absent.insert(currState[*it]);
                }
                else
                {
                   present.insert(currState[*it]);
                }
            }
        }
        else if (useAreaAdjacency)
        {
            for (it = availableAreaSet[epochIdx].begin(); it != availableAreaSet[epochIdx].end(); it++)
            {
                if (currState[*it]->getState() == 1)
                {
                    std::set<size_t>::const_iterator it_adj;
                    const std::set<size_t> adj = adjacentAreaSet[epochIdx*numAreas + *it];
                    int n = 0;
                    for (it_adj = adj.begin(); it_adj != adj.end(); it_adj++)
                    {
                        if (currState[*it_adj]->getState() == 0)
                        {
                            absent.insert(currState[*it_adj]);
                            n++;
                        }
                    }
                    if (n > 0)
                        present.insert(currState[*it]);
                }
            }
        }
        
        // in case forbid extinction is enabled
        if (present.size() == 0)
            return 1.0;
        
        std::set<CharacterEvent*>::iterator it_p;
        std::set<CharacterEvent*>::iterator it_a;
        size_t n = 0;
        for (it_a = absent.begin(); it_a != absent.end(); it_a++)
        {
            size_t idx_a = (*it_a)->getIndex();
            double d = 0.0;
            for (it_p = present.begin(); it_p != present.end(); it_p++)
            {
                size_t idx_p = (*it_p)->getIndex();
                size_t idx_e = epochIdx * epochOffset + idx_p * areaOffset + idx_a;
                
                double v = adjacentAreaVector[idx_e];
                if (useAreaAdjacency && v > eps)
                    n += 1;
                
                if (useDistanceDependence && v != 0.0)
                {
                    v *= geographicDistancePowers[idx_e];
                }

                d += v;
            }

            sum += d;
            if (idx_a == charIdx)
                rate += d;
        }
//        if (!useAreaAdjacency)
//            n = absent.size();

        double w = 1.0;
        if (useAreaAdjacency)
            w = double(present.size()) / absent.size();
        
        
        r = w * rate / (sum/absent.size());
//        std::cout << "n = " << n << "\n";
//        std::cout << "n0 = " << absent.size() << " n1 = " << present.size() << "\n";
//        std::cout << r << " = " << w << " * " << rate << " / ( " << sum << " / " << absent.size() << " )\n";
//        r = absent.size() * rate / sum;
    }

    return r;
}

double GeographyRateModifier::computeRateModifier(const TopologyNode& node, std::vector<CharacterEvent *> currState, CharacterEvent* newState, double age)
{
    return computeRateModifier(currState, newState, age);
}

double GeographyRateModifier::computeRateModifier(std::vector<CharacterEvent *> currState, CharacterEvent* newState)
{
    return computeRateModifier(currState, newState, 0.0);
}

double GeographyRateModifier::computeSiteRateModifier(const TopologyNode& node, CharacterEvent* currState, CharacterEvent* newState, double age)
{
    unsigned s = newState->getState();
    size_t charIdx = newState->getIndex();
    unsigned epochIdx = getEpochIndex(age);
    
    // r == 1 if available, r == 0 if unavailable
    double r = availableAreaVector[ epochIdx*numAreas + charIdx ];

    // force extinction if unavailable and present
    if (r == 0.0 && s == 0)
    {
        r = 1e10;
    }
    else if (r == 0.0 && s == 1)
    {
        r = 1e-10;
    }
    
    return r;
}

double GeographyRateModifier::computeSiteRateModifier(const TopologyNode& node, unsigned from, unsigned to, unsigned charIdx, double age)
{
    unsigned epochIdx = getEpochIndex(age);
    
    // r == 1 if available, r == 0 if unavailable
    double r = availableAreaVector[ epochIdx*numAreas + charIdx ];

    // force extinction if unavailable and present
    if (r == 0.0 && to == 0)
    {
      
        r = 1e10;
    }
    else if (r == 0.0 && to == 1)
    {
        
        r = 1e-10;
    }
    
    return r;
}


GeographyRateModifier* GeographyRateModifier::clone(void) const
{
    return new GeographyRateModifier(*this);
}



void GeographyRateModifier::update(void)
{
    if (useDistanceDependence)
    {
        for (size_t i = 0; i < numEpochs; i++)
        {
            unsigned iOffset = (unsigned)(i*epochOffset);
            for (size_t j = 0; j < numAreas; j++)
            {
                unsigned jOffset = (unsigned)(j*areaOffset);
                for (size_t k = 0; k < j; k++)
                {
                    //double d = exp(-distancePower * geographicDistances[ iOffset + jOffset + k ]); //, -distancePower);
                    
                    double d = 1.0;
                    if (distancePower != 0.0)
                        d = pow(geographicDistances[ iOffset + jOffset + k ], -distancePower);

                    geographicDistancePowers[ iOffset + jOffset + k ] = d;
                    geographicDistancePowers[ iOffset + k*areaOffset + j ] = d;
                }
            }
        }
    }
}

void GeographyRateModifier::initializeDistances(void)
{
    for (unsigned i = 0; i < numEpochs; i++)
    {
        for (unsigned j = 0; j < numAreas; j++)
        {
            for (unsigned k = j; k < numAreas; k++)
            {
                double d = computePairwiseDistances(i, j, k);
                geographicDistances[epochOffset*i + areaOffset*j + k] = d;
                geographicDistances[epochOffset*i + areaOffset*k + j] = d;
                if (j != k && d == 0.0)
                {
                    if (adjacentAreaVector[epochOffset*i + areaOffset*j + k] > 0 ||
                        adjacentAreaVector[epochOffset*i + areaOffset*k + j] > 0)
                    {
                        std::stringstream ss;
                        ss << "ERROR: Areas " << j << " and " << k << " have zero distance";
                        throw RbException(ss.str());
                    }
                }
            }
        }
    }
}

void GeographyRateModifier::initializeAdjacentAreas(void)
{
    availableAreaSet.clear();
    adjacentAreaSet.clear();
    adjacentAreaSet.resize(numEpochs*numAreas);
    availableAreaSet.resize(numEpochs);
    
    for (size_t i = 0; i < availableAreaSet.size(); i++)
        availableAreaSet[i].clear();
    
    for (unsigned i = 0; i < numEpochs; i++)
    {
        for (unsigned j = 0; j < numAreas; j++)
        {
            const std::vector<double>& dvs = areas[numAreas*i + j]->getDispersalValues();
            for (unsigned k = 0; k < numAreas; k++)
            {
                double d = 0.0;
                if (dvs.size() == numAreas)
                {
                    if (dvs[k] > 0.0)
                        d = dvs[k];
                }
                else
                {
                    d = 1.0;
                }
                
                adjacentAreaVector[epochOffset*i + areaOffset*j + k] = d;
                
                if (dvs[k] == 1.0)
                {
                    adjacentAreaSet[numAreas*i + j].insert(k);
                }
//                std::cout << epochOffset*i + areaOffset*j + k << " " << i << " " << j << " " << k << " " << d << "\n";
                
                if (j == k)
                {
                    availableAreaVector[numAreas*i + j] = d;
                    if (d > 0.0)
                        availableAreaSet[i].insert(k);
                }
            }
        }
    }
}


void GeographyRateModifier::setDistancePower(double dp, bool upd)
{
    bool changed = (distancePower != dp);
    if (upd && changed)
    {
        distancePower = dp;
        update();
    }
}

void GeographyRateModifier::setGeographicDistancePowers(const std::vector<double>& dp)
{
    geographicDistancePowers = dp;
}

const std::vector<double>& GeographyRateModifier::getGeographicDistancePowers(void) const
{
    return geographicDistancePowers;
}

unsigned GeographyRateModifier::getEpochIndex(double age)
{
    unsigned index = 0;
    while (age <= epochs[index] && index < numEpochs-1)
    {
        index++;
    };
    return index;
}

const std::vector<double>& GeographyRateModifier::getEpochs(void) const
{
    return epochs;
}

const std::vector<double>& GeographyRateModifier::getDispersalValues(void) const
{
    return dispersalValues;
}

const std::vector<double>& GeographyRateModifier::getExtinctionValues(void) const
{
    return extinctionValues;
}

const std::vector<double>& GeographyRateModifier::getAdjacentAreaVector(void) const
{
    return adjacentAreaVector;
}

const std::vector<double>& GeographyRateModifier::getAvailableAreaVector(void) const
{
    return availableAreaVector;
}

const std::vector<std::set<size_t> >& GeographyRateModifier::getAdjacentAreaSet() const
{
    return adjacentAreaSet;
}

const bool GeographyRateModifier::getUseAreaAdjacency(void) const
{
    return useAreaAdjacency;
}

const bool GeographyRateModifier::getUseAreaAvailable(void) const
{
    return useAreaAvailable;
}

unsigned GeographyRateModifier::getNumAvailableAreas(const TopologyNode& node, std::vector<CharacterEvent*> currState, double age)
{
    unsigned epochIdx = getEpochIndex(age);
    double eps = 1e-4;
    
//    unsigned s = newState->getState();
//    unsigned charIdx = newState->getIndex();
//    bool areaAvailable = availableAreaVector[epochIdx*numAreas + charIdx] > 0.0;
    
    present.clear();
    absent.clear();
       
    std::set<size_t>::iterator it;
    for (it = availableAreaSet[epochIdx].begin(); it != availableAreaSet[epochIdx].end(); it++)
    {
        if (currState[*it]->getState() == 0)
        {
            absent.insert(currState[*it]);
        }
        else
        {
            present.insert(currState[*it]);
        }
    }
    
    std::set<size_t> gainableAreas;
    std::set<CharacterEvent*>::iterator it_p;
    std::set<CharacterEvent*>::iterator it_a;
    for (it_a = absent.begin(); it_a != absent.end(); it_a++)
    {
        size_t idx_a = (*it_a)->getIndex();
        bool found = false;
        for (it_p = present.begin(); it_p != present.end(); it_p++)
        {
            size_t idx_p = (*it_p)->getIndex();
            size_t idx_e = epochIdx * epochOffset + idx_p * areaOffset + idx_a;
            
            double v = adjacentAreaVector[idx_e];
            if (useAreaAdjacency && v > eps)
            {
                found = true;
                gainableAreas.insert(idx_a);
                break;
            }
        }
    }
    return (unsigned)gainableAreas.size();
}

unsigned GeographyRateModifier::getNumEmigratableAreas(const TopologyNode& node, std::vector<CharacterEvent*> currState, double age)
{
    unsigned epochIdx = getEpochIndex(age);
    double eps = 1e-4;
    
    //    unsigned s = newState->getState();
    //    unsigned charIdx = newState->getIndex();
    //    bool areaAvailable = availableAreaVector[epochIdx*numAreas + charIdx] > 0.0;
    
    present.clear();
    absent.clear();
    
    std::set<size_t>::iterator it;
    for (it = availableAreaSet[epochIdx].begin(); it != availableAreaSet[epochIdx].end(); it++)
    {
        if (currState[*it]->getState() == 0)
        {
            absent.insert(currState[*it]);
        }
        else
        {
            present.insert(currState[*it]);
        }
    }
    
    std::set<size_t> emigratableAreas;
    std::set<CharacterEvent*>::iterator it_p;
    std::set<CharacterEvent*>::iterator it_a;
    for (it_p = present.begin(); it_p != present.end(); it_p++)
    {
        size_t idx_p = (*it_p)->getIndex();
        bool found = false;
        for (it_a = absent.begin(); it_a != absent.end(); it_a++)
        {
            size_t idx_a = (*it_a)->getIndex();
            size_t idx_e = epochIdx * epochOffset + idx_p * areaOffset + idx_a;
            
            double v = adjacentAreaVector[idx_e];
            if (useAreaAdjacency && v > eps)
            {
                found = true;
                emigratableAreas.insert(idx_p);
                break;
            }
        }
    }
    return (unsigned)emigratableAreas.size();
}

double GeographyRateModifier::computePairwiseDistances(int i, int j, int k)
{
    double d = 0.0;
    double rad = RbConstants::PI/180;
    
    unsigned idx0 = i*numAreas*2 + j*2;
    unsigned idx1 = i*numAreas*2 + k*2;
    
    double lat0 = geographicCoordinates[ idx0 + 0] * rad;
    double lat1 = geographicCoordinates[ idx1 + 0] * rad;
    double lon0 = geographicCoordinates[ idx0 + 1] * rad;
    double lon1 = geographicCoordinates[ idx1 + 1] * rad;

    if (distanceType == "haversine")
    {
        double dlat = lat0 - lat1;
        double dlon = lon0 - lon1;
        double sindlat = sin(dlat/2);
        double sindlon = sin(dlon/2);
        double a = sqrt(sindlat * sindlat + cos(lat0) * cos(lat1) * sindlon * sindlon);
        if (a < 0.9995)
            d = 2 * asin(a) * EARTHRADIUSKM;
        else
            d = 2 * asin(1.0) * EARTHRADIUSKM;
    }
    else if (distanceType == "euclidean")
    {
        double dlat = lat0 - lat1;
        double dlon = lon0 - lon1;
        d = pow(dlat*dlat+dlon*dlon, 0.5);
    }
    
    return d;
}

void GeographyRateModifier::setInboundDispersal(const std::vector<double> &v)
{
    std::vector<double> r = v;
    for (size_t i = 0; i < numEpochs; i++)
    {
        for (size_t j = 0; j < numAreas; j++)
        {
            // update diagonal with sum of incoming rates per epoch
            double sum = 0.0;
            for (size_t k = 0; k < numAreas; k++)
            {
                double d = v[ epochOffset*i + k*numAreas + j ];
                sum += d;
            }
            inboundDispersalValues[ epochOffset*i + j ] = (sum > 0.0 ? 1.0 : 0.0);
        }
    }
}

void GeographyRateModifier::print(std::vector<std::vector<double> > m)
{
//    std::vector<std::vector<double> >::iterator it0;
//    std::vector<double>::iterator it1;
//    
//    for (it0 = m.begin(); it0 != m.end(); it0++)
//    {
//        for (it1 = it0->begin(); it1 != it0->end(); it1++)
//        {
//            std::cout << *it1 << " ";
//        }
//        std::cout << "\n";
//    }
}

void GeographyRateModifier::printAll(void)
{
//    std::cout << "\ngc\n";
//    print(geographicCoordinates);
//        std::cout << "\ngd\n";
//    print(geographicDistances);
//        std::cout << "\ngdp\n";
//    print(geographicDistancePowers);
//    
}

std::ostream& RevBayesCore::operator<<(std::ostream& o, const GeographyRateModifier& x)
{
    o << "GeographyRateModifier";
    return o;
}
