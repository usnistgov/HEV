#! /bin/sh

# print the usage messages for the commands

grep -h "^Usage" hyv-*  | grep -v "hgv"|  sort | uniq

