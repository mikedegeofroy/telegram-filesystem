.PHONY: all build clean

# Default target
all: build

# Pull up docker container
up:
	docker-compose up -d

# Pull down docker container
down:
	docker-compose down

# Acess container shell
shell:
	docker exec -it centos-filesystem-testing /bin/bash

# Build the project inside the Docker container
build:
	docker-compose exec centos-filesystem-testing /bin/bash -c "cd /root/customfs && mkdir -p build && cd build && cmake .. && make"

# Clean the build directory
clean:
	docker-compose exec centos-filesystem-testing /bin/bash -c "cd /root/customfs && rm -rf build"

build-local:
	if [ ! -d build ]; then mkdir build; fi && cd build && cmake .. && make

rebuild: clean build
