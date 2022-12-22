BASE_DIR := $(shell pwd)
DOCKER_CMD := docker

build:
	$(DOCKER_CMD) build --tag=cs222 .

# The images folder is read only
start:
	$(eval DOCKER_CONT_ID := $(shell docker container run \
		-v $(BASE_DIR)/src:/cs222/src \
		-v $(BASE_DIR)/images:/cs222/images:ro \
		-v $(BASE_DIR)/output:/cs222/output \
		-v $(BASE_DIR)/test_infra:/cs222/test_infra \
		-d --rm --privileged -i -t cs222 bash))
	echo $(DOCKER_CONT_ID) > status.current_container_id

stop:
	$(eval DOCKER_CONT_ID := $(shell cat status.current_container_id | awk '{print $1}'))
	$(DOCKER_CMD) stop $(DOCKER_CONT_ID)
	rm status.current_container_id

interact:
	$(eval DOCKER_CONT_ID := $(shell cat status.current_container_id | awk '{print $1}'))
	$(DOCKER_CMD) exec -it $(DOCKER_CONT_ID) bash

distclean:
	rm -rf output
	mkdir -p output
	mkdir -p output/benchmarking
	mkdir -p output/benchmarking/results
	mkdir -p output/distortion
	mkdir -p output/graphs