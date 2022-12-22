import os
import sys
import matplotlib.pyplot as plt

# Given a full filepath to a results.txt file returns:
#   (compress_time, final_size, initial_size, decompress_time, quality)
def read_results_file(filepath):
  with open(filepath) as fin:
    (compress_time, final_size, initial_size, decompress_time, quality) = fin.read().split()
  return (
    float(compress_time), 
    float(final_size), 
    float(initial_size), 
    float(decompress_time), 
    float(quality)
  )

# Given (X, Y, title, xlabel, ylabel) makes a plot
def make_plot(X, Y, title, xlabel, ylabel):
  fig, ax = plt.subplots()
  ax.plot(X, Y, marker="o")
  ax.set_title(title)
  ax.set_xlabel(xlabel)
  ax.set_ylabel(ylabel)
  ax.legend()
  return (fig, ax)

def main():
  image_name = sys.argv[1]
  name_without_extension = image_name[:image_name.rfind(".")]
  results_folder = "./output/benchmarking/parallel/results"
  graphs_folder = "./output/graphs/parallel/" + name_without_extension + "/"
  if not os.path.exists(graphs_folder):
    os.mkdir(graphs_folder)

  # Get all the results files
  filenames = next(os.walk(results_folder), (None, None, []))[2]

  # Compile the result arrays
  compress_times = []
  ratios = []
  initial_sizes = []
  num_cores = []
  decompress_times = []
  qualities = []
  for filename in filenames:
    (compress_time, final_size, initial_size, decompress_time, quality) = read_results_file(results_folder + "/" + filename)
    compress_times.append(compress_time)
    ratios.append(final_size / initial_size)
    initial_sizes.append(initial_size)
    decompress_times.append(decompress_time)
    num_cores.append(int(filename.split("_")[0]))
    qualities.append(quality)
  
  (ctime_fig, ctime_ax) = make_plot(num_cores, compress_times, image_name + " Compression Time", "Number of Cores", "Compression Time (ms)")
  (cratio_fig, cratio_ax) = make_plot(initial_sizes, ratios, image_name + " Compression Ratio", "Initial Size", "Compression Ratio")
  (dtime_fig, dtime_ax) = make_plot(initial_sizes, decompress_times, image_name + " Decompression Time", "Initial Size", "Decompression Time (ms)")
  (quality_fig, quality_ax) = make_plot(initial_sizes, qualities, image_name + " Error", "Initial Size", "MSE")

  ctime_fig.savefig(graphs_folder + name_without_extension + "-ctime.png")
  #cratio_fig.savefig(graphs_folder + name_without_extension + "-cratio.png")
  #dtime_fig.savefig(graphs_folder + name_without_extension + "-dtime.png")
  #quality_fig.savefig(graphs_folder + name_without_extension + "-quality.png")

if __name__ == "__main__":
  main()

