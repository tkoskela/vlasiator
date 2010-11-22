#include <cstdlib>
#include <iostream>
#include <map>
#include <limits>
#include <list>
#include <vector>
#include <mpi.h>
#include <zoltan_cpp.h>

#include "definitions.h"
#include "parameters.h"

namespace ID {
   typedef unsigned int type;
}

enum LBM {
   Block,Random,RCB,RIB,HSFC,Graph
};

template<typename T> MPI_Datatype MPI_Type() {return 0;}
template<> MPI_Datatype MPI_Type<int>() {return MPI_INT;}
template<> MPI_Datatype MPI_Type<unsigned int>() {return MPI_UNSIGNED;}
template<> MPI_Datatype MPI_Type<float>() {return MPI_FLOAT;}
template<> MPI_Datatype MPI_Type<double>() {return MPI_DOUBLE;}

template<class C> struct ParCell {
   uint unrefInd_i;
   uint unrefInd_j;
   uint unrefInd_k;
   Real xcrd;
   Real ycrd;
   Real zcrd;
   uint N_blocks;    /**< Number of velocity blocks in the cell.*/
   
   bool hasRemoteNeighbours; /**< If true, at least one of the cell's neighbours are 
			      * assigned to a different MPI process.*/
   ID::type neighbours[6];   /**< The global ID of each neighbour. [0]=-x, [1]=+x
			      * [2]=-y, [3]=+y, [4]=-z, [5]=+z. If the neighbour does 
			      * not exist, the value is std::numeric_limits<ID::type>::max().*/

   C* dataptr;               /**< Pointer to user data.*/
   ParCell(): dataptr(NULL),N_blocks(0),hasRemoteNeighbours(false) {
      for (int i=0; i<6; ++i) neighbours[i] = std::numeric_limits<ID::type>::max();
   }
};

template<class C> class ParGrid {
 public: 
   
   ParGrid(cuint& xsize,cuint& ysize,cuint& zsize,creal& xmin,creal& ymin,creal& zmin,
	   creal& xmax,creal& ymax,creal& zmax,const LBM& method,int& argn,char* args[]);
   ~ParGrid();

   // Some functions which have the same name as in dccrg:
   Real get_cell_x_size(const ID::type& globalID) const {return unref_dx;}
   Real get_cell_y_size(const ID::type& globalID) const {return unref_dy;}
   Real get_cell_z_size(const ID::type& globalID) const {return unref_dz;}
   Real get_cell_x(const ID::type& globalID) const;
   Real get_cell_y(const ID::type& globalID) const;
   Real get_cell_z(const ID::type& globalID) const;
   
   void barrier() {MPI_Barrier(MPI_COMM_WORLD);}
   template<class CONT> void getBoundaryCells(CONT& rlist) const;
   template<class CONT> void getCells(CONT& rlist) const;
   template<class CONT> void getInnerCells(CONT& rlist) const;
   template<class CONT> void getNeighbours(CONT& rlist,const ID::type& globalID) const;
   template<class CONT> void getReceiveList(CONT& rlist) const;
   template<class CONT> void getRemoteCells(CONT& rlist) const;
   template<class CONT> void getSendList(CONT& rlist) const;
   bool initialLoadBalance();
   bool isInitialized() const {return initialized;}
   C* operator[](const ID::type& id) const;
   void print() const;
   int rank() const;
   bool setLoadBalancingMethod(const LBM& method);
   bool startNeighbourExchange(const uint& identifier);
   bool waitAll();

   class iterator;
   friend class iterator;
   class iterator {
    public:
      iterator(const bool& allCells=true,const bool& iteratingInner=false);
      iterator& operator++();
      void set(typename std::map<ID::type,ParCell<C> >::iterator& it);
      bool operator==(const ParGrid<C>::iterator& x);
      bool operator!=(const ParGrid<C>::iterator& x);
    private:
      bool allCells;
      bool iteratingInner;
      typename std::map<ID::type,ParCell<C> >::iterator it;
   };
   
   iterator begin();
   iterator boundaryCellsBegin();
   iterator innerCellsBegin();
   iterator end();
   
   // Zoltan callback functions
   static void getCellCoordinates(void* data,int N_globalEntries,int N_localEntries,ZOLTAN_ID_PTR globalID,
				  ZOLTAN_ID_PTR localID,double* geometryData,int* rcode);
   static void getLocalCellList(void* data,int N_globalIDs,int N_localIDs,ZOLTAN_ID_PTR globalIDs,
				ZOLTAN_ID_PTR localIDs,int weightDim,float* cellWeights,int* rcode);
   static int getGridDimensions(void* data,int* rcode);
   static void getEdgeList(void* parGridPtr,int N_globalIDs,int N_localIDs,ZOLTAN_ID_PTR globalID,
			   ZOLTAN_ID_PTR localID,ZOLTAN_ID_PTR nbrGlobalIDs,int* nbrHosts,
			   int N_weights,float* weight,int* rcode);
   static int getNumberOfEdges(void* parGridPtr,int N_globalIDs,int N_localIDs,ZOLTAN_ID_PTR globalIDs,
			       ZOLTAN_ID_PTR localIDs,int* rcode);
   static int getNumberOfLocalCells(void* data,int* rcode);
   
 private:
   LBM balanceMethod;        /**< The load balance method currently in use. Default is RCB.*/
   Real grid_xmin;           /**< x-coordinate of the lower left corner of the grid.*/
   Real grid_xmax;           /**< x-coordinate of the upper right corner of the grid.*/
   Real grid_ymin;           /**< y-coordinate of the lower left corner of the grid.*/
   Real grid_ymax;           /**< y-coordinate of the upper right corner of the grid.*/
   Real grid_zmin;           /**< z-coordinate of the lower left corner of the grid.*/
   Real grid_zmax;           /**< z-coordinate of the upper right corner of the grid.*/
   bool initialized;         /**< If true, ParGrid was initialized successfully and is ready for use.*/
   MPI_Datatype MPIdataType; /**< The MPI datatype which is currently used in communications.*/
   int myrank;               /**< The rank if this MPI process.*/
   int N_processes;          /**< Total number of MPI processes.*/
   bool periodic_x;          /**< If true, x-direction is periodic.*/
   bool periodic_y;          /**< If true, y-direction is periodic.*/
   bool periodic_z;          /**< If true, z-direction is periodic.*/
   Real unref_dx;            /**< Unrefined cell size in x-direction.*/
   Real unref_dy;            /**< Unrefined cell size in y-direction.*/
   Real unref_dz;            /**< Unrefined cell size in z-direction.*/
   uint unrefSize_x;         /**< Number of x-cells in unrefined grid.*/
   uint unrefSize_y;         /**< Number of y-cells in unrefined grid.*/
   uint unrefSize_z;         /**< Number of z-cells in unrefined grid.*/
   Zoltan* zoltan;           /**< Pointer to Zoltan load balancing library.*/
   
   static std::map<ID::type,int> hostProcesses;            /**< For each cell, the host process that contains that cell.
							    * The contents of this array should be the same on each MPI
							    * process.*/
   static std::map<ID::type,ParCell<C> > localCells;       /**< Associative container containing all cells that are currently 
							    * assigned to this process.*/
   std::vector<MPI_Request> MPIrequests;
   std::vector<MPI_Status> MPIstatuses;
   static std::map<ID::type,int> receiveList;              /**< A list of cells, identified by their global ID, that are 
							    * sent to this process during neighbour data exchange, and the 
							    * rank of the MPI process that sends the cell.*/
   static std::map<ID::type,ParCell<C> > remoteCells;
   static std::map<std::pair<ID::type,int>,char> sendList; /**< A list of cells, identified by their global ID, that this 
							    * MPI process has to send during neighbour data exchange, and 
							    * the rank of the receiving MPI process.*/

   void allocateCells();
   void buildExchangeLists();
   void buildInitialGrid();
   void buildUnrefNeighbourLists();
   ID::type calculateUnrefinedIndex(const ID::type& i,const ID::type& j,const ID::type& k);
   void calculateUnrefinedIndices(const ID::type& index,ID::type& i,ID::type& j,ID::type& k);
   std::string loadBalanceMethod(const LBM& method) const;
   void syncCellAssignments();
   void syncCellCoordinates();
};

// ***************************************************************
// ************** DEFINITIONS FOR STATIC MEMBERS *****************
// ***************************************************************
template<class C> std::map<ID::type,int> ParGrid<C>::hostProcesses;
template<class C> std::map<ID::type,ParCell<C> > ParGrid<C>::localCells;
template<class C> std::map<ID::type,int> ParGrid<C>::receiveList;
template<class C> std::map<ID::type,ParCell<C> > ParGrid<C>::remoteCells;
template<class C> std::map<std::pair<ID::type,int>,char> ParGrid<C>::sendList;

// ***************************************************************
// ************** BEGIN MEMBER FUNCTION DEFITINIONS **************
// ***************************************************************
template<class C> ParGrid<C>::ParGrid(cuint& xsize,cuint& ysize,cuint& zsize,creal& xmin,creal& ymin,creal& zmin,
				      creal& xmax,creal& ymax,creal& zmax,const LBM& method,int& argn,char* args[]) {
   initialized = true;
   unrefSize_x = xsize;
   unrefSize_y = ysize;
   unrefSize_z = zsize;
   unref_dx = (xmax - xmin) / xsize;
   unref_dy = (ymax - ymin) / ysize;
   unref_dz = (zmax - zmin) / zsize;
   grid_xmin = xmin;
   grid_xmax = xmax;
   grid_ymin = ymin;
   grid_ymax = ymax;
   grid_zmin = zmin;
   grid_zmax = zmax;
   balanceMethod = method;
   periodic_x = false;
   periodic_y = false;
   periodic_z = false;
   
   // Attempt to init MPI:
   int rvalue = MPI_Init(&argn,&args);
   if (rvalue != MPI_SUCCESS) {
      std::cerr << "ParGrid: MPI init failed!" << std::endl;
      initialized = false;
   }
   if (initialized == false) return;
   // Get the rank of this process, and the total number of MPI processes:
   MPI_Comm_size(MPI_COMM_WORLD,&N_processes);
   MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
   
   // Attempt to init Zoltan:
   float zoltanVersion;
   rvalue = Zoltan_Initialize(argn,args,&zoltanVersion);
   if (rvalue != ZOLTAN_OK) {
      std::cerr << "ParGrid: Zoltan init failed!" << std::endl;
      initialized = false;
   }
   if (initialized == false) return;
   
   // Create a new Zoltan object:
   zoltan = new Zoltan(MPI_COMM_WORLD);
   
   // Set Zoltan parameters:
   zoltan->Set_Param("NUM_GID_ENTRIES","1");
   zoltan->Set_Param("LB_METHOD",loadBalanceMethod(balanceMethod).c_str());
   zoltan->Set_Param("RETURN_LISTS","ALL");
   
   // Register generic callback functions:
   zoltan->Set_Num_Obj_Fn(&ParGrid<C>::getNumberOfLocalCells,this);
   zoltan->Set_Obj_List_Fn(&ParGrid<C>::getLocalCellList,this);   
   // Register geometry-based load balancing callback functions:
   zoltan->Set_Num_Geom_Fn(&ParGrid<C>::getGridDimensions,this);
   zoltan->Set_Geom_Fn(&ParGrid<C>::getCellCoordinates,this);
   // Register graph-based load balancing callback functions:
   zoltan->Set_Num_Edges_Fn(&ParGrid<C>::getNumberOfEdges,this);
   zoltan->Set_Edge_List_Fn(&ParGrid<C>::getEdgeList,this);
   
   buildInitialGrid();    // Build an initial guess for the grid
   syncCellAssignments();
   buildUnrefNeighbourLists();
   
   initialLoadBalance();  // Load balance grid. Invalidates contents of hostProcesses
                          // and neighbour lists (since no data is actually transmitted).
   syncCellAssignments();
   buildUnrefNeighbourLists();
   buildExchangeLists();  // Build send/receive lists.
   
   // Now cells have been assigned to this process, and we also know which 
   // cells this process will receive from other MPI processes. Allocate 
   // memory:
   allocateCells();
   syncCellCoordinates();
}

template<class C> ParGrid<C>::~ParGrid() {
   // Call destructor for each user-defined data cell:
   typename std::map<ID::type,ParCell<C> >::iterator it = localCells.begin();
   while (it != localCells.end()) {
      delete it->second.dataptr;
      it->second.dataptr = NULL;
      ++it;
   }   
   it = remoteCells.begin();
   while (it != remoteCells.end()) {
      delete it->second.dataptr;
      it->second.dataptr = NULL;
      ++it;
   }
   
   // Delete Zoltan before calling MPI_Finalize():
   delete zoltan;
   zoltan = NULL;
   
   if (initialized == true) MPI_Finalize();
}

/** Call the constructor of each user-defined data cell. Here we also allocate 
 * memory for remote cells.
 */
template<class C> void ParGrid<C>::allocateCells() {
   // Local cells have been pushed to localCells, but remoteCells is empty. Insert remote cells.
   for (std::map<ID::type,int>::const_iterator it=receiveList.begin(); it!=receiveList.end(); ++it) {
      remoteCells[it->first];
   }
   for (int i=0; i<N_processes; ++i) {
      if (i==myrank) {
	 // Now ask user to allocate memory for each cell (if necessary):
	 for (typename std::map<ID::type,ParCell<C> >::iterator it=localCells.begin(); it!=localCells.end(); ++it) {
	    it->second.dataptr = new C;
	 }
	 for (typename std::map<ID::type,ParCell<C> >::iterator it=remoteCells.begin(); it!=remoteCells.end(); ++it) {
	    it->second.dataptr = new C;
	 }
      }
      MPI_Barrier(MPI_COMM_WORLD);
   }
}

/**
 * Prerequisite: the neighour lists must be up-to-date.
 */
template<class C>
void ParGrid<C>::buildExchangeLists() {
   // Delete old data
   receiveList.clear();
   sendList.clear();
   
   bool hasRemotes;
   typename std::map<ID::type,ParCell<C> >::iterator it = localCells.begin();
   while (it != localCells.end()) {
      hasRemotes = false;
      // Go through the cell's neighbour list
      for (int i=0; i<6; ++i) {
	 // Check that the neighbour exists and that it is not assigned to this process:
	 const ID::type nbrID = it->second.neighbours[i];
	 if (nbrID == std::numeric_limits<ID::type>::max()) continue;
	 const int hostID = hostProcesses[it->second.neighbours[i]];
	 if (hostID == myrank) continue;
	 // Cell has a remote neighbour. Add this cell to sendList, and its remote 
	 // neighbour to receiveList.
	 hasRemotes = true;
	 //sendList[it->first] = hostID;
	 sendList[std::pair<ID::type,int>(it->first,hostID)] = 1;
	 receiveList[nbrID] = hostID;
      }
      it->second.hasRemoteNeighbours = hasRemotes;
      ++it;
   }
}

template<class C> void ParGrid<C>::buildInitialGrid() {
   cuint N_total = unrefSize_x*unrefSize_y*unrefSize_z;
   
   // Determine how many cells should be built on this process:
   cuint N_cells = N_total / N_processes;
   uint N_extra = 0;
   if (N_total % N_processes > 0) {
      if (myrank < N_total % N_processes) ++N_extra;
   }
   
   // Determine the global ID of each cell to build. Note that the number of 
   // unrefined cells is probably not exactly divisible by the number of MPI
   // processes, and thus some processes should get an extra cell whose index
   // is calculated to i_extra.
   cuint i_start = myrank*(N_total / N_processes);
   cuint i_end   = i_start + N_cells;
   uint i_extra = 0;
   if (N_extra > 0) i_extra = N_processes*(N_total/N_processes) + (myrank);
   // Push all cells to localCells:
   for (uint i=i_start; i<i_end; ++i) {
      ParCell<C> p;
      calculateUnrefinedIndices(i,p.unrefInd_i,p.unrefInd_j,p.unrefInd_k);
      p.xcrd = grid_xmin + p.unrefInd_i*unref_dx;
      p.ycrd = grid_ymin + p.unrefInd_j*unref_dy;
      p.zcrd = grid_zmin + p.unrefInd_k*unref_dz;
      localCells[i] = p;
   }
   // If an extra cell was assigned to this process, push it to localCells:
   if (i_extra > 0) {
      ParCell<C> p;
      calculateUnrefinedIndices(i_extra,p.unrefInd_i,p.unrefInd_j,p.unrefInd_k);
      p.xcrd = grid_xmin + p.unrefInd_i*unref_dx;
      p.ycrd = grid_ymin + p.unrefInd_j*unref_dy;
      p.zcrd = grid_zmin + p.unrefInd_k*unref_dz;
      localCells[i_extra] = p;
   }
}

template<class C> void ParGrid<C>::buildUnrefNeighbourLists() {
   ID::type globalID;
   ID::type i,j,k;
   ID::type x_neg,x_pos,y_neg,y_pos,z_neg,z_pos;
   for (typename std::map<ID::type,ParCell<C> >::iterator it=localCells.begin(); it!=localCells.end(); ++it) {
      globalID = it->first;
      calculateUnrefinedIndices(globalID,i,j,k);
      // Determine x-indices of left and right x-neighbours:
      if (i == 0) {
	 if (periodic_x == true) x_neg = unrefSize_x-1;
	 else x_neg = std::numeric_limits<ID::type>::max();
      } else
	x_neg = i-1;
      if (i == unrefSize_x-1) {
	 if (periodic_x == true) x_pos = 0;
	 else x_pos = std::numeric_limits<ID::type>::max();
      } else
	x_pos = i+1;
      // Determine y-indices of left and right y-neighbours:
      if (j == 0) {
	 if (periodic_y == true) y_neg = unrefSize_y-1;
	 else y_neg = std::numeric_limits<ID::type>::max();
      } else
	y_neg = j-1;
      if (j == unrefSize_y-1) {
	 if (periodic_y == true) y_pos = 0;
	 else y_pos = std::numeric_limits<ID::type>::max();
      } else
	y_pos = j+1;
      // Determine z-indices of left and right z-neighbours:
      if (k == 0) {
	 if (periodic_z == true) z_neg = unrefSize_z-1;
	 else z_neg = std::numeric_limits<ID::type>::max();
      } else
	z_neg = k-1;
      if (k == unrefSize_z-1) {
	 if (periodic_z == true) z_pos = 0;
	 else z_pos = std::numeric_limits<ID::type>::max();
      } else
	z_pos = k+1;
      
      // Store calculated neighbour indices:
      if (x_neg != std::numeric_limits<ID::type>::max()) x_neg = calculateUnrefinedIndex(x_neg,j,k);
      if (x_pos != std::numeric_limits<ID::type>::max()) x_pos = calculateUnrefinedIndex(x_pos,j,k);
      if (y_neg != std::numeric_limits<ID::type>::max()) y_neg = calculateUnrefinedIndex(i,y_neg,k);
      if (y_pos != std::numeric_limits<ID::type>::max()) y_pos = calculateUnrefinedIndex(i,y_pos,k);
      if (z_neg != std::numeric_limits<ID::type>::max()) z_neg = calculateUnrefinedIndex(i,j,z_neg);
      if (z_pos != std::numeric_limits<ID::type>::max()) z_pos = calculateUnrefinedIndex(i,j,z_pos);
      it->second.neighbours[0] = x_neg;
      it->second.neighbours[1] = x_pos;
      it->second.neighbours[2] = y_neg;
      it->second.neighbours[3] = y_pos;
      it->second.neighbours[4] = z_neg;
      it->second.neighbours[5] = z_pos;
      /*
      if (myrank == 0) {
	 std::cout << "Cell #" << it->first << " (" << i << ',' << j << ',' << k <<") has nbrs = ";
	 for (int i=0; i<6; ++i) if (it->second.neighbours[i] != std::numeric_limits<ID::type>::max())
	   std::cout << i << ":" << it->second.neighbours[i] << ' ';
	 std::cout << std::endl;
      }*/
   }
}

template<class C> ID::type ParGrid<C>::calculateUnrefinedIndex(const ID::type& i,const ID::type& j,const ID::type& k) {
   return k*unrefSize_y*unrefSize_x + j*unrefSize_x + i;
}

template<class C> void ParGrid<C>::calculateUnrefinedIndices(const ID::type& index,ID::type& i,ID::type& j,ID::type& k) {
   cuint N_total = unrefSize_x*unrefSize_y*unrefSize_z;
   uint ind = index;
   k = ind / (unrefSize_x*unrefSize_y);
   ind -= k*unrefSize_x*unrefSize_y;
   j = ind / unrefSize_x;
   i = ind - j*unrefSize_x;
}

template<class C> bool ParGrid<C>::startNeighbourExchange(cuint& identifier) {
   bool rvalue = true;
   // Get the MPI_Datatype used in communication from user and pass it to MPI:
   // (This does not work with variable-lenght user data)
   C::getMPIdatatype(identifier,MPIdataType);   
   int rcode = MPI_Type_commit(&MPIdataType);
   #ifndef NDEBUG
   if (rcode != MPI_SUCCESS) {std::cerr << "ParGrid::exchangeNeighbourData MPI_Type_commit failed!" << std::endl; rvalue=false;}
   #endif
   
   // Post receives for each remote cell:
   uint counter = 0;
   //if (MPIrequests.size() < receiveList.size()) MPIrequests.resize(receiveList.size());
   //if (MPIstatuses.size() < receiveList.size()) MPIstatuses.resize(receiveList.size());
   for (std::map<ID::type,int>::const_iterator it=receiveList.begin(); it!=receiveList.end(); ++it) {
      void* const buffer = (remoteCells[it->first].dataptr)->getBaseAddress(identifier); // Starting address of receive buffer
      const int count = 1;                            // Number of elements in receive buffer
      const int source = it->second;                  // Rank of source MPI process
      const int tag = it->first;                      // Message tag (use global ID)
      MPIrequests.push_back(MPI_Request());
      if (MPI_Irecv(buffer,count,MPIdataType,source,tag,MPI_COMM_WORLD,&(MPIrequests.back())) != MPI_SUCCESS) rvalue=false;
      ++counter;
   }
   // Post sends for each boundary cell:
   for (std::map<std::pair<ID::type,int>,char>::const_iterator it=sendList.begin(); it!=sendList.end(); ++it) {
      void* const buffer = (localCells[it->first.first].dataptr)->getBaseAddress(identifier); // Starting address of send buffer
      const int count = 1;                            // Number of elements in send buffer
      const int dest = it->first.second;              // Rank of destination MPI process
      const int tag = it->first.first;                // Message tag
      MPI_Request dummyRequest;
      MPIstatuses.push_back(MPI_Status());
      if (MPI_Issend(buffer,count,MPIdataType,dest,tag,MPI_COMM_WORLD,&dummyRequest) != MPI_SUCCESS) rvalue=false;
      if (MPI_Request_free(&dummyRequest) != MPI_SUCCESS) rvalue=false;
   }
   /*
   if (myrank == 0) {
      std::cerr << "MPIrequests.size() = " << MPIrequests.size() << " MPIstatuses.size() = " << MPIstatuses.size() << std::endl;
   }*/
   //MPI_Barrier(MPI_COMM_WORLD); // This seems to be necessary...
   return rvalue;
}

template<class C> Real ParGrid<C>::get_cell_x(const ID::type& globalID) const {
   typename std::map<ID::type,ParCell<C> >::const_iterator it = localCells.find(globalID);
   if (it != localCells.end()) return it->second.xcrd + 0.5*unref_dx;
   it = remoteCells.find(globalID);
   if (it != remoteCells.end()) return it->second.xcrd + 0.5*unref_dx;
   return NAN;
}

template<class C> Real ParGrid<C>::get_cell_y(const ID::type& globalID) const {
   typename std::map<ID::type,ParCell<C> >::const_iterator it = localCells.find(globalID);
   if (it != localCells.end()) return it->second.ycrd + 0.5*unref_dy;
   it = remoteCells.find(globalID);
   if (it != remoteCells.end()) return it->second.ycrd + 0.5*unref_dy;
   return NAN;
}

template<class C> Real ParGrid<C>::get_cell_z(const ID::type& globalID) const {
   typename std::map<ID::type,ParCell<C> >::const_iterator it = localCells.find(globalID);
   if (it != localCells.end()) return it->second.zcrd + 0.5*unref_dz;
   it = remoteCells.find(globalID);
   if (it != remoteCells.end()) return it->second.zcrd + 0.5*unref_dz;
   return NAN;
}

template<class C> template<class CONT> void ParGrid<C>::getBoundaryCells(CONT& rlist) const {
   rlist.clear();
   typename std::map<ID::type,ParCell<C> >::const_iterator it = localCells.begin();
   while (it != localCells.end()) {
      if (it->second.hasRemoteNeighbours == true) rlist.push_back(it->first);
      ++it;
   }
}

template<class C> template<class CONT> void ParGrid<C>::getCells(CONT& rlist) const {
   rlist.clear();
   typename std::map<ID::type,ParCell<C> >::const_iterator it = localCells.begin();
   while (it != localCells.end()) {
      rlist.push_back(it->first);
      ++it;
   }
}

template<class C> template<class CONT> void ParGrid<C>::getInnerCells(CONT& rlist) const {
   rlist.clear();
   typename std::map<ID::type,ParCell<C> >::const_iterator it = localCells.begin();
   while (it != localCells.end()) {
      if (it->second.hasRemoteNeighbours == false) rlist.push_back(it->first);
      ++it;
   }
}

/** Get the globalIDs of the neighbours of the given cell.
 * The neighbours are given in order (-x,+x,-y,+y,-z,+z). If a neighbour does not exist,
 * e.g. due to the cell being on the boundary of the simulation box, the neighbour index is 
 * equal to std::numeric_limits<ID::type>::max(). If the given cell is not local to this 
 * MPI process, the rlist container is empty.
 * @param CONT An STL container where the neighbour indices are inserted.
 * @param globalID The global ID of the cell whose neighbours are queried.
 */
template<class C> template<class CONT> void ParGrid<C>::getNeighbours(CONT& rlist,const ID::type& globalID) const {
   rlist.clear();
   typename std::map<ID::type,ParCell<C> >::const_iterator it = localCells.find(globalID);
   if (it == localCells.end()) return;
   for (int i=0; i<6; ++i) rlist.push_back((it->second).neighbours[i]);
}

template<class C> template<class CONT> void ParGrid<C>::getReceiveList(CONT& rlist) const {
   rlist.clear();
   for (std::map<ID::type,int>::const_iterator it = receiveList.begin(); it != receiveList.end(); ++it) {
      rlist.push_back(std::pair<ID::type,int>(it->first,it->second));
   }
}

template<class C> template<class CONT> void ParGrid<C>::getRemoteCells(CONT& rlist) const {
   rlist.clear();
   typename std::map<ID::type,ParCell<C> >::const_iterator it = remoteCells.begin();
   while (it != remoteCells.end()) {
      rlist.push_back(it->first);
      ++it;
   }
}

template<class C> template<class CONT> void ParGrid<C>::getSendList(CONT& rlist) const {
   rlist.clear();
   for (std::map<std::pair<ID::type,int>,char>::const_iterator it = sendList.begin(); it != sendList.end(); ++it) {
      rlist.push_back(std::pair<ID::type,int>(it->first.first,it->first.second));
   }
}

template<class C> bool ParGrid<C>::initialLoadBalance() {
   bool rvalue = true;
   int changes,N_globalIDs,N_localIDs,N_import,N_export;
   int* importProcesses;
   int* importParts;
   int* exportProcesses;
   int* exportParts;
   ZOLTAN_ID_PTR importGlobalIDs;
   ZOLTAN_ID_PTR importLocalIDs;
   ZOLTAN_ID_PTR exportGlobalIDs;
   ZOLTAN_ID_PTR exportLocalIDs;
   
   // Ask Zoltan the cells which should be imported and exported:
   if (zoltan->LB_Partition(changes,N_globalIDs,N_localIDs,N_import,importGlobalIDs,importLocalIDs,
			    importProcesses,importParts,N_export,exportGlobalIDs,exportLocalIDs,
			    exportProcesses,exportParts) == ZOLTAN_OK) {
      // All required data is now known. Since this is an initial load balance, we actually don't 
      // need to send/receive anything. The unrefined indices and coordinates can be recalculated 
      // from the global IDs.

      // Erase exported cells:
      for (int i=0; i<N_export; ++i) {
	 localCells.erase(exportGlobalIDs[i]);
      }
      // Insert and recalculate imported cells:
      for (int i=0; i<N_import; ++i) {
	 ParCell<C> p;
	 calculateUnrefinedIndices(importGlobalIDs[i],p.unrefInd_i,p.unrefInd_j,p.unrefInd_k);
	 p.xcrd = grid_xmin + p.unrefInd_i*unref_dx;
	 p.ycrd = grid_ymin + p.unrefInd_j*unref_dy;
	 p.zcrd = grid_zmin + p.unrefInd_k*unref_dz;
	 localCells[importGlobalIDs[i]] = p;
      }
   } else {
      rvalue = false;
   }

   // Deallocate arrays:
   zoltan->LB_Free_Part(&importGlobalIDs,&importLocalIDs,&importProcesses,&importParts);
   zoltan->LB_Free_Part(&exportGlobalIDs,&exportLocalIDs,&exportProcesses,&exportParts);
   return rvalue;
}

template<class C> std::string ParGrid<C>::loadBalanceMethod(const LBM& method) const {
   switch (method) {
    case Block:
      return "BLOCK";
      break;
    case Random:
      return "RANDOM";
      break;
    case RCB:
      return "RCB";
      break;
    case RIB:
      return "RIB";
      break;
    case HSFC:
      return "HSFC";
      break;
    case Graph:
      return "GRAPH";
      break;
   }
}

template<class C> C* ParGrid<C>::operator[](const ID::type& id) const {
   typename std::map<ID::type,ParCell<C> >::const_iterator it = localCells.find(id);
   if (it != localCells.end()) return it->second.dataptr;
   it = remoteCells.find(id);
   if (it != remoteCells.end()) return it->second.dataptr;
   return NULL;
}

template<class C> void ParGrid<C>::print() const {
   std::cerr << "ParGrid:" << std::endl;
   std::cerr << "\tMPI reports " << N_processes << " processes." << std::endl;
   std::cerr << "\tMPI says my rank is " << myrank << std::endl;
}

template<class C> int ParGrid<C>::rank() const {return myrank;}

/** Synchronizes the information of cells-to-processes assignments among all 
 * MPI processes. Upon successful completion of this function, the member variable 
 * hostProcesses contains for each global ID the rank of the process which has that 
 * cell.
 */
template<class C> void ParGrid<C>::syncCellAssignments() {
   // First gather the number of cells on each process:
   int* cellsPerProcess = new int[N_processes];
   uint N_mycells = localCells.size();
   MPI_Allgather(&N_mycells,1,MPI_UNSIGNED,cellsPerProcess,1,MPI_UNSIGNED,MPI_COMM_WORLD);
   MPI_Barrier(MPI_COMM_WORLD); // May not be necessary
   
   // Count the total number of cells:
   uint sum = 0;
   for (uint i=0; i<N_processes; ++i) sum += cellsPerProcess[i];

   // Write the global IDs of the cells on this process to send buffer:
   uint* myCells = new uint[N_mycells];
   uint i=0; 
   for (typename std::map<ID::type,ParCell<C> >::const_iterator it=localCells.begin(); it!=localCells.end(); ++it) {
      myCells[i] = it->first;
      ++i;
   }
   
   // Create a receive buffer:
   uint* allCells = new uint[sum];
   int* offsets = new int[N_processes];
   offsets[0] = 0;
   for (i=1; i<N_processes; ++i) offsets[i] = offsets[i-1] + cellsPerProcess[i-1];
   
   // Gather cell assignments:
   MPI_Allgatherv(myCells,N_mycells,MPI_UNSIGNED,allCells,cellsPerProcess,offsets,MPI_UNSIGNED,MPI_COMM_WORLD);
   hostProcesses.clear();
   uint process = 0;
   uint counter = 0;
   while (process < N_processes) {
      for (i=0; i<cellsPerProcess[process]; ++i) {
	 hostProcesses[allCells[counter]] = process;
	 ++counter;
      }
      ++process;
   }
   delete offsets;
   delete allCells;
   delete myCells;
   delete cellsPerProcess;
}

template<class C> void ParGrid<C>::syncCellCoordinates() {
   #ifndef NDEBUG
   if (remoteCells.size() != receiveList.size()) {
      std::cerr << "ParGrid::syncCellCoordinates remoteCells.size() = " << remoteCells.size();
      std::cerr << " receiveList.size() = " << receiveList.size() << std::endl;
      exit(1);
   }
   #endif
   
   // Here we send other MPI processes the coordinates of each cell.
   // First construct an appropriate MPI datatype.
   //MPI_Datatype CellType;
   MPI_Datatype type[6] = {MPI_UNSIGNED,MPI_UNSIGNED,MPI_UNSIGNED,MPI_Type<Real>(),MPI_Type<Real>(),MPI_Type<Real>()};
   int blockLen[6] = {1,1,1,1,1,1};
   MPI_Aint disp[6]; // Byte displacements of each variable
   disp[0] = 0;
   disp[1] = 4;
   disp[2] = 8;
   disp[3] = 12;
   disp[4] = 16;
   disp[5] = 20;
   
   //int result = MPI_Type_create_struct(6,blockLen,disp,type,&CellType);
   int result = MPI_Type_create_struct(6,blockLen,disp,type,&MPIdataType);
   #ifndef NDEBUG
   if (result != MPI_SUCCESS) {std::cerr << "ParGrid::syncCellCoordinates MPI_Type_create_struct failed!" << std::endl;}
   #endif
   
   //result = MPI_Type_commit(&CellType);
   result = MPI_Type_commit(&MPIdataType);
   #ifndef NDEBUG
   if (result != MPI_SUCCESS) {std::cerr << "ParGrid::syncCellCoordinates MPI_Type_commit failed!" << std::endl;}
   #endif
   
   // Create an array for the MPI requests. Make it large enough for both sends and receives:
   if (MPIrequests.size() < receiveList.size()) MPIrequests.resize(receiveList.size());
   if (MPIstatuses.size() < receiveList.size()) MPIstatuses.resize(receiveList.size());
   // Post receives for each remote cell, then sends for each boundary cell:
   uint counter = 0;
   for (std::map<ID::type,int>::const_iterator it=receiveList.begin(); it!=receiveList.end(); ++it) {
      void* const buffer = &(remoteCells[it->first]); // Starting address of receive buffer
      const int count = 1;                            // Number of elements in receive buffer
      const int source = it->second;                  // Rank of source MPI process
      const int tag = it->first;                      // Message tag
      MPI_Irecv(buffer,count,MPIdataType,source,tag,MPI_COMM_WORLD,&(MPIrequests[counter]));
      //MPI_Irecv(buffer,count,CellType,source,tag,MPI_COMM_WORLD,&(MPIrequests[counter]));
      ++counter;
   }
   MPI_Barrier(MPI_COMM_WORLD);

   for (std::map<std::pair<ID::type,int>,char>::const_iterator it=sendList.begin(); it!=sendList.end(); ++it) {
      void* const buffer = &(localCells[it->first.first]); // Starting address of send buffer
      const int count = 1;                                 // Number of elements in send buffer
      const int dest = it->first.second;                   // Rank of destination MPI process
      const int tag = it->first.first;                     // Message tag
      MPI_Request dummyRequest;
      MPI_Issend(buffer,count,MPIdataType,dest,tag,MPI_COMM_WORLD,&dummyRequest);
      //MPI_Issend(buffer,count,CellType,dest,tag,MPI_COMM_WORLD,&dummyRequest);
      MPI_Request_free(&dummyRequest);
   }
   // Wait until all data has been received and deallocate memory:
   waitAll();
}

template<class C> bool ParGrid<C>::waitAll() {
   bool rvalue = true;
   MPI_Waitall(MPIrequests.size(),&(MPIrequests[0]),&(MPIstatuses[0]));
   #ifndef NDEBUG
   for (uint i=0; i<MPIstatuses.size(); ++i) if (MPIstatuses[i].MPI_ERROR != MPI_SUCCESS) rvalue=false;
   #endif
   
   // Free memory allocated for the transmission:
   for (uint i=0; i<MPIrequests.size(); ++i)
     if (MPI_Request_free(&(MPIrequests[i])) != MPI_SUCCESS) rvalue=false;
   MPI_Type_free(&MPIdataType);
   MPIrequests.clear();
   MPIstatuses.clear();
   return rvalue;
}

// *************************************************************
// ************************ ITERATORS ************************** 
// *************************************************************

template<class C> typename ParGrid<C>::iterator ParGrid<C>::begin() {
   ParGrid<C>::iterator i;
   i.set(localCells.begin());
   return i;
}

template<class C> typename ParGrid<C>::iterator ParGrid<C>::boundaryCellsBegin() {
   ParGrid<C>::iterator i;
   i.allCells = false;
   i.iteratingInner = false;
   i.it = localCells.begin();
   // Set the iterator to point to first cell with remote neighbours:
   while (i.it->second.hasRemotes == false && i.it != localCells.end()) {
      ++(i.it);
   }
   return i;
}

template<class C> typename ParGrid<C>::iterator ParGrid<C>::innerCellsBegin() {
   ParGrid<C>::iterator i;
   i.allCells = false;
   i.iteratingInner = true;
   // Set the iterator to point to a first cell without remote neighbours:
   while (i.it->second.hasRemotes == true && i.it != localCells.end()) {
      ++(i.it);
   }
   return i;
}

template<class C> typename ParGrid<C>::iterator ParGrid<C>::end() {
   ParGrid<C>::iterator i;
   i.set(localCells.end());
   return i;
}

template<class C> ParGrid<C>::iterator::iterator(const bool& allCells,const bool& iteratingInner): 
allCells(allCells),iteratingInner(iteratingInner) {
   it = localCells.end();
}

template<class C> typename ParGrid<C>::iterator& ParGrid<C>::iterator::operator++() {
   ++it;
   if (allCells == false) {
      if (iteratingInner == true)
	while (it != localCells.end() && it->second.hasRemotes == true) ++it;
      else 
	while (it != localCells.end() && it->second.hasRemotes == false) ++it; 
   }
   return *this;
}
/*
template<class C> void ParGrid<C>::set(typename std::map<ID::type,ParCell<C> >::iterator& i) {
   it = i;
   return *this;
}*/

template<class C> bool ParGrid<C>::iterator::operator==(const ParGrid<C>::iterator& x) {
   return it == x.it;
}

template<class C> bool ParGrid<C>::iterator::operator!=(const ParGrid<C>::iterator& x) {
   return it != x.it;
}

// *************************************************************
// ***************** ZOLTAN CALLBACK FUNCTIONS *****************
// *************************************************************

// GENERIC CALLBACK FUNCTIONS
// The functions below are required for everything.

/** Definition for Zoltan callback function ZOLTAN_NUM_OBJ_FN. This function 
 * is required to use Zoltan. The purpose is to tell Zoltan how many cells 
 * are currently assigned to this process.
 * @param parGridPtr A pointer to ParGrid.
 * @param rcode The return code. Upon success this should be ZOLTAN_OK.
 * @return The number of cells assigned to this process.
 */
template<class C>
int ParGrid<C>::getNumberOfLocalCells(void* parGridPtr,int* rcode) {     
   ParGrid<C>* ptr = reinterpret_cast<ParGrid<C>*>(parGridPtr);
   *rcode = ZOLTAN_OK;
   return ptr->localCells.size();
}

/** Definition for Zoltan callback function ZOLTAN_OBJ_LIST_FN. This function 
 * is required to use Zoltan. The purpose is to tell Zoltan the global and local 
 * IDs of the cells assigned to this process, as well as their weights.
 * @param parGridPtr A pointer to ParGrid.
 * @param N_globalIDs The number of array entries used to describe one global ID.
 * @param N_localIDs The number of array entries used to describe one local ID.
 * @param globalIDs An array which is to be filled with the global IDs of the cells 
 * currently assigned to this process. This array has been allocated by Zoltan.
 * @param localIDs An array which is to be filled with the local IDs of the cells 
 * currently assigned to this process. This array has been allocated by Zoltan.
 * @param N_weights
 * @param cellWeights An array which is to be filled with the cell weights.
 * @param rcode The return code. Upon success this should be ZOLTAN_OK.
 */
template<class C>
void ParGrid<C>::getLocalCellList(void* parGridPtr,int N_globalIDs,int N_localIDs,ZOLTAN_ID_PTR globalIDs,
				  ZOLTAN_ID_PTR localIDs,int N_weights,float* cellWeights,int* rcode) {
   ParGrid<C>* parGrid = reinterpret_cast<ParGrid<C>*>(parGridPtr);

   int i=0;
   //std::map<ID::type,ParCell<C> >::const_iterator it = parGrid->localCells.begin();
   for (typename std::map<ID::type,ParCell<C> >::const_iterator it = parGrid->localCells.begin(); it!=localCells.end(); ++it) {
      globalIDs[i] = it->first;
      ++i;
   }   
   *rcode = ZOLTAN_OK;
}


// GEOMETRY-BASED LOAD BALANCING:                                                          
// The functions below are for geometry-based load balancing
// functions (RCB,RIB,HSFC,Reftree).                         
// -------------------------------------------------------------- 

/** Definition for Zoltan callback function ZOLTAN_NUM_GEOM_FN. This function is 
 * required for geometry-based load balancing. The purpose is 
 * to tell Zoltan the dimensionality of the grid. Here we use three-dimensional 
 * grid.
 * @param parGridPtr A pointer to ParGrid.
 * @param rcode The return code. Upon success this should be ZOLTAN_OK.
 * @return The number of physical dimensions.
 */
template<class C>
int ParGrid<C>::getGridDimensions(void* parGridPtr,int* rcode) {   
   *rcode = ZOLTAN_OK;
   return 3;
}

/** Definition for Zoltan callback function ZOLTAN_GEOM_FN. This function is required for
 * geometry-based load balancing. The purpose of this function is to tell 
 * Zoltan the physical x/y/z coordinates of a given cell.
 * @param parGridPtr Pointer to ParGrid.
 * @param N_globalEntries The size of array globalID.
 * @param N_localEntries The size of array localID.
 * @param globalID The global ID of the cell whose coordinates are queried.
 * @param localID The local ID of the cell whose coordinates are queried.
 * @param geometryData Array where the coordinates should be written. Zoltan has reserved this array, its size 
 * is determined by the getGridDimensions function.
 * @param rcode The return code. Upon success this should be ZOLTAN_OK.
 */
template<class C> 
void ParGrid<C>::getCellCoordinates(void* parGridPtr,int N_globalEntries,int N_localEntries,ZOLTAN_ID_PTR globalID,
				    ZOLTAN_ID_PTR localID,double* geometryData,int* rcode) {
   ParGrid<C>* parGrid = reinterpret_cast<ParGrid<C>*>(parGridPtr);

   // Try to find the given cell:
   typename std::map<ID::type,ParCell<C> >::const_iterator it = parGrid->localCells.find(globalID[0]);
   #ifndef NDEBUG
   if (it == parGrid->localCells.end()) {
      *rcode = ZOLTAN_FATAL;
      std::cerr << "ParGrid getCellCoordinates queried non-existing local cell." << std::endl;
      return;
   }
   #endif
   geometryData[0] = (it->second).xcrd;
   geometryData[1] = (it->second).ycrd;
   geometryData[2] = (it->second).zcrd;
   rcode = ZOLTAN_OK;
}

// GRAPH-BASED LOAD BALANCING

/** Definition for Zoltan callback function ZOLTAN_NUM_EDGES_FN. This function is required 
 * for graph-based load balancing. The purpose is to tell how many edges a given cell has, i.e. 
 * how many neighbours it has to share data with.
 * @param parGridPtr Pointer to ParGrid.
 * @param N_globalIDs The size of array globalID.
 * @param N_localIDs The size of array localID.
 * @param globalID The global ID of the cell whose edges are queried.
 * @param localID The local ID of the cell whose edges are queried.
 * @param rcode The return code. Upon success this should be ZOLTAN_OK.
 * @return The number of edges the cell has. For three-dimensional box grid this is between 3 and 6, 
 * depending on if the cell is on the edge of the simulation volume.
 */
template<class C>
int ParGrid<C>::getNumberOfEdges(void* parGridPtr,int N_globalIDs,int N_localIDs,ZOLTAN_ID_PTR globalID,
		     ZOLTAN_ID_PTR localID,int* rcode) {

   ParGrid<C>* parGrid = reinterpret_cast<ParGrid<C>*>(parGridPtr);
   typename std::map<ID::type,ParCell<C> >::const_iterator it = localCells.find(globalID[0]);
   #ifndef NDEBUG
   if (it == localCells.end()) {
      *rcode = ZOLTAN_FATAL;
      std::cerr << "ParGrid::getNumberOfEdges Zoltan tried to query nonexisting cell!" << std::endl;
      return 0;
   }
   #endif
   // Calculate the number of edges the cell has:
   int edges = 0;
   for (int i=0; i<6; ++i) 
     if (it->second.neighbours[i] != std::numeric_limits<ID::type>::max()) ++edges;
   
   *rcode = ZOLTAN_OK;
   return edges;
}

/** Definition for Zoltan callback function ZOLTAN_EDGE_LIST_FN. This function is required 
 * for graph-based load balancing. The purpose is to give the global IDs of each neighbour of 
 * a given cell, as well as the ranks of the MPI processes which have the neighbouring cells.
 * MAKE SURE THAT hostProcesses IS UP TO DATE BEFORE CALLING THIS!
 * @param parGridPtr A pointer to ParGrid.
 * @param N_globalIDs The size of array globalID.
 * @param N_localIDs The size of array localID.
 * @param globalID The global ID of the cell whose neighbours are queried.
 * @param localID The local ID of the cell whose neighbours are queried.
 * @param nbrGlobalIDs An array where the global IDs of the neighbours are written. Note that
 * Zoltan has already allocated this array based on a call to getNumberOfEdges function.
 * @param nbrHosts For each neighbour, the rank of the MPI process which has the cell.
 * @param N_weights The size of array weight.
 * @param weight The weight of each edge.
 * @param rcode The return code. Upon success should be ZOLTAN_OK.
 */
template<class C>
void ParGrid<C>::getEdgeList(void* parGridPtr,int N_globalIDs,int N_localIDs,ZOLTAN_ID_PTR globalID,
			     ZOLTAN_ID_PTR localID,ZOLTAN_ID_PTR nbrGlobalIDs,int* nbrHosts,
			    int N_weights,float* weight,int* rcode) {
   ParGrid<C>* parGrid = reinterpret_cast<ParGrid<C>*>(parGridPtr);
   typename std::map<ID::type,ParCell<C> >::const_iterator it = localCells.find(globalID[0]);
   #ifndef NDEBUG
   if (it == localCells.end()) {
      *rcode = ZOLTAN_FATAL;
      std::cerr << "ParGrid::getEdgeList Zoltan tried to query nonexisting cell!" << std::endl;
      return;
   }
   #endif
   int index = 0;
   for (int i=0; i<6; ++i) {
      if (it->second.neighbours[i] != std::numeric_limits<ID::type>::max()) {
	 nbrGlobalIDs[index] = it->second.neighbours[i];
	 nbrHosts[index] = hostProcesses[it->second.neighbours[i]];
	 ++index;
      }
   }
   *rcode = ZOLTAN_OK;
}











