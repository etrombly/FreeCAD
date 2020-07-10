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
#include <boost/optional.hpp>

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

void nest(TopoDS_Shape& stock, ClipperLib::Paths& parts){

}

TYPESYSTEM_SOURCE(Path::GeneticAlgo, Base::Persistence)

GeneticAlgo::GeneticAlgo()
{
}

GeneticAlgo::GeneticAlgo(const GeneticAlgo& otherGA)
{
    *this = otherGA;
}

GeneticAlgo::GeneticAlgo(const std::vector<TopoDS_Shape>& adam, const std::vector<std::string>& sources, const NestingConfig& config)
{
    this->config = config;
    Population population;
    for(unsigned int i=0; i<adam.size(); i++){
			float angle = floor(rand()*this->config.rotations)*(360/this->config.rotations);
            population.individuals.push_back(Individual{adam[i], angle, i, sources[i], 0});
	}

    /*
    while(this.population.length < config.populationSize){
        var mutant = this.mutate(this.population[0]);
        this.population.push(mutant);
    }
    */

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

Population GeneticAlgo::mutate(const Population& population){
    Population clone = population;
    for(unsigned int i=0; i<clone.individuals.size(); i++){
        if(rand() < 0.01*this->config.mutationRate){
            // swap current part with next part
            unsigned int j = i+1;
            
            if(j < clone.individuals.size()){
                auto temp = clone.individuals[i].placement;
                clone.individuals[i].placement = clone.individuals[j].placement;
                clone.individuals[j].placement = temp;
            }
        }
        
        if(rand() < 0.01*this->config.mutationRate){
            clone.individuals[i].rotation = floor(rand()*this->config.rotations)*(360/this->config.rotations);
        }
    }
    
    return clone;
}

struct compare
{
	Individual key;
	compare(Individual const &i): key(i) { }

	bool operator()(Individual const &i)
	{
		return (i.id == key.id);
	}
};

// single point crossover
std::vector<Population> GeneticAlgo::mate (const Population& male, const Population& female){
    auto cutpoint = round(std::min(std::max((double)rand(), 0.1), 0.9)*(male.individuals.size()-1));
    
    auto maleIndividuals = std::vector<Individual>(male.individuals.begin(), male.individuals.begin() + cutpoint);
    auto clone1 = Population{maleIndividuals};
    auto femaleIndividuals = std::vector<Individual>(female.individuals.begin(), female.individuals.begin() + cutpoint);
    auto clone2 = Population{femaleIndividuals};
    
    for(unsigned int i=0; i<female.individuals.size(); i++){
        if(std::none_of(clone1.individuals.begin(), clone1.individuals.end(),  compare(female.individuals[i]))){
            clone1.individuals.push_back(female.individuals[i]);
        }
    }
    
    for(unsigned int i=0; i<male.individuals.size(); i++){
        if(std::none_of(clone2.individuals.begin(), clone2.individuals.end(),  compare(male.individuals[i]))){
            clone2.individuals.push_back(male.individuals[i]);
        }
    }
    
   std::vector<Population> result;
   result.push_back(clone1);
   result.push_back(clone2);
   return result;
}

bool populationSort (Individual i,Individual j) { return (i.fitness<j.fitness); }

void GeneticAlgo::generation() {
    std::sort (this->population.individuals.begin(), this->population.individuals.end(), populationSort);

    Population newpopulation;

    newpopulation.individuals.push_back(this->population.individuals[0]);
    
    while(newpopulation.individuals.size() < this->population.individuals.size()){
        auto male = this->randomWeightedIndividual(boost::none);
        auto female = this->randomWeightedIndividual(male);
        
        // each mating produces two children
        //auto children = this->mate(male, female);
        
        // slightly mutate children
        //newpopulation.individuals.push_back(this->mutate(children[0]));
            
        if(newpopulation.individuals.size() < this->population.individuals.size()){
            //newpopulation.individuals.push_back(this->mutate(children[1]));
        }
    }
            
    this->population = newpopulation;
}

Individual GeneticAlgo::randomWeightedIndividual(boost::optional<const Individual&> exclude = boost::none){
    auto pop = this->population;

    if(exclude){
        const Individual& excluded = *exclude;
        //pop.individuals.erase(std::remove(pop.individuals.begin(), pop.individuals.end(), excluded), pop.individuals.end());
    }

    float random = rand();
    
    float lower = 0;
    float weight = 1/pop.individuals.size();
    float upper = weight;
    
    for(unsigned int i=0; i<pop.individuals.size(); i++){
        // if the random number falls between lower and upper bounds, select this individual
        if(random> lower && random < upper){
            return pop.individuals[i];
        }
        lower = upper;
        upper += 2*weight * ((pop.individuals.size()-i)/pop.individuals.size());
    }
		
    return pop.individuals[0];
}