#!/bin/bash

cd "$(dirname "$0")" || exit
cmake -S .. -B ../build
