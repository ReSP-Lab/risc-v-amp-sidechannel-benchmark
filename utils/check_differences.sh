#!/bin/bash
# or execute with `bash`, not with `sh`

diff -y <( nl -b a $1 ) <( nl -b a $2 ) | grep '|'
