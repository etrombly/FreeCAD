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


#ifndef PATH_Nesting_H
#define PATH_Nesting_H

#include "Command.h"
#include <Base/BoundBox.h>
#include <Base/Persistence.h>
#include <Base/Vector3D.h>
# include <TopoDS_Shape.hxx>

namespace Path
{

    /** Configuration values for the nesting algorithm */
    struct PathExport NestingConfig {
        int clipperScale;
        float curveTolerance;
        int spacing;
        int rotations;
        int populationSize;
        int mutationRate;
        // threads
        // placementType 'gravity'
        bool mergeLines;
        float timeRatio;
        int scale;
        bool simplify;
    };

    class PathExport Nesting : public Base::Persistence {
        TYPESYSTEM_HEADER();

        public:
            Nesting();
            Nesting(const Nesting&);
            Nesting(const NestingConfig&);
            ~Nesting();
            
            // from base class
            virtual unsigned int getMemSize (void) const;
            virtual void Save (Base::Writer &/*writer*/) const;
            virtual void Restore(Base::XMLReader &/*reader*/);
        
        protected:
            NestingConfig config;
            //std::vector<Part> parts;
            //std::vector<Part> partsTree;
            //std::vector<Placement> nests;
            //GeneticAlgo ga;
    };

    struct PathExport Population {
        std::vector<TopoDS_Shape> placement;
        std::vector<float> angles;
        std::vector<std::string> ids;
    };
    
    class PathExport GeneticAlgo : public Base::Persistence {
        TYPESYSTEM_HEADER();

        public:
            GeneticAlgo();
            GeneticAlgo(const GeneticAlgo&);
            GeneticAlgo(const std::vector<TopoDS_Shape>&, const std::vector<std::string>&, const NestingConfig&);
            ~GeneticAlgo();
            
            // from base class
            virtual unsigned int getMemSize (void) const;
            virtual void Save (Base::Writer &/*writer*/) const;
            virtual void Restore(Base::XMLReader &/*reader*/);

            // interface
            Population mutate(const Population&);
            std::vector<Population> mate (const Population&, const Population&);

        protected:
            std::vector<Population> population;
            NestingConfig config;
    };



} //namespace Nesting


#endif // PATH_Nesting_H
