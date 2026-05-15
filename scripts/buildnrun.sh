#!/bin/bash

cd "$(dirname "$0")" || exit
./build.sh
./run.sh
