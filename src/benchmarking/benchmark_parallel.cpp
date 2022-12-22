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
int run_cores_trial(string path_prefix, string image_name, int num_cores) {
  string before_path = path_prefix + image_name;
  string after_path = "./output/benchmarking/parallel/images/" + to_string(num_cores) + "_cores_" + image_name;
  int child_pid = fork();
  if (child_pid == 0) {
    // Forks a child and has it run the individual file benchmarks, writing resutls to a file
    string results_file = "./output/benchmarking/parallel/results/" + to_string(num_cores) + "_results.txt";
    int fd = open(results_file.c_str(), O_RDWR | O_CREAT);
    if (fd < 0) {
      exit(-1);
    }
    dup2(fd, 1);
    close(fd);
    execl(
      "./src/benchmarking/benchmark_file", 
      "benchmark_file", before_path.c_str(), after_path.c_str(), to_string(24).c_str(), to_string(12).c_str(), to_string(6).c_str(), to_string(num_cores).c_str(),
      (char*) NULL
    );
    exit(-1);
  }

  int cpid;
  int status;
  while ((cpid = wait(&status)) > 0) {
    cout << "Trial on " << num_cores << " finished" << endl;
  }

  return child_pid;
}

// A simple program to benchmark a single file
// Expects as input <InputImagePath> <OutputImagePath>
// The intermediate .fractal file is put in a scratch file and disregarded
// Results are written to standard output with simple form: encode_time final_size initial_size decode_time quality

int main(int argc, char** argv) {
  if (argc != 3) {
    cout << "usage: Compress.out <Name of Image> <Max Cores>\n" << endl;
    return -1;
  }

  int max_cores = atoi(argv[2]);

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

  // One run of the file benchmark per subdivision
  int num_cores = 1;
  while (num_cores <= max_cores) {
    run_cores_trial(path_prefix, image_name, num_cores);
    num_cores += 1;
  }

  return 0;
}