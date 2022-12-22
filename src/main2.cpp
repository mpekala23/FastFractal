#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <string>
#include "include/api.h"

#include <fstream>

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
  if (argc != 3) {
    cout << "usage: Compress.out <Image_Path> <Output_path>\n" << endl;
    return -1;
  }

  string input_file(argv[1]);
  string output_file(argv[2]);

  decompress(input_file, output_file, 10);

  return 0;
}