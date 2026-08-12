#!/usr/bin/env bash

/usr/games/cowsay "Hello, world!"

cd /LocalCont/source
./premake5 gmake
make

/LocalCont/source/build/autograder/RunSubmission