# Testing Infra Structure

There are two important things to know:

- These tests rely on having your file structure set up a certain way. Specifically you should run them from the base directory, images should be in ./images, and there should be an ./output folder
- Right now, the graphing is done in Python. I made a bad choice which was instead of changing the docker whatever so it would come installed with the right version and matplotlib, I just manually installed everything. I know I know. Bad decision. But I didn't want to have to wait the like 25 minutes again for everything to rebuild and python takes like 30 seconds to install

## Benchmark single file

This script simply compresses and then decompresses an image and then outputs the relevant info to std out as described in the .sh file.

## Bencmark scales

The purpose of this test is to get a better sense of how our implementation is scaling with larger and larger images. Since it's annoying to have to maintain a bunch of different images and then extract image sizes from there, it creates a bunch of reduced images, at 1/n, 2/n, 3/n, ..., n/n times the original size of the image, saves them, and then runs benchmark file on all of them. It pipes the results to files which it stores in a results folder.

Oh, and this baby is parallelized. This makes up for the fact that I made a bad Python decision now
