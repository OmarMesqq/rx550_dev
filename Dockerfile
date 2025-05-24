FROM rocm/dev-ubuntu-22.04:5.4

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        gcc \
        ocl-icd-opencl-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


WORKDIR /app

COPY square.c square.cl .

RUN gcc square.c -o square -lOpenCL

CMD ["./square"]
