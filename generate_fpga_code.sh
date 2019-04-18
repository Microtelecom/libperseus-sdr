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
// Author:              (Automatically generated)
// ------------------------------------------------------------------------------



//
// Automatically generated at $dt
// Please don't manually change this file !!!
//
#include <fpga_data.h>

ZZZ_PRELUDE_END

RBS=$1/'*.rbs'


N_FPGA=0
#
# generate the FPGA data arrays
#
for x in $RBS
do
  # compute file length
  if [[ "$OSTYPE" == "darwin"* ]]; then
    file_size=$(stat -f%k "$x")
  else
    file_size=$(stat -c%s "$x")
  fi

  
  ### ->  # removes the shortest match from the beginning
  ### ->  ## removes the longest match from the beginning
  ### ->  % removes the shortest match from the end
  ### ->  %% removes the longest match from the end

  
  # file name without path
  file=${x##*/}

  speed=${file#perseus}
  speed2=${speed%%v$RBS}

  mega=${speed2%%m*}
  kilo=${speed2%%k*}

  tag=""
  
  if [ -n "$( echo $mega | sed 's/^[+-]//;s/[0-9]//g;s/\.//;s/d//' )" ] ; then
     
    if [ -n "$( echo $kilo | sed 's/^[+-]//;s/[0-9]//g;s/\.//' )" ] ; then
		echo "FATAL: UNEXPECTED: $kilo"
        exit 255
    else
        tag=$(expr $kilo \* 1000)
    fi
	 
  else
	### search for a 'd' embedded
	if [ -n "$( echo $mega | sed 's/^[+-]//;s/[0-9]//g;s/\.//' )" ] ; then
		dd=${mega#*d}
		ii=${mega%d*}
	    tag=$(expr $ii \* 1000000 + $dd \* 100000 )
	else	
		tag=$(expr $mega \* 1000000)
	fi
  fi
  
  echo "////********** $file: s: $speed s2: $speed2 M: $mega K: $kilo $tag"

  echo "//// [$x]"
  echo "const unsigned char fpga_data_$tag[$file_size] = "
  echo "  { "

  # produce the data in C format
  cat "$x" | xxd -i
  echo "  };"
  echo "//// end of $file"
  N_FPGA=$(( $N_FPGA + 1 ))
done




#
# Generate the table (sorted by speed)
#

cat << YYY_END

FpgaImages fpgaImgTbl [] = {

YYY_END


echo -n "" > tmpfile

for x in $RBS
do
  if [[ "$OSTYPE" == "darwin"* ]]; then
    file_size=$(stat -f%k "$x")
  else
    file_size=$(stat -c%s "$x")
  fi

  file=${x##*/}

  speed=${file#perseus}
  speed2=${speed%%v$RBS}

  mega=${speed2%%m*}
  kilo=${speed2%%k*}

  tag=""

  if [ -n "$( echo $mega | sed 's/^[+-]//;s/[0-9]//g;s/\.//;s/d//' )" ] ; then
     
    if [ -n "$( echo $kilo | sed 's/^[+-]//;s/[0-9]//g;s/\.//' )" ] ; then
 		echo "FATAL: UNEXPECTED: $kilo"
        exit 255
     else
        tag=$(expr $kilo \* 1000)
    fi

  else
	### search for a 'd' embedded
	if [ -n "$( echo $mega | sed 's/^[+-]//;s/[0-9]//g;s/\.//' )" ] ; then
		dd=${mega#*d}
		ii=${mega%d*}
	    tag=$(expr $ii \* 1000000 + $dd \* 100000 )
	else	
		tag=$(expr $mega \* 1000000)
	fi
  fi
  
  ##echo "********** $file: $speed: $speed2: M: $mega K: $kilo"
  echo "$tag $file $file_size $x" >> tmpfile

done

#
# sort
#
cat tmpfile | sort -g > tmpfile2


while read SPEED FN SIZE FN2
do
   echo "////  >>>>$SPEED<>$FN<>$SIZE<<<<<<"

   obj_name=$( echo $FN | sed 's/[\.]/_/' )

   echo "{"
   echo "    \"$FN\",    "
   echo "    $SPEED,        "
   echo "    $SIZE,  "
   echo "    fpga_data_$SPEED,"
   echo "    sizeof( fpga_data_$SPEED )"
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

/*
 * Compile and test with the following command:
 *
 * gcc -D__TEST_MODULE__ -I. fpga_data.c -o fpga_data_test && ./fpga_data_test
 */

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
      fprintf (stderr, "name: %20.20s\tspeed: %10.d\tsize: %10.d\tsizeof: %10.d\n",
               fpgaImgTbl [i].name, fpgaImgTbl [i].speed, fpgaImgTbl [i].size, fpgaImgTbl [i].osize
              );
      if (fpgaImgTbl [i].size != fpgaImgTbl[i].osize)
          fprintf (stderr, "FATAL error: declared size %d != from real size: %d \n", fpgaImgTbl [i].size, fpgaImgTbl [i].osize ); 
   }
   return 0;
}
#endif
XXXXX_END

rm -f tmpfile tmpfile2






