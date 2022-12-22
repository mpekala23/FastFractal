#include <opencv2/opencv.hpp>
#include <stdio.h>
#include "include/api.h"
#include <fstream>

// Get the size of a file
int get_filesize(string path) {
  ifstream in_file(path, ios::binary);
  in_file.seekg(0, ios::end);
  int file_size = in_file.tellg();
  in_file.close();
  return file_size;
}

// Calculates the MSE between two matrices
float MSE(const Mat &M, const Mat &N) {
  assert(M.rows == N.rows && M.cols == N.cols);
  float total = 0;
  float diff = 0;
  for (int rx=0; rx<M.rows; ++rx) {
    for(int cx=0; cx<M.cols; ++cx) {
      diff = (int) M.at<unsigned char>(rx, cx) - (int) N.at<unsigned char>(rx, cx);
      total += diff * diff;
    }
  }
  total /= M.rows * M.cols;
  return total;
}

// Calculates the MSE between two images
float MSE(string source_file, string decoded_file) {
  Mat M = imread(source_file, IMREAD_GRAYSCALE);
  Mat N = imread(decoded_file, IMREAD_GRAYSCALE);
  if (!M.data || !N.data) {
    return -1;
  }
  return MSE(M, N);
}