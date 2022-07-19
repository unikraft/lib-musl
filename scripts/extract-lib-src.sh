#! /bin/bash

LIB=$1
ARCH=$2
MUSL=$3

if [[ "$MUSL" == "" ]]; then
	MUSL=$(pwd)
fi

if [[ ! "$ARCH" =~ aarch64|arm|i386|microblaze|mips|mips64|mipsn32|or1k|powerpc|powerpc64|s390x|sh|x32|x86_64|riscv64 ]]; then
	echo "Unknown architecture: $ARCH" 1>&2
	ARCH=""
fi

if [[ ! -d "$MUSL/src/$LIB" ]]; then
	echo "Can not find library directory: $MUSL/$LIB" 1>&2
	LIB=""
fi

if [[ "$LIB" == "" || "$ARCH" == "" ]]; then
	echo "USAGE: ./extract-lib-src.sh <LIBRARY_NAME> <ARCHITECTURE> [PATH_TO_MUSL]" 1>&2
	exit 1
fi

pushd "$MUSL" > /dev/null || exit
BASE_SRC=$(find src/$LIB -maxdepth 1 -name '*.c' | sort)
if [[ -d "src/$LIB/$ARCH" ]]; then
	ARCH_SRC=$(find src/$LIB/$ARCH -maxdepth 1 -name '*.[csS]' | sort)
else
	ARCH_SRC=""
fi
popd > /dev/null || exit

BASE_SUB_SRC=$(sed -r "s%/$ARCH/([^ ]*)\\.[csS]%/\\1.c%g" <(echo $ARCH_SRC))

echo "# reset sources proviously added"
echo "LIBMUSL_${LIB^^}_SRCS-y ="
echo "# base sources (skips the ones replaced by $ARCH)"
for src in $BASE_SRC; do
	if [[ "$BASE_SUB_SRC" =~ $src ]] ; then
		echo "#LIBMUSL_${LIB^^}_SRCS-y += \$(LIBMUSL)/$src"
	else
		echo "LIBMUSL_${LIB^^}_SRCS-y += \$(LIBMUSL)/$src"
	fi
done

if [[ "$ARCH_SRC" != "" ]]; then
	echo "# $ARCH specific sources"
	for src in $ARCH_SRC; do
		echo "LIBMUSL_${LIB^^}_SRCS-y += \$(LIBMUSL)/$src|$ARCH"
	done
else
	echo "# no $ARCH specific sources"
fi

