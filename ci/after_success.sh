#!/bin/bash

if [ "$RUN_TYPE" = "coverage" ]; then
   coveralls-lcov --repo-token "$COVERALLS_REPO_TOKEN" --service-name travis-pro --service-job-id "$TRAVIS_JOB_ID" ./build/coverage.info
fi

if ! [[ -z $BUILD_DOCKER ]]; then
   TAG="$TRAVIS_BRANCH"
   if [ "$TAG" = "master" ]; then
      TAG="latest"
   fi

   echo "$DOCKER_PASSWORD" | docker login -u $DOCKER_USERNAME --password-stdin
   docker push koinos/koinos-services:$TAG
fi
