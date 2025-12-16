#!/usr/bin/env bash
set -e

g++ -std=c++20 -O2 \
  src/main.cpp third_party/glad/src/gl.c \
  -Ithird_party/glad/include \
  -lsfml-window -lsfml-system \
  -lGL \
  -o engine
