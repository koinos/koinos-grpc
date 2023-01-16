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
   docker build . -t koinos-grpc-ccache --target builder
   docker build . -t koinos/koinos-grpc:$TAG
   docker run -td --name ccache koinos-grpc-ccache
   docker cp ccache:/koinos-grpc/.ccache ~/
fi
