FROM ubuntu:18.04
WORKDIR /cs222

# for opencv installation
# https://www.geeksforgeeks.org/how-to-install-opencv-in-c-on-linux/
RUN bash -c 'apt-get update && apt-get -y install \
	sudo \
	g++ \
	cmake \
	make \
	git \
	libgtk2.0-dev \
	pkg-config'
RUN bash -c 'git clone https://github.com/opencv/opencv.git'
RUN bash -c 'mkdir -p build'
RUN bash -c 'cd build && cmake ../opencv'
RUN bash -c 'cd build && make -j4'
RUN echo 'export OpenCV_DIR=/cs222/build' >> ~/.bashrc
