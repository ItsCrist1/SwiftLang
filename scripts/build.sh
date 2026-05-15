#!/bin/bash

cd "$(dirname "$0")" || exit
cmake --build ../build -j "$(nproc)"
