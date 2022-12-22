#include <opencv2/opencv.hpp>
#include <stdio.h>
#include "include/api.h"
#include <fstream>


using namespace cv;
using namespace std;

void JPEG_postprocess(Mat& m, int dct_size) {
  dct(m, m, DCT_INVERSE);
  /*
  for (int rx=0; rx<m.rows; rx+=dct_size) {
    for (int cx=0; cx<m.cols; cx+=dct_size) {
      for (int u=0; u<8; ++u) {
        for (int v=0; v<8; ++v) {
          int val = (int) m.at<unsigned char>(rx+u, cx+v);
          double frac = (255.0 - val) / 255;
          int new_val = 255 - 255 * pow(frac, 1.0/6);
          m(Rect(rx,cx,1,1)).setTo(new_val);
        }
      }
      dct(m(Rect(rx,cx,dct_size,dct_size)), m(Rect(rx,cx,dct_size,dct_size)), DCT_INVERSE);
    }
  }
  */
}

void decompress(string input_file, string output_file, int num_iters) {
	Mat reconstructed_image = decompress(input_file, num_iters);
	imwrite(output_file, reconstructed_image);
}

Mat decompress(string input_file, int num_iters) {
	// Open the input file
	ifstream infile;
	infile.open(input_file, ios::binary | ios::in);

	// Read the metadata
	int m, n, src_sz, dst_sz, step, rows_added, cols_added;
	infile.read((char*) &m, sizeof(int));
	infile.read((char*) &n, sizeof(int));
	infile.read((char*) &src_sz, sizeof(int));
	infile.read((char*) &dst_sz, sizeof(int));
	infile.read((char*) &step, sizeof(int));
	infile.read((char*) &rows_added, sizeof(int));
	infile.read((char*) &cols_added, sizeof(int));


	// Read the actual transforms
	vector<vector<Transform>> transforms;
	for (int i = 0; i < m; i++) {
		vector<Transform> row;
		for (int j = 0; j < n; j++) {
			TransformArtifact artifact;
			infile.read((char*) &artifact, sizeof(TransformArtifact));
      Transform t;
      t.i = artifact.i;
      t.j = artifact.j;
      t.brightness = artifact.brightness;
      t.contrast = artifact.contrast;
      t.direction = artifact.direction;
      t.rotation = artifact.rotation;
			row.push_back(t);
		}
		transforms.push_back(row);
	}

	infile.close();

	// Decompress using the metadata and the transforms
	vector<Mat> iterations = decompress(transforms, src_sz, dst_sz, step, num_iters);

  // Undo the JPEG transformation
  Mat raw_result = iterations.back();
  // Remove the rows/cols we added to make it divisible by domains/ranges
  Rect final_size(0, 0, raw_result.rows - rows_added, raw_result.cols - cols_added);
  Mat final_result(final_size.width, final_size.height, raw_result.type());
  raw_result(final_size).copyTo(final_result(final_size));

	return final_result;
}

vector<Mat> decompress(const vector<vector<Transform>> &transforms, int src_sz, int dst_sz, int step, int num_iters) {

  float shrink_factor = (float) dst_sz / (float) src_sz;

  int m = transforms.size();
  int n = transforms[0].size();

  // Compute image dimensions
  int final_width = m * dst_sz;
  int final_height = n * dst_sz;

  // Image at each iteration
  vector<Mat> iter_results;

  // Create a random initial matrix
  Mat initial = Mat(final_width, final_height, CV_64FC1);
  float low = 0; float high = 255;
  randu(initial, Scalar(low), Scalar(high));

  // Initial iteration is random image
  iter_results.push_back(initial);

  // Iterate to reconstruct the image
  for (int iter = 0; iter < num_iters; iter++) {

    // Generate new image iteration
    Mat range = Mat(final_width, final_height, CV_64F, 0.0);
    Mat domain;
    reduce(iter_results.back(), shrink_factor, domain);

    // Iterate through every range block
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < n; j++) {

      	// Grab the associated transform for the range block
        Transform t = transforms[i][j];

        // Grab the domain block associated with the transform
        Rect dsub(
        	t.i * step * shrink_factor,
        	t.j * step * shrink_factor,
        	src_sz * shrink_factor,
        	src_sz * shrink_factor
        );
        Mat dblock = domain(dsub).clone();

        // Get the new range block
        Mat rblock;
        apply_transform(dblock, t, rblock);
        rblock.copyTo(range(Rect(i * dst_sz, j * dst_sz, dst_sz, dst_sz)));
      }
    }
    iter_results.push_back(range);
  }

  return iter_results;
}