#!/bin/bash

curl -L https://files.scene.org/get/demos/groups/snorpung/pocket.zip -o pocket.zip
curl -L https://files.scene.org/get/demos/groups/snorpung/gejmboj.zip -o gejmboj.zip
curl -L https://files.scene.org/get/demos/groups/snorpung/oh_2.zip -o oh_2.zip
curl -L https://archive.org/download/sml_gb_2/sml.gb -o sml.gb
curl -L https://archive.org/download/tetris_gb/tetris.gb -o tetris.gb
#curl -L https://archive.org/download/tetrisp_gb/tetrisp.gb -o tetrisp.gb

for filename in ./*.zip; do
    if [ -f "$filename" ]; then
        # Extract the contents of the zip file
        unzip -o "$filename" -d .
    fi
done

for filename in ./*; do
    if [ -f "$filename" ] && [[ "$filename" != *.gb ]]; then
        rm -f "$filename"
    fi
done
