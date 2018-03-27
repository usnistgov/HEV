#!/bin/sh
export OSG_FILE_PATH=$IRIS_HOME/examples/tim:$OSG_FILE_PATH 
export MCP_FILES=flipbook.mcp 
irisfly --ex flipbook.iris
