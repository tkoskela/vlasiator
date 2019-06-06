/*
 * This file is part of Vlasiator.
 * Copyright 2010-2016 Finnish Meteorological Institute
 *
 * For details of usage, see the COPYING file and read the "Rules of the Road"
 * at http://www.physics.helsinki.fi/vlasiator/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <cstdlib>
#include <iostream>

#include "datareducer.h"
#include "../common.h"
#include "dro_populations.h"
using namespace std;

void initializeDataReducers(DataReducer * outputReducer, DataReducer * diagnosticReducer)
{
   typedef Parameters P;

   vector<string>::const_iterator it;
   for (it = P::outputVariableList.begin();
        it != P::outputVariableList.end();
        it++) {
      if(*it == "fg_B" || *it == "B") { // Bulk magnetic field at Yee-Lattice locations
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_B",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]*3);

               // Iterate through fsgrid cells and extract total magnetic field
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x)] =     (*BgBGrid.get(x,y,z))[fsgrids::BGBX]
                           + (*perBGrid.get(x,y,z))[fsgrids::PERBX];
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x) + 1] = (*BgBGrid.get(x,y,z))[fsgrids::BGBY]
                           + (*perBGrid.get(x,y,z))[fsgrids::PERBY];
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x) + 2] = (*BgBGrid.get(x,y,z))[fsgrids::BGBZ]
                           + (*perBGrid.get(x,y,z))[fsgrids::PERBZ];
                     }
                  }
               }
               return retval;
         }
         ));
         continue;
      }
      if(*it == "fg_BackgroundB" || *it == "BackgroundB") { // Static (typically dipole) magnetic field part
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_background_B",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]*3);

               // Iterate through fsgrid cells and extract background B
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x)] =     (*BgBGrid.get(x,y,z))[fsgrids::BGBX];
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x) + 1] = (*BgBGrid.get(x,y,z))[fsgrids::BGBY];
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x) + 2] = (*BgBGrid.get(x,y,z))[fsgrids::BGBZ];
                     }
                  }
               }
               return retval;
         }
         ));
         continue;
      }
      if(*it == "fg_PerturbedB" || *it == "PerturbedB") { // Fluctuating magnetic field part
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_perturbed_B",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]*3);

               // Iterate through fsgrid cells and extract values
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x)] =     (*perBGrid.get(x,y,z))[fsgrids::PERBX];
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x) + 1] = (*perBGrid.get(x,y,z))[fsgrids::PERBY];
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x) + 2] = (*perBGrid.get(x,y,z))[fsgrids::PERBZ];
                     }
                  }
               }
               return retval;
         }
         ));
         continue;
      }
      if(*it == "fg_E" || *it== "E") { // Bulk electric field at Yee-lattice locations
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_E",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]*3);

               // Iterate through fsgrid cells and extract E values
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x)] =     (*EGrid.get(x,y,z))[fsgrids::EX];
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x) + 1] = (*EGrid.get(x,y,z))[fsgrids::EY];
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x) + 2] = (*EGrid.get(x,y,z))[fsgrids::EZ];
                     }
                  }
               }
               return retval;
         }
         ));
         continue;
      }
      if(*it == "vg_Rhom" || *it == "Rhom") { // Overall mass density (summed over all populations)
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("rhom",CellParams::RHOM,1));
         continue;
      }
      if(*it == "fg_Rhom") { // Overall mass density (summed over all populations)
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_rhom",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]);

               // Iterate through fsgrid cells and extract rho valuesg
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        retval[gridSize[1]*gridSize[0]*z + gridSize[0]*y + x] = (*momentsGrid.get(x,y,z))[fsgrids::RHOM];
                     }
                  }
               }
               return retval;
         }
         ));
         continue;
      }
      if(*it == "vg_Rhoq" || *it == "Rhoq") { // Overall charge density (summed over all populations)
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("rhoq",CellParams::RHOQ,1));
         continue;
      }
      if(*it == "fg_Rhoq") { // Overall charge density (summed over all populations)
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_rhoq",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]);

               // Iterate through fsgrid cells and extract charge density
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        retval[gridSize[1]*gridSize[0]*z + gridSize[0]*y + x] = (*momentsGrid.get(x,y,z))[fsgrids::RHOQ];
                     }
                  }
               }
               return retval;
         }
         ));
         continue;
      }
      if(*it == "populations_Rho") { // Per-population particle number density
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            species::Species& species=getObjectWrapper().particleSpecies[i];
            const std::string& pop = species.name;
            outputReducer->addOperator(new DRO::DataReductionOperatorPopulations<Real>(pop + "/rho", i, offsetof(spatial_cell::Population, RHO), 1));
         }
         continue;
      }
      
      if(*it == "V" || *it == "vg_V") { // Overall effective bulk density defining the center-of-mass frame from all populations
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("V",CellParams::VX,3));
         continue;
      }
      if(*it == "fg_V") { // Overall effective bulk density defining the center-of-mass frame from all populations
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_V",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]*3);

               // Iterate through fsgrid cells and extract bulk Velocity
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x)] =     (*momentsGrid.get(x,y,z))[fsgrids::VX];
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x) + 1] = (*momentsGrid.get(x,y,z))[fsgrids::VY];
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x) + 2] = (*momentsGrid.get(x,y,z))[fsgrids::VZ];
                     }
                  }
               }
               return retval;
         }
         ));
         continue;
      }
      if(*it == "populations_V") { // Per population bulk velocities
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            species::Species& species=getObjectWrapper().particleSpecies[i];
            const std::string& pop = species.name;
            outputReducer->addOperator(new DRO::DataReductionOperatorPopulations<Real>(pop + "/V", i, offsetof(spatial_cell::Population, V), 3));
         }
         continue;
      }
      if(*it == "populations_moments_Backstream") { // Per-population moments of the backstreaming part
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            outputReducer->addOperator(new DRO::VariableRhoBackstream(i));
            outputReducer->addOperator(new DRO::VariableVBackstream(i));
            outputReducer->addOperator(new DRO::VariablePTensorBackstreamDiagonal(i));
            outputReducer->addOperator(new DRO::VariablePTensorBackstreamOffDiagonal(i));
         }
         continue;
      }
      if(*it == "populations_moments_NonBackstream") { // Per-population moments of the non-backstreaming (thermal?) part.
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            outputReducer->addOperator(new DRO::VariableRhoNonBackstream(i));
            outputReducer->addOperator(new DRO::VariableVNonBackstream(i));
            outputReducer->addOperator(new DRO::VariablePTensorNonBackstreamDiagonal(i));
            outputReducer->addOperator(new DRO::VariablePTensorNonBackstreamOffDiagonal(i));
         }
         continue;
      }
      if(*it == "populations_MinValue" || *it == "populations_EffectiveSparsityThreshold") {
         // Effective sparsity threshold affecting each cell, if dynamic threshould algorithm is used
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            outputReducer->addOperator(new DRO::VariableEffectiveSparsityThreshold(i));
         }
         continue;
      }
      if(*it == "populations_RhoLossAdjust") {
         // Accumulated lost particle number, per population, in each cell, since last restart
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            species::Species& species=getObjectWrapper().particleSpecies[i];
            const std::string& pop = species.name;
            outputReducer->addOperator(new DRO::DataReductionOperatorPopulations<Real>(pop + "/rho_loss_adjust", i, offsetof(spatial_cell::Population, RHOLOSSADJUST), 1));
         }
         continue;
      }
      if(*it == "LBweight" || *it == "vg_LBweight") {
         // Load balance metric for LB debugging
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("LB_weight",CellParams::LBWEIGHTCOUNTER,1));
         continue;
      }
      if(*it == "MaxVdt") {
         // Overall maximum timestep constraint as calculated by the velocity space vlasov update
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("max_v_dt",CellParams::MAXVDT,1));
         continue;
      }
      if(*it == "populations_MaxVdt") {
         // Per-population maximum timestep constraint as calculated by the velocity space vlasov update
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            species::Species& species=getObjectWrapper().particleSpecies[i];
            const std::string& pop = species.name;
            outputReducer->addOperator(new DRO::DataReductionOperatorPopulations<Real>(pop + "/MaxVdt", i, offsetof(spatial_cell::Population, max_dt[1]), 1));
         }
         continue;
      }
      if(*it == "MaxRdt") {
         // Overall maximum timestep constraint as calculated by the real space vlasov update
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("max_r_dt",CellParams::MAXRDT,1));
         continue;
      }
      if(*it == "populations_MaxRdt") {
         // Per-population maximum timestep constraint as calculated by the real space vlasov update
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            species::Species& species=getObjectWrapper().particleSpecies[i];
            const std::string& pop = species.name;
            outputReducer->addOperator(new DRO::DataReductionOperatorPopulations<Real>(pop + "/MaxRdt", i, offsetof(spatial_cell::Population, max_dt[0]), 1));
         }
         continue;
      }
      if(*it == "populations_EnergyDensity") {
         // Per-population energy density in three energy ranges
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            outputReducer->addOperator(new DRO::VariableEnergyDensity(i));
         }
         continue;
      }
      if(*it == "populations_PrecipitationFlux") {
         // Per-population precipitation differential flux
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            outputReducer->addOperator(new DRO::VariablePrecipitationDiffFlux(i));
         }
         continue;
      }
      if(*it == "MaxFieldsdt" || *it == "fg_MaxFieldsdt") {
         // Maximum timestep constraint as calculated by the fieldsolver
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("MaxFieldsdt",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]);

               // Iterate through fsgrid cells and extract field solver timestep limit
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        retval[gridSize[1]*gridSize[0]*z + gridSize[0]*y + x] = technicalGrid.get(x,y,z)->maxFsDt;
                     }
                  }
               }
               return retval;
         }
         ));
         continue;
      }
      if(*it == "MPIrank" || *it == "vg_rank") {
         // Map of spatial decomposition of the DCCRG grid into MPI ranks
         outputReducer->addOperator(new DRO::MPIrank);
         continue;
      }
      if(*it == "FsGridRank" || *it == "fg_rank") {
         // Map of spatial decomposition of the FsGrid into MPI ranks
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("FsGridRank",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2],technicalGrid.getRank());
               return retval;
             }
         ));
         continue;
      }
      if(*it == "BoundaryType" || *it == "vg_BoundaryType") {
         // Type of boundarycells
         outputReducer->addOperator(new DRO::BoundaryType);
         continue;
      }
      if(*it == "fg_BoundaryType") {
         // Type of boundarycells as stored in FSGrid
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("FsGridBoundaryType",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]);

               // Iterate through fsgrid cells and extract boundary flag
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        retval[gridSize[1]*gridSize[0]*z + gridSize[0]*y + x] = technicalGrid.get(x,y,z)->sysBoundaryFlag;
                     }
                  }
               }
               return retval;
         }
         ));
         continue;
      }
      if(*it == "BoundaryLayer" || *it == "vg_BoundaryLayer") {
         // For boundaries with multiple layers: layer count per cell
         outputReducer->addOperator(new DRO::BoundaryLayer);
         continue;
      }
      if(*it == "fg_BoundaryLayer") {
         // Type of boundarycells as stored in FSGrid
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("FsGridBoundaryLayer",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]);

               // Iterate through fsgrid cells and extract boundary layer
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        retval[gridSize[1]*gridSize[0]*z + gridSize[0]*y + x] = technicalGrid.get(x,y,z)->sysBoundaryLayer;
                     }
                  }
               }
               return retval;
         }
         ));
         continue;
      }
      if (*it == "populations_Blocks") {
         // Per-population velocity space block counts
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            outputReducer->addOperator(new DRO::Blocks(i));
         }
         continue;
      }
      if(*it == "fSaved") {
         // Boolean marker whether a velocity space is saved in a given spatial cell
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("fSaved",CellParams::ISCELLSAVINGF,1));
         continue;
      }
      if(*it == "populations_accSubcycles") {
         // Per-population number of subcycles performed for velocity space update
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            species::Species& species=getObjectWrapper().particleSpecies[i];
            const std::string& pop = species.name;
            outputReducer->addOperator(new DRO::DataReductionOperatorPopulations<uint>(pop + "/acc_subcycles", i, offsetof(spatial_cell::Population, ACCSUBCYCLES), 1));
         }
         continue;
      }
      if(*it == "HallE" || *it == "fg_HallE") {
         for(int index=0; index<fsgrids::N_EHALL; index++) {
            std::string reducer_name = "fg_HallE" + std::to_string(index);
            outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid(reducer_name,[index](
                         FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                         FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                         FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                         FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                         FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                         FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                         FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                         FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                         FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                         FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

                  std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
                  std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]);

                  // Iterate through fsgrid cells and extract EHall
                  for(int z=0; z<gridSize[2]; z++) {
                     for(int y=0; y<gridSize[1]; y++) {
                        for(int x=0; x<gridSize[0]; x++) {
                           retval[gridSize[1]*gridSize[0]*z + gridSize[0]*y + x] =  + (*EHallGrid.get(x,y,z))[index];
                        }
                     }
                  }
                  return retval;
            }
            ));
         }
         continue;
      }
      if(*it =="GradPeE") {
         // Electron pressure gradient contribution to the generalized ohm's law
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("EGRADPE",CellParams::EXGRADPE,3));
         continue;
      }
      if(*it == "VolB" || *it == "vg_VolB") {
         // Volume-averaged magnetic field
         outputReducer->addOperator(new DRO::VariableBVol);
         continue;
      }
      if(*it == "fg_VolB") { // Static (typically dipole) magnetic field part
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_volB",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]*3);

               // Iterate through fsgrid cells and extract total BVOL
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x)] =     (*BgBGrid.get(x,y,z))[fsgrids::BGBXVOL]
                           + (*volGrid.get(x,y,z))[fsgrids::PERBXVOL];
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x) + 1] = (*BgBGrid.get(x,y,z))[fsgrids::BGBYVOL]
                           + (*volGrid.get(x,y,z))[fsgrids::PERBYVOL];
                        retval[3*(gridSize[1]*gridSize[0]*z + gridSize[0]*y + x) + 2] = (*BgBGrid.get(x,y,z))[fsgrids::BGBZVOL]
                           + (*volGrid.get(x,y,z))[fsgrids::PERBZVOL];
                     }
                  }
               }
               return retval;
         }
         ));
         continue;
      }
      if(*it == "BackgroundVolB") {
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("BGB_vol",CellParams::BGBXVOL,3));
         continue;
      }
      if(*it == "PerturbedVolB") {
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("PERB_vol",CellParams::PERBXVOL,3));
         continue;
      }
      if(*it == "Pressure" || *it== "vg_Pressure") {
         // Overall scalar pressure from all populations
         outputReducer->addOperator(new DRO::VariablePressureSolver);
         continue;
      }
      if(*it == "fg_Pressure") {
         // Overall scalar pressure from all populations
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_Pressure",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]);

               // Iterate through fsgrid cells and extract boundary flag
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        auto& moments=(*momentsGrid.get(x,y,z));
                        retval[gridSize[1]*gridSize[0]*z + gridSize[0]*y + x] = 1./3. * (moments[fsgrids::P_11] + moments[fsgrids::P_22] + moments[fsgrids::P_33]);
                     }
                  }
               }
               return retval;
         }
         ));
         continue;
      }
      if(*it == "populations_PTensor") {
         // Per-population pressure tensor, stored as diagonal and offdiagonal components
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            outputReducer->addOperator(new DRO::VariablePTensorDiagonal(i));
            outputReducer->addOperator(new DRO::VariablePTensorOffDiagonal(i));
         }
         continue;
      }
      if(*it == "BVOLderivs") {
         // Volume-averaged derivatives
         outputReducer->addOperator(new DRO::DataReductionOperatorBVOLDerivatives("dPERBXVOLdy",bvolderivatives::dPERBXVOLdy,1));
         outputReducer->addOperator(new DRO::DataReductionOperatorBVOLDerivatives("dPERBXVOLdz",bvolderivatives::dPERBXVOLdz,1));
         outputReducer->addOperator(new DRO::DataReductionOperatorBVOLDerivatives("dPERBYVOLdx",bvolderivatives::dPERBYVOLdx,1));
         outputReducer->addOperator(new DRO::DataReductionOperatorBVOLDerivatives("dPERBYVOLdz",bvolderivatives::dPERBYVOLdz,1));
         outputReducer->addOperator(new DRO::DataReductionOperatorBVOLDerivatives("dPERBZVOLdx",bvolderivatives::dPERBZVOLdx,1));
         outputReducer->addOperator(new DRO::DataReductionOperatorBVOLDerivatives("dPERBZVOLdy",bvolderivatives::dPERBZVOLdy,1));
         continue;
      }
      if(*it == "vg_GridCoordinates") {
         // Spatial coordinates for each cell
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("vg_X",CellParams::XCRD,1));
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("vg_Y",CellParams::YCRD,1));
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("vg_Z",CellParams::ZCRD,1));
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("vg_DX",CellParams::DX,1));
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("vg_DY",CellParams::DY,1));
         outputReducer->addOperator(new DRO::DataReductionOperatorCellParams("vg_DZ",CellParams::DZ,1));
         continue;
      }
      if(*it == "fg_GridCoordinates") { 
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_X",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]);

               // Iterate through fsgrid cells and extract X coordinate
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        retval[gridSize[1]*gridSize[0]*z + gridSize[0]*y + x] = technicalGrid.getPhysicalCoords(x,y,z)[0];
                     }
                  }
               }
               return retval;
         }
         ));
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_Y",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]);

               // Iterate through fsgrid cells and extract Y coordinate
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        retval[gridSize[1]*gridSize[0]*z + gridSize[0]*y + x] = technicalGrid.getPhysicalCoords(x,y,z)[1];
                     }
                  }
               }
               return retval;
         }
         ));
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_Z",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2]);

               // Iterate through fsgrid cells and extract Z coordinate
               for(int z=0; z<gridSize[2]; z++) {
                  for(int y=0; y<gridSize[1]; y++) {
                     for(int x=0; x<gridSize[0]; x++) {
                        retval[gridSize[1]*gridSize[0]*z + gridSize[0]*y + x] = technicalGrid.getPhysicalCoords(x,y,z)[2];
                     }
                  }
               }
               return retval;
         }
         ));
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_DX",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2], technicalGrid.DX);
               return retval;
         }
         ));
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_DY",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2], technicalGrid.DY);
               return retval;
         }
         ));
         outputReducer->addOperator(new DRO::DataReductionOperatorFsGrid("fg_DZ",[](
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid)->std::vector<double> {

               std::array<int32_t,3>& gridSize = technicalGrid.getLocalSize();
               std::vector<double> retval(gridSize[0]*gridSize[1]*gridSize[2], technicalGrid.DZ);
               return retval;
         }
         ));
         continue;
      }
      if (*it == "MeshData") {
         outputReducer->addOperator(new DRO::VariableMeshData);
         continue;
      }
      // After all the continue; statements one should never land here.
      int myRank;
      MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
      if (myRank == MASTER_RANK) {
         std::cerr << __FILE__ << ":" << __LINE__ << ": The output variable " << *it << " is not defined." << std::endl;
      }
      MPI_Finalize();
      exit(1);
   }

   for (it = P::diagnosticVariableList.begin();
        it != P::diagnosticVariableList.end();
        it++) {
      if (*it == "populations_Blocks") {
         // Per-population total block counts
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            diagnosticReducer->addOperator(new DRO::Blocks(i));
         }
         continue;
      }
      if(*it == "Rhom") {
         // Overall mass density
         diagnosticReducer->addOperator(new DRO::DataReductionOperatorCellParams("rho",CellParams::RHOM,1));
         continue;
      }
      if(*it == "populations_RhoLossAdjust") {
         // Per-particle overall lost particle number
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            species::Species& species=getObjectWrapper().particleSpecies[i];
            const std::string& pop = species.name;
            diagnosticReducer->addOperator(new DRO::DataReductionOperatorPopulations<Real>(pop + "/rho_loss_adjust", i, offsetof(spatial_cell::Population, RHOLOSSADJUST), 1));
         }
         continue;
      }
      //if(*it == "RhoLossVelBoundary") {
      //   diagnosticReducer->addOperator(new DRO::DataReductionOperatorCellParams("rho_loss_velocity_boundary",CellParams::RHOLOSSVELBOUNDARY,1));
      //   continue;
      //}
      if(*it == "LBweight") {
         diagnosticReducer->addOperator(new DRO::DataReductionOperatorCellParams("LB_weight",CellParams::LBWEIGHTCOUNTER,1));
         continue;
      }
      if(*it == "MaxVdt") {
         diagnosticReducer->addOperator(new DRO::DataReductionOperatorCellParams("max_v_dt",CellParams::MAXVDT,1));
         continue;
      }
      if(*it == "MaxRdt") {
         diagnosticReducer->addOperator(new DRO::DataReductionOperatorCellParams("max_r_dt",CellParams::MAXRDT,1));
         continue;
      }
      if(*it == "MaxFieldsdt") {
         diagnosticReducer->addOperator(new DRO::DataReductionOperatorCellParams("max_fields_dt",CellParams::MAXFDT,1));
         continue;
      }
      if(*it == "populations_MaxDistributionFunction") {
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            diagnosticReducer->addOperator(new DRO::MaxDistributionFunction(i));
         }
         continue;
      }
      if(*it == "populations_MinDistributionFunction") {
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            diagnosticReducer->addOperator(new DRO::MinDistributionFunction(i));
         }
         continue;
      }
      if(*it == "populations_MaxRdt") {
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            species::Species& species=getObjectWrapper().particleSpecies[i];
            const std::string& pop = species.name;
            diagnosticReducer->addOperator(new DRO::DataReductionOperatorPopulations<Real>(pop + "/Blocks", i, offsetof(spatial_cell::Population, max_dt[0]), 1));
         }
         continue;
      }
      if(*it == "populations_MaxVdt") {
         for(unsigned int i =0; i < getObjectWrapper().particleSpecies.size(); i++) {
            species::Species& species=getObjectWrapper().particleSpecies[i];
            const std::string& pop = species.name;
            diagnosticReducer->addOperator(new DRO::DataReductionOperatorPopulations<Real>(pop + "/Blocks", i, offsetof(spatial_cell::Population, max_dt[1]), 1));
         }
         continue;
      }
      // After all the continue; statements one should never land here.
      int myRank;
      MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
      if (myRank == MASTER_RANK) {
         std::cerr << __FILE__ << ":" << __LINE__ << ": The diagnostic variable " << *it << " is not defined." << std::endl;
      }
      MPI_Finalize();
      exit(1);
   }
}

// ************************************************************
// ***** DEFINITIONS FOR DATAREDUCER CLASS *****
// ************************************************************

/** Constructor for class DataReducer.
 */
DataReducer::DataReducer() { }

/** Destructor for class DataReducer. All stored DataReductionOperators 
 * are deleted.
 */
DataReducer::~DataReducer() {
   // Call delete for each DataReductionOperator:
   for (vector<DRO::DataReductionOperator*>::iterator it=operators.begin(); it!=operators.end(); ++it) {
      delete *it;
      *it = NULL;
   }
}

/** Add a new DRO::DataReductionOperator which has been created with new operation. 
 * DataReducer will take care of deleting it.
 * @return If true, the given DRO::DataReductionOperator was added successfully.
 */
bool DataReducer::addOperator(DRO::DataReductionOperator* op) {
   operators.push_back(op);
   return true;
}

/** Get the name of a DataReductionOperator.
 * @param operatorID ID number of the operator whose name is requested.
 * @return Name of the operator.
 */
std::string DataReducer::getName(const unsigned int& operatorID) const {
   if (operatorID >= operators.size()) return "";
   return operators[operatorID]->getName();
}

/** Get info on the type of data calculated by the given DataReductionOperator.
 * A DataReductionOperator writes an array on disk. Each element of the array is a vector with n elements. Finally, each
 * vector element has a byte size, as given by the sizeof function.
 * @param operatorID ID number of the DataReductionOperator whose output data info is requested.
 * @param dataType Basic datatype, must be int, uint, or float.
 * @param dataSize Byte size of written datatype, for example double-precision floating points
 * have byte size of sizeof(double).
 * @param vectorSize How many elements are in the vector returned by the DataReductionOperator.
 * @return If true, DataReductionOperator was found and it returned sensible values.
 */
bool DataReducer::getDataVectorInfo(const unsigned int& operatorID,std::string& dataType,unsigned int& dataSize,unsigned int& vectorSize) const {
   if (operatorID >= operators.size()) return false;
   return operators[operatorID]->getDataVectorInfo(dataType,dataSize,vectorSize);
}

/** Ask a DataReductionOperator if it wants to take care of writing the data 
 * to output file instead of letting be handled in iowrite.cpp. 
 * @param operatorID ID number of the DataReductionOperator.
 * @return If true, then VLSVWriter should be passed to the DataReductionOperator.*/
bool DataReducer::handlesWriting(const unsigned int& operatorID) const {
   if (operatorID >= operators.size()) return false;
   return dynamic_cast<DRO::DataReductionOperatorHandlesWriting*>(operators[operatorID]) != nullptr;
}

/** Ask a DataReductionOperator if it wants to write parameters to the vlsv file header
 * @param operatorID ID number of the DataReductionOperator.
 * @return If true, then VLSVWriter should be passed to the DataReductionOperator.*/
bool DataReducer::hasParameters(const unsigned int& operatorID) const {
   if (operatorID >= operators.size()) return false;
   return dynamic_cast<DRO::DataReductionOperatorHasParameters*>(operators[operatorID]) != nullptr;
}

/** Request a DataReductionOperator to calculate its output data and to write it to the given buffer.
 * @param cell Pointer to spatial cell whose data is to be reduced.
 * @param operatorID ID number of the applied DataReductionOperator.
 * @param buffer Buffer in which DataReductionOperator should write its data.
 * @return If true, DataReductionOperator calculated and wrote data successfully.
 */
bool DataReducer::reduceData(const SpatialCell* cell,const unsigned int& operatorID,char* buffer) {
   // Tell the chosen operator which spatial cell we are counting:
   if (operatorID >= operators.size()) return false;
   if (operators[operatorID]->setSpatialCell(cell) == false) return false;

   if (operators[operatorID]->reduceData(cell,buffer) == false) return false;
   return true;
}

/** Request a DataReductionOperator to calculate its output data and to write it to the given variable.
 * @param cell Pointer to spatial cell whose data is to be reduced.
 * @param operatorID ID number of the applied DataReductionOperator.
 * @param result Real variable in which DataReductionOperator should write its result.
 * @return If true, DataReductionOperator calculated and wrote data successfully.
 */
bool DataReducer::reduceDiagnostic(const SpatialCell* cell,const unsigned int& operatorID,Real * result) {
   // Tell the chosen operator which spatial cell we are counting:
   if (operatorID >= operators.size()) return false;
   if (operators[operatorID]->setSpatialCell(cell) == false) return false;
   
   if (operators[operatorID]->reduceDiagnostic(cell,result) == false) return false;
   return true;
}

/** Get the number of DataReductionOperators stored in DataReducer.
 * @return Number of DataReductionOperators stored in DataReducer.
 */
unsigned int DataReducer::size() const {return operators.size();}

/** Write all data from given DataReductionOperator to the output file.
 * @param operatorID ID number of the selected DataReductionOperator.
 * @param mpiGrid Parallel grid library.
 * @param cells Vector containing spatial cell IDs.
 * @param meshName Name of the spatial mesh in the output file.
 * @param vlsvWriter VLSV file writer that has output file open.
 * @return If true, DataReductionOperator wrote its data successfully.*/
bool DataReducer::writeData(const unsigned int& operatorID,
                  const dccrg::Dccrg<spatial_cell::SpatialCell,dccrg::Cartesian_Geometry>& mpiGrid,
                  const std::vector<CellID>& cells,const std::string& meshName,
                  vlsv::Writer& vlsvWriter) {
   if (operatorID >= operators.size()) return false;
   DRO::DataReductionOperatorHandlesWriting* writingOperator = dynamic_cast<DRO::DataReductionOperatorHandlesWriting*>(operators[operatorID]);
   if(writingOperator == nullptr) {
      return false;
   }
   return writingOperator->writeData(mpiGrid,cells,meshName,vlsvWriter);
}

/** Write parameters related to given DataReductionOperator to the output file.
 * @param operatorID ID number of the selected DataReductionOperator.
 * @param vlsvWriter VLSV file writer that has output file open.
 * @return If true, DataReductionOperator wrote its parameters successfully.*/
bool DataReducer::writeParameters(const unsigned int& operatorID, vlsv::Writer& vlsvWriter) {
   if (operatorID >= operators.size()) return false;
   DRO::DataReductionOperatorHasParameters* parameterOperator = dynamic_cast<DRO::DataReductionOperatorHasParameters*>(operators[operatorID]);
   if(parameterOperator == nullptr) {
      return false;
   }
   return parameterOperator->writeParameters(vlsvWriter);
}
/** Write all data thet the given DataReductionOperator wants to obtain from fsgrid into the output file.
 */
bool DataReducer::writeFsGridData(
                      FsGrid< std::array<Real, fsgrids::bfield::N_BFIELD>, 2>& perBGrid,
                      FsGrid< std::array<Real, fsgrids::efield::N_EFIELD>, 2>& EGrid,
                      FsGrid< std::array<Real, fsgrids::ehall::N_EHALL>, 2>& EHallGrid,
                      FsGrid< std::array<Real, fsgrids::egradpe::N_EGRADPE>, 2>& EGradPeGrid,
                      FsGrid< std::array<Real, fsgrids::moments::N_MOMENTS>, 2>& momentsGrid,
                      FsGrid< std::array<Real, fsgrids::dperb::N_DPERB>, 2>& dPerBGrid,
                      FsGrid< std::array<Real, fsgrids::dmoments::N_DMOMENTS>, 2>& dMomentsGrid,
                      FsGrid< std::array<Real, fsgrids::bgbfield::N_BGB>, 2>& BgBGrid,
                      FsGrid< std::array<Real, fsgrids::volfields::N_VOL>, 2>& volGrid,
                      FsGrid< fsgrids::technical, 2>& technicalGrid, const std::string& meshName, const unsigned int operatorID, vlsv::Writer& vlsvWriter) {
   
   if (operatorID >= operators.size()) return false;
   DRO::DataReductionOperatorFsGrid* DROf = dynamic_cast<DRO::DataReductionOperatorFsGrid*>(operators[operatorID]);
   if(!DROf) {
      return false;
   } else {
      return DROf->writeFsGridData(perBGrid, EGrid, EHallGrid, EGradPeGrid, momentsGrid, dPerBGrid, dMomentsGrid, BgBGrid, volGrid, technicalGrid, meshName, vlsvWriter);
   }
}
