# Compresses and decompresses a single image at a variety of scales
# Useful for reasoning about how our algorithm performs on various input sizes
# USAGE: ./benchmark_scales.sh name_of_file_in_images
# Output has form (encode time, final size, initial size, decode time)
# Should be run from the base directory

# Make sure necessary output folders for scales exist
rm -rf output/benchmarking/resizing
mkdir -p output/benchmarking/resizing/after
mkdir -p output/benchmarking/resizing/before
mkdir -p output/benchmarking/resizing/results

src/benchmarking/benchmark_scales ./images/$1 $2

# Make sure necessary output folder for graphs exist
mkdir -p output/graphs
python3 src/benchmarking/GraphScales.py $1
