# A filesystem liked to telegram saved messages

This is a way to mount telegram to your filesystem and use it as unlimited storage ;) 

> I've lost count of all of the dependencies of this application, but I will make sure to make a build script for the application, for the time being, you can mess around with the [docker image](#Makefile).

## Todo
- [ ] Fix multithreading problem (fuse kills all threads that spawned before return fuse_main(), unless mounted with -f)
- [ ] Build script
- [ ] Add permissions, so we could create readonly filesystems (for channel subscribers, that are non-admins)
- [ ] Find a graceful way to handle subdirectories, while keeping the ability to just drop in the file where you need it
- [ ] Macos support
- [ ] Gracefully handle external libraries
- [ ] Separate large files into chunks, maybe add a bot for downloads??..

## Makefile

You can spin up a dev environment with

```shell
make up
```

You can spin down a dev environment with

```shell
make down
```

Enter the shell of the dev environment with

```shell
make shell
```

Use make build, rebuild and clean, to build in the dev environment

```shell
make build
make rebuild
make clean
```

If you are building localy use

```shell
make build-local
```

## Contributions
I would like to thank [Etozheraf](https://github.com/Etozheraf) for helping me out in this project and writing most of the functions for fuse. If you'd like to contribute contact me at [@mikedegeofroy](https://t.me/mikedegeofroy).
