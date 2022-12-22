#include <opencv2/opencv.hpp>
#include <stdio.h>
#include "../include/api.h"
#include <fstream>
#include <chrono>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace cv;
using namespace std;
using namespace std::chrono;

// Given a base truth image, resizes and benchmarks on that new image
int run_resize_trial(string path_prefix, string image_name, float size_factor) {
  Mat M = imread(path_prefix + image_name, IMREAD_GRAYSCALE);
  resize(M, M, Size(), size_factor, size_factor, INTER_AREA);
  string before_path = "./output/benchmarking/resizing/before/" + to_string(size_factor) + image_name;
  string after_path = "./output/benchmarking/resizing/after/" + to_string(size_factor) + image_name;
  imwrite(before_path, M);
  int child_pid = fork();
  if (child_pid == 0) {
    // Forks a child and has it run the individual file benchmarks, writing resutls to a file
    string results_file = "./output/benchmarking/resizing/results/" + to_string(size_factor) + ".txt";
    int fd = open(results_file.c_str(), O_RDWR | O_CREAT);
    dup2(fd, 1);
    close(fd);
    execl("./src/benchmarking/benchmark_file", "benchmark_file", before_path.c_str(), after_path.c_str(), (char*) NULL);
    exit(-1);
  }
  return child_pid;
}

// A simple program to benchmark a single file
// Expects as input <InputImagePath> <OutputImagePath>
// The intermediate .fractal file is put in a scratch file and disregarded
// Results are written to standard output with simple form: encode_time final_size initial_size decode_time quality

int main(int argc, char** argv) {
  if (argc != 3) {
    cout << "usage: Compress.out <Name of Image> <Number of subdivisions>\n" << endl;
    return -1;
  }

  // Split the input into path_prefix and image_name
  string input_file(argv[1]);
  int split_ix = input_file.length();
  while (split_ix > 0) {
    if (input_file[split_ix] == '/') {
      ++split_ix;
      break;
    }
    --split_ix;
  }
  string path_prefix = input_file.substr(0, split_ix);
  string image_name = input_file.substr(split_ix, input_file.length());

  unordered_map<int, float> child_map;

  // One run of the file benchmark per subdivision
  int num_subdivisions = atoi(argv[2]);
  for (int sx=0; sx<num_subdivisions; ++sx) {
    float size_factor = (sx + 1.0) * (1.0 / num_subdivisions);
    int pid = run_resize_trial(path_prefix, image_name, size_factor);
    child_map.insert(make_pair(pid, size_factor));
  }

  int cpid;
  int status;
  while ((cpid = wait(&status)) > 0) {
    auto child_info = child_map.find(cpid);
    if (child_info == child_map.end()) continue;
    cout << "Child " << child_info->first << " for " << child_info->second << " returned " << status << endl;
  }

  return 0;
}