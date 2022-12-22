#include <opencv2/opencv.hpp>
#include <stdio.h>
#include "../include/api.h"
#include <fstream>
#include <chrono>
#include<iostream>
#include<fstream>

using namespace cv;
using namespace std;
using namespace std::chrono;

// A simple program to benchmark a single file
// Expects as input <InputImagePath> <OutputImagePath>
// The intermediate .fractal file is put in a scratch file and disregarded
// Results are written to standard output with simple form: encode_time final_size initial_size decode_time quality

int main(int argc, char** argv) {
  if (argc < 3) {
    cout << "usage: Compress.out <Image_Path> <Output_path> ?<source_size> ?<dest_size> ?<step_size> ?<num_cores>\n" << endl;
    return -1;
  }

  string input_file(argv[1]);
  string encoded_file = "./test_infra/scratch/scratch.fractal";
  string output_file(argv[2]);

  // Variables to play around with
  int source_size = argc > 3 ? atoi(argv[3]) : 16;
  int destination_size = argc > 4 ? atoi(argv[4]) : 8;
  int step = argc > 5 ? atoi(argv[5]) : 4;
  int num_cores = argc > 6 ? atoi(argv[6]) : 4;

  auto start_compress = high_resolution_clock::now();
  compress(input_file, encoded_file, source_size, destination_size, step, num_cores, false);
  auto stop_compress = high_resolution_clock::now();
  auto compress_duration = duration_cast<milliseconds>(stop_compress - start_compress);
  cout << compress_duration.count() << " ";
  cout << get_filesize(encoded_file) << " ";
  cout << get_filesize(input_file) << " ";

  auto start_decompress = high_resolution_clock::now();
  decompress(encoded_file, output_file, 8);
  auto stop_decompress = high_resolution_clock::now();
  auto decompress_duration = duration_cast<milliseconds>(stop_decompress - start_decompress);
  cout << decompress_duration.count() << " ";

  float quality = MSE(input_file, output_file);
  cout << quality << endl;

  return 0;
}