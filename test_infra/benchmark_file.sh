# Compresses and decompresses a single image
# USAGE: ./benchmark_file.sh name_of_file_in_images > PIPE OUTPUT
# Output has form (encode time, final size, initial size, decode time)
# Should be run from the base directory

src/benchmarking/benchmark_file ./images/$1 ./output/benchmarking/$1 $2 $3 $4 $5
