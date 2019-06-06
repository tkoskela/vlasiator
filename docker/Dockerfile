FROM ubuntu:18.04
MAINTAINER Tuomas Koskela (tuomas.koskela@helsinki.fi)

#gcc, make, OpenMPI, wget, git, silo, eigen, boost
RUN apt-get -y update && apt-get install -y gcc g++ make unzip libopenmpi-dev wget git libeigen3-dev libboost-dev libboost-program-options-dev libsilo-dev gawk vim-common emacs

RUN useradd vlasov

ENV LIB /home/lib
ENV VLAS /home/vlasiator

#DCCRG
WORKDIR $LIB
RUN git clone https://github.com/fmihpc/dccrg.git
WORKDIR dccrg
RUN git checkout 10eed23fe2d0eea8ca3724f24b68bcba7783aa4b

#Fsgrid
WORKDIR $LIB
RUN git clone https://github.com/fmihpc/fsgrid.git

#Phiprof
WORKDIR $LIB
ADD https://github.com/fmihpc/phiprof/archive/v2.0-beta.tar.gz .
RUN tar xf v2.0-beta.tar.gz
WORKDIR $LIB/phiprof-2.0-beta/src
RUN make CCC=mpic++

#Vlsv
WORKDIR $LIB/
RUN git clone https://github.com/fmihpc/vlsv.git
WORKDIR vlsv
RUN make

#Vectorclass
WORKDIR $LIB
ADD https://www.agner.org/optimize/vectorclass.zip .
WORKDIR vectorclass
RUN unzip ../vectorclass.zip
RUN unzip special.zip

#Papi
WORKDIR $LIB
ADD http://icl.cs.utk.edu/projects/papi/downloads/papi-5.4.3.tar.gz .
RUN tar xf papi-5.4.3.tar.gz
WORKDIR papi-5.4.3/src
RUN ./configure --prefix=$LIB/papi && make CC=gcc && make install
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"$LIB/papi/lib"

#Jemalloc
WORKDIR $LIB
ADD https://github.com/jemalloc/jemalloc/releases/download/5.2.0/jemalloc-5.2.0.tar.bz2 .
RUN tar xf jemalloc-5.2.0.tar.bz2
WORKDIR jemalloc-5.2.0
RUN ./configure --prefix=$LIB/jemalloc --with-jemalloc-prefix=je_ && make && make install
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"$LIB/jemalloc/lib"

#Zoltan
WORKDIR $LIB
ADD http://www.cs.sandia.gov/~kddevin/Zoltan_Distributions/zoltan_distrib_v3.83.tar.gz .
RUN tar xf zoltan_distrib_v3.83.tar.gz
WORKDIR zoltan-build
RUN ../Zoltan_v3.83/configure --prefix=$LIB/zoltan --enable-mpi --with-mpi-compilers --with-gnumake --with-id-type=ullong && make && make install


WORKDIR $VLAS
RUN git clone https://github.com/tkoskela/vlasiator.git
WORKDIR vlasiator
#RUN git checkout dev
ENV VLASIATOR_ARCH docker
RUN make
