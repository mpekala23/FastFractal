#include <opencv2/opencv.hpp>
#include <stdio.h>
#include "include/api.h"
#include <fstream>

using namespace cv;
using namespace std;

int main() {
  int width = 256;
  int height = 256;
  // Create a random initial matrix
  Mat initial = Mat(width, height, CV_64FC1);
  float low = 0; float high = 255;
  randu(initial, Scalar(low), Scalar(high));

  // Initial iteration is random image
  vector<Mat> iter_results;
  iter_results.push_back(initial);

  // Set iterations
  int num_iters = 10;

  // Iterate to reconstruct the image
  for (int iter = 0; iter < num_iters; iter++) {
    Mat range = Mat(width, height, CV_64F, 255);
    Mat domain = iter_results.back().clone();
    reduce(domain, 0.5, domain);

    // Range 1
    domain.copyTo(range(Rect(
      width / 4, 0, width / 2, height / 2
    )));

    // Range 2
    domain.copyTo(range(Rect(
      0, height / 2, width / 2, height / 2
    )));

    // Range 3
    domain.copyTo(range(Rect(
      width / 2, height / 2, width / 2, height / 2
    )));

    //range.convertTo(range, -1, 1, 75);

    iter_results.push_back(range);
  }

  for (int ix=0; ix<num_iters; ++ix) {
    String save_path = "../output/sierpinski/sierp" + to_string(ix) + ".png";
    imwrite(save_path, iter_results[ix]);
  }
}
