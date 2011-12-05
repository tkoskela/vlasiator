/*
This file is part of Vlasiator.

Copyright 2010, 2011 Finnish Meteorological Institute

Vlasiator is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.

Vlasiator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <omp.h>

#include "cpu_acc_leveque.h"

using namespace std;


///FIXME, is this function an old remnant of KT?? Does not appear to be called for leveque

bool cpu_acceleration(SpatialCell& cell) {
   // Clear spatial cell velocity moments:
   cell.cpu_cellParams[CellParams::RHO]   = 0.0;
   cell.cpu_cellParams[CellParams::RHOVX] = 0.0;
   cell.cpu_cellParams[CellParams::RHOVY] = 0.0;
   cell.cpu_cellParams[CellParams::RHOVZ] = 0.0;
   
   bool success = true;
   
   Real accmat[9]; // Acceleration matrix
   Real B[3];
   B[0] = cell.cpu_cellParams[CellParams::BX];
   B[1] = cell.cpu_cellParams[CellParams::BY];
   B[2] = cell.cpu_cellParams[CellParams::BZ];
   creal Bmag = sqrt(B[0]*B[0] + B[1]*B[1] + B[2]*B[2]);
   for (uint i=0; i<3; ++i) B[i] /= Bmag;
   
   creal dt = 0.0*Parameters::dt;
   creal omega = Parameters::q_per_m * Bmag;
   creal omega2 = omega*omega;

   accmat[0] = 0.5*omega2*(B[0]*B[0]-1.0)*dt;
   accmat[1] = 0.5*omega2*B[0]*B[1]*dt - omega*B[2];
   accmat[2] = 0.5*omega2*B[0]*B[2]*dt + omega*B[1];
   accmat[3] = 0.5*omega2*B[0]*B[1]*dt + omega*B[2];
   accmat[4] = 0.5*omega2*(B[1]*B[1]-1.0)*dt;
   accmat[5] = 0.5*omega2*B[1]*B[2]*dt - omega*B[0];
   accmat[6] = 0.5*omega2*B[0]*B[2]*dt - omega*B[1];
   accmat[7] = 0.5*omega2*B[1]*B[2]*dt + omega*B[0];
   accmat[8] = 0.5*omega2*(B[2]*B[2]-1.0)*dt;
   /*
   accmat[0] = omega2*(B[0]*B[0]-1.0)*dt;
   accmat[1] = omega2*B[0]*B[1]*dt/6.0 - 0.5*omega*B[2];
   accmat[2] = omega2*B[0]*B[2]*dt/6.0 + 0.5*omega*B[1];
   accmat[3] = omega2*B[0]*B[1]*dt/6.0 + 0.5*omega*B[2];
   accmat[4] = omega2*(B[1]*B[1]-1.0)*dt/6.0;
   accmat[5] = omega2*B[1]*B[2]*dt/6.0 - 0.5*omega*B[0];
   accmat[6] = omega2*B[0]*B[2]*dt/6.0 - 0.5*omega*B[1];
   accmat[7] = omega2*B[1]*B[2]*dt/6.0 + 0.5*omega*B[0];
   accmat[8] = omega2*(B[2]*B[2]-1.0)*dt/6.0;
   */
   for (uint block=0; block<cell.N_blocks; ++block) 
      cpu_clearVelFluxes<Real>(cell,block);

   for (uint block=0; block<cell.N_blocks; ++block) 
     cpu_calcVelFluxes<Real>(cell,block,Parameters::dt,accmat);

   for (uint block=0; block<cell.N_blocks; ++block) 
     cpu_propagateVel<Real>(cell,block,Parameters::dt);
   
   return success;
}

