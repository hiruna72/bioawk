#!/bin/sh

# for armeabi-v7a and arm64-v8a cross compiling
toolchain_file="/media/shan/OS/Hiruna/temp-old/android-sdk-linux/ndk-bundle/build/cmake/android.toolchain.cmake"


touch bioawkmisc.h
echo "#include <sys/resource.h>
#include <sys/time.h>
static inline double realtime(void) {
    struct timeval tp;
    //struct timezone tzp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec + tp.tv_usec * 1e-6;
}
// taken from minimap2/misc
static inline double cputime(void) {
    struct rusage r;
    getrusage(RUSAGE_SELF, &r);
    return r.ru_utime.tv_sec + r.ru_stime.tv_sec +
           1e-6 * (r.ru_utime.tv_usec + r.ru_stime.tv_usec);
}
//taken from minimap2
static inline long peakrss(void)
{
    struct rusage r;
    getrusage(RUSAGE_SELF, &r);
#ifdef __linux__
    return r.ru_maxrss * 1024;
#else
    return r.ru_maxrss;
#endif
}" > bioawkmisc.h

# to create a bioawk library
cp main.c tempmain
## sed -i 's/int main(int argc/int init_samtools(int argc/g' bamtk.c
sed -i ':a;N;$!ba;s/int main(int argc, char \*argv\[\])\n{/#include "bioawkmisc.h"\n#include "interface_bioawk.h"\nint init_bioawk(int argc, char *argv[])\n{\n\tdouble realtime0 = realtime();/g' main.c
sed -i 's+return(errorflag);+fprintf(stderr,"[%s] Real time: %.3f sec; CPU time: %.3f sec; Peak RAM: %.3f GB\\n\\n",\n\t\t__func__, realtime() - realtime0, cputime(),peakrss() / 1024.0 / 1024.0 / 1024.0);\n\treturn(errorflag);+g' main.c
##sed -i 's/#include "version.h"/#include "version.h"\n#include "samtoolmisc.h"/g' bamtk.c
#
touch interface_bioawk.h
echo "#ifdef __cplusplus
extern "C" {
#endif
#ifndef INTERFACE_BIOAWK_H
#define INTERFACE_BIOAWK_H

int init_bioawk(int argc, char *argv[]);

#endif
#ifdef __cplusplus
}
#endif" > interface_bioawk.h


mkdir -p build
rm -rf build
mkdir build
cd build

# for architecture x86 
# cmake .. -DDEPLOY_PLATFORM=x86
# make -j 8

# # for architecture armeabi-V7a
#cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE:STRING=$toolchain_file -DANDROID_PLATFORM=android-24 -DDEPLOY_PLATFORM:STRING="armeabi-v7a" -DANDROID_ABI="armeabi-v7a"
#ninja

# # for architecture arm64-v8a
 cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE:STRING=$toolchain_file -DANDROID_PLATFORM=android-24 -DDEPLOY_PLATFORM:STRING="arm64-v8a" -DANDROID_ABI="arm64-v8a"
 ninja

cd ..
mv tempmain main.c
echo "exiting..."