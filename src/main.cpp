#include <opencv2/opencv.hpp>
#include <stdio.h>
#include "include/api.h"
#include <fstream>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
  if (argc != 3 && false) {
    cout << "usage: Compress.out <Image_Path> <Output_Path>\n" << endl;
    return -1;
  }

  //string input_file(argv[1]);
  //string output_file(argv[2]);
  string input_file = "../images/camera.bmp";
  string output_file = "../output/manual.fractal";

  // Variables to play around with
  int source_size=64;
  int destination_size=32;
  int step=16;

  cout << "Beginning compression" << endl;
  compress(input_file, output_file, source_size, destination_size, 4, step);
  cout << "Finished compression" << endl;

  return 0;
}