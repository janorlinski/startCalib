#!/bin/sh
###############################################################################
#
#  Hades Software Environment Setup Script (Bourne shell family version)
#
#  Each Software Collection should have it dedicated verion of this script.
#  They can be distinguished e.g. by the used paths.
#
#  Author: Simon Lang, GSI, 30.09.2006
#
###############################################################################

. /cvmfs/hades.gsi.de/install/admin/hsc-functions.sh

hsc_checkContext

# Root, and dependent packages
export ROOTSYS=/cvmfs/hades.gsi.de/install/root-6.24.02

# Global Hydra Location
export HADDIR=/cvmfs/hades.gsi.de/install/6.24.02/hydra2-6.5
export ROOT_INCLUDE_PATH=${HADDIR}/include

# Private Hydra Location - not used by default
#export MYHADDIR=/e.g./somewhere/in/my/home/directory
#export ROOT_INCLUDE_PATH=${MYHADDIR}/include:${HADDIR}/include

# Oracle
export ORACLE_HOME=/cvmfs/it.gsi.de/oracle/product/12.1.2/client_x86_64_1
export ORA_USER=hades_ana/hades@db-hades

# CERNLIB - for HGeant
export CERN_ROOT=/cvmfs/hades.gsi.de/install/cernlib_gfortran/2005

# PLUTO
export PLUTODIR=

# RFIO support
export ADSM_BASE_NEW=/misc/hadessoftware/etch32/install/gstore-may07

hsc_setEnvironment
hsc_shortPrintout

hsc_finalizeScript
