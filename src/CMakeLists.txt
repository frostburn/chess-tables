ADD_LIBRARY(
  chesstables
  chess.c
  node.c
)
INSTALL(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include/chess-tables
  DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)
INSTALL(
  TARGETS chesstables
  ARCHIVE
  DESTINATION lib
)
