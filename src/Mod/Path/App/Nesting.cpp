/***************************************************************************
 *   Copyright (c) Yorik van Havre (yorik@uncreated.net) 2014              *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"

#ifndef _PreComp_
# include <boost/regex.hpp>
#endif

#include <Base/Writer.h>
#include <Base/Reader.h>
#include <Base/Stream.h>
#include <Base/Exception.h>
#include <Base/Console.h>
# include <TopoDS_Shape.hxx>
#include <math.h>
#include <algorithm>

#include "Nesting.h"
#include <Mod/Path/App/PathSegmentWalker.h>

using namespace Path;
using namespace Base;

TYPESYSTEM_SOURCE(Path::Nesting, Base::Persistence)

Nesting::Nesting()
{
    NestingConfig config;
    config.clipperScale = 10000000;
    config.curveTolerance = 0.3;
    config.spacing = 0;
    config.rotations = 4;
    config.populationSize = 10;
    config.mutationRate = 10;
    //config.threads = 4;
    //config.PlacementType = 'gravity';
    config.mergeLines = true;
    config.timeRatio = 0.5;
    config.scale = 72;
    config.simplify = false;
    this->config = config;
}

Nesting::Nesting(const Nesting& otherNest)
{
    *this = otherNest;
}

Nesting::Nesting(const NestingConfig& config)
{
    this->config = config;
}

Nesting::~Nesting()
{
 
}

// Reimplemented from base class

unsigned int Nesting::getMemSize (void) const
{
    return 0;
}

void Nesting::Save (Writer &writer) const
{

}

void Nesting::Restore(XMLReader &reader)
{

}

TYPESYSTEM_SOURCE(Path::GeneticAlgo, Base::Persistence)

GeneticAlgo::GeneticAlgo()
{
}

GeneticAlgo::GeneticAlgo(const GeneticAlgo& otherGA)
{
    *this = otherGA;
}

GeneticAlgo::GeneticAlgo(const std::vector<TopoDS_Shape>& adam, const std::vector<std::string>& ids, const NestingConfig& config)
{
    this->config = config;
    std::vector<float> angles;
    for(unsigned int i=0; i<adam.size(); i++){
			float angle = floor(rand()*this->config.rotations)*(360/this->config.rotations);
			angles.push_back(angle);
	}
    std::vector<Population> population;
    population.push_back(Population{adam, angles, ids});
    this->population = population;
}

GeneticAlgo::~GeneticAlgo()
{
 
}

// Reimplemented from base class

unsigned int GeneticAlgo::getMemSize (void) const
{
    return 0;
}

void GeneticAlgo::Save (Writer &writer) const
{

}

void GeneticAlgo::Restore(XMLReader &reader)
{

}

Population GeneticAlgo::mutate(const Population& individual){
    Population clone = individual;
    for(unsigned int i=0; i<clone.placement.size(); i++){
        if(rand() < 0.01*this->config.mutationRate){
            // swap current part with next part
            unsigned int j = i+1;
            
            if(j < clone.placement.size()){
                auto temp = clone.placement[i];
                clone.placement[i] = clone.placement[j];
                clone.placement[j] = temp;
            }
        }
        
        if(rand() < 0.01*this->config.mutationRate){
            clone.angles[i] = floor(rand()*this->config.rotations)*(360/this->config.rotations);
        }
    }
    
    return clone;
}

struct compare
{
	std::string key;
	compare(std::string const &i): key(i) { }

	bool operator()(std::string const &i)
	{
		return (i == key);
	}
};

// single point crossover
std::vector<Population> GeneticAlgo::mate (const Population& male, const Population& female){
    auto cutpoint = round(std::min(std::max((double)rand(), 0.1), 0.9)*(male.placement.size()-1));
    
    auto gene1 = std::vector<TopoDS_Shape>(male.placement.begin(), male.placement.begin() + cutpoint);
    auto rot1 = std::vector<float>(male.angles.begin(), male.angles.begin() + cutpoint);
    auto ids1 = std::vector<std::string>(male.ids.begin(), male.ids.begin() + cutpoint);

    auto gene2 = std::vector<TopoDS_Shape>(female.placement.begin(), female.placement.begin() + cutpoint);
    auto rot2 = std::vector<float>(female.angles.begin(), female.angles.begin() + cutpoint);
    auto ids2 = std::vector<std::string>(female.ids.begin(), female.ids.begin() + cutpoint);
    
    for(unsigned int i=0; i<female.placement.size(); i++){
        if(std::none_of(ids1.begin(), ids1.end(),  compare(female.ids[i]))){
            gene1.push_back(female.placement[i]);
            rot1.push_back(female.angles[i]);
            ids1.push_back(female.ids[i]);
        }
    }
    
    for(unsigned int i=0; i<male.placement.size(); i++){
        if(std::none_of(ids2.begin(), ids2.end(),  compare(male.ids[i]))){
            gene2.push_back(male.placement[i]);
            rot2.push_back(male.angles[i]);
            ids2.push_back(male.ids[i]);
        }
    }
    
   std::vector<Population> result;
   result.push_back(Population{gene1, rot1, ids1});
   result.push_back(Population{gene2, rot2, ids2});
   return result;
}