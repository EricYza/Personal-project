#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

// Function to perform convolution using im2col method
void performConvolution(double* input, double* kernel, double* output, double* im2col,
                        int input_height, int input_width, int input_channel,
                        int kernel_size, int stride, int padding, int rank, int size, int output_channel) {
    int output_height = (input_height - kernel_size + 2 * padding) / stride + 1;
    int output_width = (input_width - kernel_size + 2 * padding) / stride + 1;

    // Calculate the range of output rows to be processed by this rank
    int chunk_size = output_height / size;
    int start_row = rank * chunk_size;
    int end_row = (rank == size - 1) ? output_height : start_row + chunk_size;

    // Calculate the number of elements in im2col matrix
    int im2col_height = output_height * output_width;
    int im2col_width = input_channel * kernel_size * kernel_size;

    // Perform matrix multiplication
    #pragma omp parallel for collapse(3)
    for (int oc = 0; oc < output_channel; oc++) {
        for (int oh = start_row; oh < end_row; oh++) {
            for (int ow = 0; ow < output_width; ow++) {
                double sum = 0;
                for (int col = 0; col < im2col_width; col++) {
                    int kernel_index = oc * im2col_width + col;
                    int im2col_index = (oh * output_width + ow) * im2col_width + col;
                    sum += kernel[kernel_index] * im2col[im2col_index];
                }
                int output_index = oc * output_height * output_width + oh * output_width + ow;
                output[output_index] = sum;
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
    int output_channel = 3; // change to the desired number of output channels

    // Allocate memory for input, kernel, and output
    double* input = (double*)malloc(input_height * input_width * input_channel * sizeof(double));
    double* kernel = (double*)malloc(kernel_size * kernel_size * output_channel * input_channel * sizeof(double));
    int output_height = (input_height - kernel_size + 2 * padding) / stride + 1;
    int output_width = (input_width - kernel_size + 2 * padding) / stride + 1;
    double* output = (double*)malloc(output_height * output_width * output_channel * sizeof(double));

    // Calculate the number of elements in im2col matrix
    int im2col_height = output_height * output_width;
    int im2col_width = input_channel * kernel_size * kernel_size;

    // Allocate memory for im2col matrix
    double* im2col = (double*)malloc(im2col_height* im2col_width * sizeof(double));

    // Initialize input and kernel
    for (int i = 0; i < input_height * input_width * input_channel; i++) {
        input[i] = 1;
    }

    for (int i = 0; i < kernel_size * kernel_size * output_channel * input_channel; i++) {
        kernel[i] = 2;
    }

    // Perform im2col operation
    int col_index = 0;
    for (int oh = 0; oh < output_height; oh++) {
        for (int ow = 0; ow < output_width; ow++) {
            for (int ic = 0; ic < input_channel; ic++) {
                for (int kh = 0; kh < kernel_size; kh++) {
                    for (int kw = 0; kw < kernel_size; kw++) {
                        int ih = oh * stride - padding + kh;
                        int iw = ow * stride - padding + kw;
                        if (ih >= 0 && ih < input_height && iw >= 0 && iw < input_width) {
                            int input_index = ic * input_height * input_width + ih * input_width + iw;
                            im2col[col_index] = input[input_index];
                        } else {
                            im2col[col_index] = 0;
                        }
                        col_index++;
                    }
                }
            }
        }
    }

    // Perform convolution
    double start_time = MPI_Wtime();
    performConvolution(input, kernel, output, im2col, input_height, input_width, input_channel, kernel_size, stride, padding, rank, size, output_channel);
    double end_time = MPI_Wtime();
    double elapsed_time = end_time - start_time;

    // Reduce the output from all ranks
    double* reduced_output = (double*)malloc(output_height * output_width * output_channel * sizeof(double));
    MPI_Reduce(output, reduced_output, output_height * output_width * output_channel, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    // Print the output
    if (rank == 0) {
        printf("Elapsed Time (Rank %d): %.6f seconds\n", rank, elapsed_time);
        for (int chan = 0; chan < output_channel; chan++) {
            printf("Output (Channel %d):\n", chan);
            for (int outh = 0; outh < output_height; outh++) {
                for (int outw = 0; outw < output_width; outw++) {
                    int output_index = chan * output_height * output_width + outh * output_width + outw;
                    printf("%.1f ", reduced_output[output_index]);
                }
                printf("\n");
            }
            printf("\n");
        }
    }

    // Free memory
    free(input);
    free(kernel);
    free(output);
    free(reduced_output);
    free(im2col);

    MPI_Finalize();
    return 0;
}

