#include <opencv2/opencv.hpp>
#include <stdio.h>
#include "include/api.h"
#include <fstream>
#include <thread>
#include <cmath>
#include <math.h>
#include <random>

using namespace cv;
using namespace std;

const char* DirStr(Direction d) {
    switch (d)
    {
        case NO_DIRECTION: return "no_direction";
        case HORIZONTAL: return "horizontal";
        case VERTICAL: return "vertical";
        case BOTH: return "both";
    }
    return "invalid";
}

const char* RotStr(Rotation r) {
    switch (r)
    {
        case NO_ROTATION: return "0";
        case NINETY: return "90";
        case ONEEIGHTY: return "180";
        case TWOSEVENTY: return "270";
    }
    return "invalid";
}

void reduce(const Mat &m, float factor, Mat &result) {
  resize(m, result, Size(), factor, factor, INTER_AREA);
}

void flip(const Mat &m, Direction direction, Mat &result) {
  if (direction == NO_DIRECTION) {
    result = m;
    return;
  };
  flip(m, result, direction);
}

void rotate(const Mat &m, Rotation rotation, Mat &result) {
  if (rotation == NO_ROTATION) {
    result = m;
    return;
  }
  switch (rotation) {
    case NINETY:
      transpose(m, result);
      flip(result, result, 0);
      break;
    case ONEEIGHTY:
      flip(m, result, -1);
      break;
    case TWOSEVENTY:
      transpose(m, result);
      flip(result, result, 1);
      break;
  }
}

void apply_transform(const Mat &m, Transform &t, Mat &result) {
  flip(m, t.direction, result);
  rotate(result, t.rotation, result);
  result = t.contrast * result;
  result = result + t.brightness;
}

String wavelet_hash(Transform t) {
  String result = "";
  for (int d=0; d<t.jpeg.rows; ++d) {
    int num_pos = 0;
    int num_neg = 0;
    for (int l=0; l<=d; ++l) {
      if (t.jpeg.at<float>(d-l, l) >= 0) {
        result += "1";
      } else {
        result += "0";
      }
    }
  }
  return result;
}

bool wavelet_hash_sort(Transform t1, Transform t2) {
  return wavelet_hash(t1) < wavelet_hash(t2);
}

void generate_transforms_chunked(const Mat &m, int src_sz, int dst_sz, int step, int low_r, int high_r, vector<Transform>* result) {
  float shrink_factor = ((float)dst_sz) / src_sz;
  for (int ix=low_r; ix < high_r; ++ix) {
    for (int jx=0; jx < m.cols / step; ++jx ) {
    	// Iterate over all possible transformations of
    	// those blocks
    	for (auto d : directions) {
    		for (auto r : rotations) {
    			if (ix * step + src_sz > m.rows || jx * step + src_sz > m.cols) {
            // TODO: Is this the best way to do this? Just ignore any block that
            // would take us out of bounds
            continue;
          }
          Transform t;
          t.direction = d;
          t.rotation = r;
          t.i = ix;
          t.j = jx;
          t.data = m(
            Rect(ix * step, jx * step, src_sz, src_sz)
          ).clone();
          reduce(t.data, shrink_factor, t.data);
          flip(t.data, t.direction, t.data);
          rotate(t.data, t.rotation, t.data);
          dct(t.data, t.jpeg);
          result->push_back(t);
    		}
    	}
    }
  }
}

vector<Transform> generate_transforms(const Mat &m, int src_sz, int dst_sz, int step) {
  vector<Transform> result;

  int processor_chunk = (m.rows / step) / NUM_PROCESSORS;
  vector<thread> threads;

  vector<vector<Transform>*> meta_results;

  for (int th=0; th<NUM_PROCESSORS; ++th) {
    int low_r = processor_chunk * th;
    int high_r = processor_chunk * (th + 1);
    if (th + 1 == NUM_PROCESSORS) {
      // Fix weird sizing if not perfectly divisible
      high_r = m.rows / step;
    }
    vector<Transform>* this_result = new vector<Transform>();
    meta_results.push_back(this_result);
    threads.push_back(thread(generate_transforms_chunked, m, src_sz, dst_sz, step, low_r, high_r, this_result));
  }

  for (auto &th : threads) {
    th.join();
  }

  for (auto partial_result : meta_results) {
    result.insert(result.end(), partial_result->begin(), partial_result->end());
  }

  // cout << "About to sort all " << result.size() << " transformations" << endl;
  sort(result.begin(), result.end(), wavelet_hash_sort);
  // cout << "Sorted" << endl;
  for (auto t : result) {
    // cout << wavelet_hash(t) << endl;
  }

  return result;
}

// NOTE: Probably bad practice but this has the side-effect of setting correct
// contrast and brightness
float distortion(const Mat &rblock, const Mat &dblock, Transform &t) {
  t.contrast = 1;
  t.brightness = 0;
  Mat candidate = dblock.clone();
  
  Mat A = candidate.reshape(candidate.channels(), candidate.rows * candidate.cols);
  Mat one_col = A.clone();
  one_col = 1;
  hconcat(A, one_col, A);
  Mat b = rblock.reshape(rblock.channels(), rblock.rows * rblock.cols);
  Mat X(2, 1, CV_64F);

  solve(A, b, X, DECOMP_QR);
  t.contrast = X.at<double>(0,0);
  t.brightness = X.at<double>(1,0);

  candidate = t.contrast * candidate + t.brightness;
  Mat diff = candidate - rblock;

  return diff.dot(diff);
}

void compress(string input_file, string output_file, int src_sz, int dst_sz, int step, int num_cores, bool silent) {
	// Turn the image into transforms
  int rows_added = 0;
  int cols_added = 0;
	auto transforms = compress(input_file, src_sz, dst_sz, step, rows_added, cols_added, num_cores, silent);

	// The original image was m x n blocks
	// of size dst_sz
	int m = transforms.size();
	int n = transforms[0].size();

	// Open the output file
	ofstream outfile;
	outfile.open(output_file, ios::binary | ios::out);

	// Write the metadata
	outfile.write((char*) &m, sizeof(int));
	outfile.write((char*) &n, sizeof(int));
	outfile.write((char*) &src_sz, sizeof(int));
	outfile.write((char*) &dst_sz, sizeof(int));
	outfile.write((char*) &step, sizeof(int));
  outfile.write((char*) &rows_added, sizeof(int));
  outfile.write((char*) &cols_added, sizeof(int));

	// Write the transforms
	for (const auto &row : transforms) {
		for (const auto &col : row) {
      TransformArtifact artifact;
      artifact.i = col.i;
      artifact.j = col.j;
      artifact.brightness = col.brightness;
      artifact.contrast = col.contrast;
      artifact.direction = col.direction;
      artifact.rotation = col.rotation;
			outfile.write((char*) &artifact, sizeof(TransformArtifact));
		}
	}

	outfile.close();
}

vector<vector<Transform>> compress(string input_file, int src_sz, int dst_sz, int step, int &rows_added, int &cols_added, int num_cores, bool silent) {

	// Load the image
  Mat image = imread(input_file, IMREAD_GRAYSCALE);
  if (!image.data) {
  	std::cerr << "No image data" << std::endl;
    vector<vector<Transform>> empty(0, vector<Transform>(0));
    return empty;
  }	

  // Convert to necessary precision
  image.convertTo(image, CV_64F);

  // Compress the image
  auto transforms = compress(image, src_sz, dst_sz, step, rows_added, cols_added, num_cores, silent);
  return transforms;
}

vector<int> real_vals;
vector<int> fake_vals;

vector<float> real_ds;
vector<float> metric_ds;
vector<float> random_ds;

void find_range_chunked(const Mat &domain, const Mat &range, const vector<Transform> &transforms, int src_sz, int dst_sz, int step, int low_i, int high_i, vector<vector<Transform>>* result) {
  // (m,n) shape block stuff
  int m = range.rows / dst_sz;
  int n = range.cols / dst_sz;

  // The domain block should encompass a larger region of the original
	// image as compared to the range block so the shrink factor should
	// be in (0, 1)
  float shrink_factor = (float) dst_sz / (float) src_sz;

  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<> distr(0, transforms.size());

  int WINDOW = 3;

  for (int i = low_i; i < high_i && i < m; i++) {
    for (int j = 0; j < n; j++) {
    	// Grab the range block
      Mat rblock = range(
      	Rect(i * dst_sz, j * dst_sz, dst_sz, dst_sz)
      ).clone();

      Mat rjpeg = rblock.clone();
      dct(rblock, rjpeg);
      Transform dummy;
      dummy.jpeg = rjpeg;
      String val = wavelet_hash(dummy);

    /*
      // Okay baby, let's see what happens
      int lx = 0; int ux = transforms.size();
      while (ux - lx > 1) {
        int mx = (lx + ux) / 2;
        Transform t = transforms[mx];
        String compare_val = wavelet_hash(t);
        if (val == compare_val) {
          lx = mx;
          break;
        }
        if (val < compare_val) {
          ux = mx;
        } else {
          lx = mx;
        }
      }

      
      int metric_start = lx - WINDOW < 0 ? 0 : lx - WINDOW;
      int metric_end = lx + WINDOW > transforms.size() ? transforms.size() : lx + WINDOW;
      float metric_min = 1000000000;
      for (int ix=metric_start; ix<metric_end; ++ix) {
        Transform t = transforms[ix];
        // Update the block if the distortion is less
        float d = distortion(rblock, t.data, t);
        if (d < metric_min) {
          metric_min = d;
        }
      }
      metric_ds.push_back(metric_min);

      int random_ix = distr(gen);
      int random_start = random_ix - WINDOW < 0 ? 0 : random_ix - WINDOW;
      int random_end = random_ix + WINDOW > transforms.size() ? transforms.size() : random_ix + WINDOW;
      float random_min = 1000000000;
      for (int ix=random_start; ix<random_end; ++ix) {
        Transform t = transforms[ix];
        // Update the block if the distortion is less
        float d = distortion(rblock, t.data, t);
        if (d < random_min) {
          random_min = d;
        }
      }
      random_ds.push_back(random_min);
      */

      float min_dist = -1;
      bool set_min = false;

      // Iterate over all the transform candidates
      // and select the closest one
      int final_ix = -1;
      for (int ix=0; ix < transforms.size(); ++ix) {
        Transform t = transforms[ix];
        // Update the block if the distortion is less
        float d = distortion(rblock, t.data, t);
        if (d < min_dist || !set_min) {
          set_min = true;
          min_dist = d;
          result->at(i)[j] = t;
          final_ix = ix;
        }
      }
      real_ds.push_back(min_dist);
    }
  }
}

// src_sz is the domain block size
// dst_sz is the range block size
vector<vector<Transform>> compress(const Mat &c_range,  int src_sz, int dst_sz, int step, int &rows_added, int &cols_added, int num_cores, bool silent) {
  // The domain block should encompass a larger region of the original
	// image as compared to the range block so the shrink factor should
	// be in (0, 1)
  float shrink_factor = (float) dst_sz / (float) src_sz;

  // Make a clone so we can add rows/cols if we need
  Mat range = c_range.clone();
  // generate the domain image
  Mat domain = range.clone();

  // Ensure that the image is divisible by our 
  // domain block size
  rows_added = 0;
  cols_added = 0;
  while (range.rows % dst_sz != 0) {
    Mat new_row = Mat(1, range.cols, range.type(), 0.0);
    range.push_back(new_row);
    ++rows_added;
  }
  while (range.cols % dst_sz != 0) {
    Mat new_col = Mat(range.rows, 1, range.type(), 0.0);
    hconcat(range, new_col, range);
    ++cols_added;
  }

  // Generate the transform candidates
  vector<Transform> transforms = generate_transforms(
  	domain, src_sz, dst_sz, step
  );

  //cout << "Number of transforms: " << transforms.size() << endl;

  // Our range image is m x n blocks
  int m = range.rows / dst_sz;
  int n = range.cols / dst_sz;

  vector<vector<Transform>>* result = new vector<vector<Transform>>();
  for (int row=0; row<m; ++row) {
    vector<Transform> blank_row(n);
    result->push_back(blank_row);
  }

  num_cores = 1;

  int processor_chunk = m / num_cores;
  vector<thread> threads;
  if (m % num_cores != 0) {
    processor_chunk += 1;
  }

  for (int th=0; th<num_cores; ++th) {
    int low_i = processor_chunk * th;
    int high_i = processor_chunk * (th + 1);
    if (th + 1 == num_cores) {
      // Make sure last chunk accounts for weird sizing
      high_i = m;
    }
    threads.push_back(thread(find_range_chunked, domain, range, transforms, src_sz, dst_sz, step, low_i, high_i, result));
  }

  for (auto &th : threads) {
    th.join();
  }

  //accumulate(v.begin(), v.end(), 0.0) / v.size();
  float real_avg = accumulate(real_ds.begin(), real_ds.end(), 0.0) / real_ds.size();
  float metric_avg = accumulate(metric_ds.begin(), metric_ds.end(), 0.0) / metric_ds.size();
  float random_avg = accumulate(random_ds.begin(), random_ds.end(), 0.0) / random_ds.size();

  cout << "good stuff below" << endl;
  cout << real_avg << endl << metric_avg << endl << random_avg << endl;

  return *result;
}
