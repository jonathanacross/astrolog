A record of my astonomy observations.

# Building the project

```
./gradlew installDist
```

# Generate a web page of messier objects

```
./build/install/astrolog/bin/astrolog \
--log=data/observations.tsv \
--logIds=data/messier_ids.tsv \
--template=data/template.html \
--name="Messier Objects" \
--imageDir=sketches \
--imageExtension=.jpg \
--output=data/messier_output.html
```

# Image processing

To convert images to black and white (better for laserprinting)

```
mkdir sketches_bw
pushd sketches
for f in *.jpg ; do
    echo $f
    convert $f -colors 2 -colorspace gray -normalize ../sketches_bw/$f
done
popd
