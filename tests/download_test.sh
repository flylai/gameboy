#!/bin/bash

git clone https://github.com/retrio/gb-test-roms.git --depth 1
curl -L https://gekkio.fi/files/mooneye-test-suite/mts-20240127-1204-74ae166/mts-20240127-1204-74ae166.zip -o mts.zip
unzip mts.zip -d .
mv mts-20240127-1204-74ae166 mts
