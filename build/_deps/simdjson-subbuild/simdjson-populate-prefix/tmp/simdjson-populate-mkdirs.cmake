# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/workspaces/New/build/_deps/simdjson-src"
  "/workspaces/New/build/_deps/simdjson-build"
  "/workspaces/New/build/_deps/simdjson-subbuild/simdjson-populate-prefix"
  "/workspaces/New/build/_deps/simdjson-subbuild/simdjson-populate-prefix/tmp"
  "/workspaces/New/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src/simdjson-populate-stamp"
  "/workspaces/New/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src"
  "/workspaces/New/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src/simdjson-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/workspaces/New/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src/simdjson-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/workspaces/New/build/_deps/simdjson-subbuild/simdjson-populate-prefix/src/simdjson-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
