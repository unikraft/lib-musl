#!/bin/bash
# Generate list of MUSL headers for a given library
#
# Authors: Alexander Jung <alexander.jung@neclab.eu>
#
# Copyright (c) 2020, NEC Europe Ltd., NEC Corporation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

MUSL_SRC=$1
MUSL_LIB=$2

if [ "x$MUSL_SRC" == "x" ]; then
  echo "Usage: $0 [MUSL_SRC] [LIBRARY]"
  exit 1
elif [ ! -d $MUSL_SRC/src ]; then
  echo "Could not find: $MUSL_SRC"
  exit 1
elif [ ! "x$MUSL_LIB" == "x" ] && [ ! -d $MUSL_SRC/src/$MUSL_LIB ]; then
  echo "Could not find: $MUSL_SRC/src/$MUSL_LIB"
  exit 1
fi

LIB_HDRS=()

FILES=$(find $MUSL_SRC/src/$MUSL_LIB/ -type f -name '*.c')

for FILE in $FILES; do
  FILE_HDRS=$(grep -oP '#\s*include\s*[<"]\K[\w\/]+.h' $FILE)
  LIB_HDRS+=($FILE_HDRS)
done

# Remove duplicates
IFS=$'\n' LIB_HDRS=($(sort -u <<<"${LIB_HDRS[*]}"))
unset IFS

for HDR in "${LIB_HDRS[@]}"; do
  if [ -f $MUSL_SRC/include/$HDR ]; then
    echo "LIBMUSL_${MUSL_LIB^^}_HDRS-y += \$(LIBMUSL)/include/$HDR"
  elif [ -f $MUSL_SRC/src/internal/$HDR ]; then
    echo "LIBMUSL_${MUSL_LIB^^}_HDRS-y += \$(LIBMUSL)/src/internal/$HDR"
  else
    echo "# LIBMUSL_${MUSL_LIB^^}_HDRS-y += $HDR # not found"
  fi
done
