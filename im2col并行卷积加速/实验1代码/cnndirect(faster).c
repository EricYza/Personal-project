#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

void performConvolution(double* input, double* kernel, double* output,
                        int input_height, int input_width, int input_channel, int kernel_size, 
                        int stride, int padding, int rank, int size, int output_channel) {
    int output_height = (input_height - kernel_size + 2 * padding) / stride + 1;
    int output_width = (input_width - kernel_size + 2 * padding) / stride + 1;

    int chunk_size = output_height / size;
    int start_row = rank * chunk_size;
    int end_row = (rank == size - 1) ? output_height : start_row + chunk_size;

    // Pre-calculate constant indices outside loops
    int kernel_size_sq = kernel_size * kernel_size;
    int input_hw = input_height * input_width;

    #pragma omp parallel for collapse(3)
    for (int oc = 0; oc < output_channel; oc++) {
        for (int oh = start_row; oh < end_row; oh++) {
            for (int ow = 0; ow < output_width; ow++) {
                double temp = 0.0;
                int output_index = oc * output_height * output_width + oh * output_width + ow;
                for (int ic = 0; ic < input_channel; ic++) {
                    for (int kh = 0; kh < kernel_size; kh++) {
                        for (int kw = 0; kw < kernel_size; kw++) {
                            int ih = oh * stride - padding + kh;
                            int iw = ow * stride - padding + kw;
                            if(ih >= 0 && ih < input_height && iw >= 0 && iw < input_width) {
                                int input_index = ic * input_hw + ih * input_width + iw;
                                int kernel_index = oc * input_channel * kernel_size_sq + ic * kernel_size_sq + kh * kernel_size + kw;
                                temp += input[input_index] * kernel[kernel_index];
                            }
                        }
                    }
                }
                output[output_index] = temp;
            }
        }
    }
}

int main() {
    int rank, size;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int input_height = 1024;
    int input_width = 1024;
    int input_channel = 3;
    int kernel_size = 3;
    int stride = 1;
    int padding = 1;
    int output_channel = 3;

    double* input = (double*)malloc(input_height * input_width * input_channel * sizeof(double));
    double* kernel = (double*)malloc(kernel_size * kernel_size * output_channel * input_channel * sizeof(double));
    int output_height = (input_height - kernel_size + 2 * padding) / stride + 1;
    int output_width = (input_width - kernel_size + 2 * padding) / stride + 1;
    double* output = (double*)calloc(output_height * output_width * output_channel, sizeof(double));

    for (int i = 0; i < input_height * input_width * input_channel; i++) {
        input[i] = 1;
    }

    for (int i = 0; i < kernel_size * kernel_size * output_channel * input_channel; i++) {
        kernel[i] = 2;
    }

    double start_time = MPI_Wtime();
    performConvolution(input, kernel, output, input_height, input_width, input_channel, kernel_size, stride, padding, rank, size, output_channel);
    double end_time = MPI_Wtime();
    double elapsed_time = end_time - start_time;

    double* reduced_output = (double*)calloc(output_height * output_width * output_channel, sizeof(double));

    MPI_Reduce(output, reduced_output, output_height * output_width * output_channel, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Elapsed Time (Rank %d): %.6f seconds\n", rank, elapsed_time);
        for (int oc = 0; oc < output_channel; oc++) {
            printf("Output (filter%d):\n", oc);
            for (int outh = 0; outh < output_height; outh++) {
                for (int outw = 0; outw < output_width; outw++) {
                    int output_index = oc * output_height * output_width + outh * output_width + outw;
                    printf("%.1f ", reduced_output[output_index]);
                }
                printf("\n");
            }
            printf("\n");
        }
    }

    free(input);
    free(kernel);
    free(output);
    free(reduced_output);

    MPI_Finalize();
    return 0;
}
