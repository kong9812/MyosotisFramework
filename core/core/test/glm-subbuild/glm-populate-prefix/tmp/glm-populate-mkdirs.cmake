# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/proj/MyosotisFramework/core/core/test/glm-src"
  "D:/proj/MyosotisFramework/core/core/test/glm-build"
  "D:/proj/MyosotisFramework/core/core/test/glm-subbuild/glm-populate-prefix"
  "D:/proj/MyosotisFramework/core/core/test/glm-subbuild/glm-populate-prefix/tmp"
  "D:/proj/MyosotisFramework/core/core/test/glm-subbuild/glm-populate-prefix/src/glm-populate-stamp"
  "D:/proj/MyosotisFramework/core/core/test/glm-subbuild/glm-populate-prefix/src"
  "D:/proj/MyosotisFramework/core/core/test/glm-subbuild/glm-populate-prefix/src/glm-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/proj/MyosotisFramework/core/core/test/glm-subbuild/glm-populate-prefix/src/glm-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/proj/MyosotisFramework/core/core/test/glm-subbuild/glm-populate-prefix/src/glm-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
