#!/bin/bash

echo "#Build tar.gz in preparation for building the RPM"
VERSION="0.1"
tar czvf diseval-$VERSION.tar.gz source/* --transform s/source/diseval-$VERSION/
