#!/bin/sh
export PATH="$HEV_IDEA_DIR/etc/hev-animator/bin:$PATH"
export OSG_FILE_PATH=$IRIS_HOME/examples/tim:$OSG_FILE_PATH 
export MCP_FILES=tim.mcp 
irisfly --ex tim.iris

