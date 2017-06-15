#!/bin/sh
cat <<__EOF__
%C - ${DESC:?}

To use this library, list it at the beginning of the "wfd-dlls" line in
graphics.conf.
__EOF__
