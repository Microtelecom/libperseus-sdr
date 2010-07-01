#!/bin/bash

dt=$(date)

#
# Emit prelude code
#

cat << ZZZ_PRELUDE_END
// ------------------------------------------------------------------------------
// Perseus Receiver FPGA code
//
// Copyright (c) 2010 Nicolangelo Palermo / IV3NWV
// This file is part of the Perseus SDR Library
//
// The Perseus SDR Library is free software; you can redistribute
// it and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either version
// 2.1 of the License, or (at your option) any later version.

// The Perseus SDR Library is distributed in the hope that it will
// be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with the Perseus SDR Library;
// if not, see <http://www.gnu.org/licenses/>.

// Creation date:       $dt
// Version:             N/A
// Author:              N/A (Automatically generated)
// ------------------------------------------------------------------------------



//
// Automatically generated at $dt
// Please don't manually change this file !!!
//
#include "fpga_data.h"

ZZZ_PRELUDE_END




N_FPGA=0
#
# generate the FPGA data arrays
#
for x in *.rbs
do
  file_size=$(stat -c%s "$x")

  file=${x##*/}

  speed=${file#perseus}
  speed2=${speed%%v*.rbs}

  mega=${speed2%%m*}
  kilo=${speed2%%k*}

  tag=""

  if [ -n "$( echo $mega | sed 's/^[+-]//;s/[0-9]//g;s/\.//' )" ] ; then
     
     if [ -n "$( echo $kilo | sed 's/^[+-]//;s/[0-9]//g;s/\.//' )" ] ; then
         exit 255
     else
         tag=$(expr $kilo \* 1000)
     fi


  else
     tag=$(expr $mega \* 1000000)
  fi
#  echo "********** $file: $speed: $speed2: M: $mega K: $kilo"

  echo "///// $tag"

  echo ""
  echo "const unsigned char fpga_data_$tag[$file_size] = "
  echo "  { "
  #echo "     0x00, 0x00,"
  /usr/bin/hexdump -v -e '1/1 "0x%02x, "' -e '"\n"' "./$x"
  echo "  };"
  echo "////"
  N_FPGA=$(( $N_FPGA + 1 ))
done




#
# Generate the table (sorted by speed)
#

cat << YYY_END

FpgaImages fpgaImgTbl [] = {

YYY_END


echo -n "" > tmpfile

for x in *.rbs
do
  file_size=$(stat -c%s "$x")

  file=${x##*/}

  speed=${file#perseus}
  speed2=${speed%%v*.rbs}

  mega=${speed2%%m*}
  kilo=${speed2%%k*}

  tag=""

  if [ -n "$( echo $mega | sed 's/^[+-]//;s/[0-9]//g;s/\.//' )" ] ; then
     
     if [ -n "$( echo $kilo | sed 's/^[+-]//;s/[0-9]//g;s/\.//' )" ] ; then
         exit 255
     else
         tag=$(expr $kilo \* 1000)
     fi


  else
     tag=$(expr $mega \* 1000000)
  fi
  
  #####echo "********** $file: $speed: $speed2: M: $mega K: $kilo"
  echo "$tag $file $file_size $x" >> tmpfile

done

#
# sort
#
cat tmpfile | sort -g > tmpfile2


while read SPEED FN SIZE FN2
do
   # echo ">>>>$SPEED<>$FN<>$SIZE<<<<<<"

   echo "{"
   echo "    \"$FN\",    "
   echo "    $SPEED,        "
   echo "    $SIZE,  "
   echo "    fpga_data_$SPEED,"
   echo "},  " 
   echo "////"

done < tmpfile2



cat << XXXX_END
}; // end of table *******

XXXX_END

#emit table size
echo "int nFpgaImages = $N_FPGA;"


# emit trailers
cat << XXXXX_END

#if defined __TEST_MODULE__
#include <stdio.h>
#include <assert.h>

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0])) 
int main (void)
{
   int i;

   assert(ARRAYSIZE(fpgaImgTbl)==nFpgaImages);

   fprintf (stderr, "Cores available: %d\n\n", nFpgaImages);

   for (i=0; i < ARRAYSIZE(fpgaImgTbl); ++i) {
      fprintf (stderr, "name: %20.20s\tspeed: %10.d\tsize: %10.d\n",
               fpgaImgTbl [i].name, fpgaImgTbl [i].speed, fpgaImgTbl [i].size
              );
   }
   return 0;
}
#endif
XXXXX_END






