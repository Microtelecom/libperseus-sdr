#!/bin/sh

# ------------------------------------------------------------------------------
# Perseus SDR Library Interface
# 
# Copyright (c) 2010 Nicolangelo Palermo / IV3NWV 
# This file is part of the Perseus SDR Library
#
# The Perseus SDR Library is free software; you can redistribute 
# it and/or modify it under the terms of the GNU Lesser General Public 
# License as published by the Free Software Foundation; either version 
# 2.1 of the License, or (at your option) any later version.
#
# The Perseus SDR Library is distributed in the hope that it will
# be useful, but WITHOUT ANY WARRANTY; without even the implied warranty 
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with the Perseus SDR Library; 
# if not, see <http://www.gnu.org/licenses/>.
#
# Creation date:	25 June 2010
# Author: 			Andrea Montefusco (IW0HDV)
#						(andrew at montefusco dot com)
# ------------------------------------------------------------------------------

if [ -e "Makefile" ]
then
  make distclean
fi
bash ./cleanup.sh

autoreconf -i


#
# On old system, try to submit the following commands (without leading #)
#
# rm -fr config.cache autom4te*.cache config.guess  config.sub ltmain.sh
# aclocal
# automake --add-missing
# autoconf
# autoreconf --install

