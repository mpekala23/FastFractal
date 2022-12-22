#include <opencv2/opencv.hpp>
#include <stdio.h>
#include "include/api.h"
#include <fstream>
#include <chrono>
#include<iostream>
#include<fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace cv;
using namespace std;

/**
 * @brief This file documents the code and logic behind my initial attempt
 * to use JPEG preprocessing to improve compression. Results were terrible.
 */

void initial_JPEG_preprocess(Mat& m, int dct_size) {
  for (int rx=0; rx<m.rows; rx+=dct_size) {
    for (int cx=0; cx<m.cols; cx+=dct_size) {
      dct(m(Rect(rx,cx,dct_size,dct_size)), m(Rect(rx,cx,dct_size,dct_size)));
    }
  }
}

void initial_JPEG_postprocess(Mat& m, int dct_size) {
  for (int rx=0; rx<m.rows; rx+=dct_size) {
    for (int cx=0; cx<m.cols; cx+=dct_size) {
      dct(m(Rect(rx,cx,dct_size,dct_size)), m(Rect(rx,cx,dct_size,dct_size)), DCT_INVERSE);
    }
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "usage: jpeg_inital.out <Image_name.ext>\n" << endl;
    return -1;
  }

  // Get the image prefix and name.ext
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

  Mat image = imread(input_file, IMREAD_GRAYSCALE);
  if (!image.data) {
  	std::cerr << "No image data" << std::endl;
    vector<vector<Transform>> empty(0, vector<Transform>(0));
    return -1;
  }
  image.convertTo(image, CV_64F);
  initial_JPEG_preprocess(image, 32);
  string jpeg_input_file = "./output/jinput.bmp";
  imwrite(jpeg_input_file, image);
  string scratch_file = "./output/scratch.fractal";
  string jpeg_output_file = "./output/joutput.bmp";
  
  int source_size = 16;
  int destination_size = 8;
  int step = 4;
  int num_cores = 4;
  compress(jpeg_input_file, scratch_file, source_size, destination_size, step, num_cores, false);
  decompress(scratch_file, jpeg_output_file, 16);

  Mat raw_output = imread(jpeg_output_file, IMREAD_GRAYSCALE);
  if (!raw_output.data) {
  	std::cerr << "No image data" << std::endl;
    vector<vector<Transform>> empty(0, vector<Transform>(0));
    return -1;
  }
  raw_output.convertTo(raw_output, CV_64F);
  initial_JPEG_postprocess(raw_output, 32);
  string output_file = "./output/jfinal.bmp";
  imwrite(output_file, raw_output);

  return 1;
}

