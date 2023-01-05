#!/bin/bash

set -e
set -x

if [[ -z $BUILD_DOCKER ]]; then
   mkdir build
   cd build

   if [ "$RUN_TYPE" = "test" ]; then
      cmake -DCMAKE_BUILD_TYPE=Release ..
      cmake --build . --config Release --parallel 3
   elif [ "$RUN_TYPE" = "coverage" ]; then
      cmake -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON ..
      cmake --build . --config Debug --parallel 3 --target coverage
   fi
else
   TAG="$TRAVIS_BRANCH"
   if [ "$TAG" = "master" ]; then
      TAG="latest"
   fi

   cp -R ~/.ccache ./.ccache
   docker build . -t koinos-services-ccache --target builder
   docker build . -t koinos/koinos-services:$TAG
   docker run -td --name ccache koinos-services-ccache
   docker cp ccache:/koinos-services/.ccache ~/
fi
