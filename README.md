## An attempt to learn about GPUs on a low budget GPU
AMD's RX550 is old and unsupported, but that doesn't stop us from learning to
do awesome graphics and ML with it.

1. Build the image:
```shell
docker build -t rx550-dev .
```

2. Run with:
```shell
docker run --rm -it \
  --device=/dev/kfd \
  --device=/dev/dri \
  --group-add video \
  --cap-add=SYS_PTRACE \
  --security-opt seccomp=unconfined \
  rx550-dev
``` 
