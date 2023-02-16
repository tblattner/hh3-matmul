// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the
// software in any medium, provided that you keep intact this entire notice. You may improve, modify and create
// derivative works of the software or any portion of the software, and you may copy and distribute such modifications
// or works. Modified works should carry a notice stating that you changed the software and should note the date and
// nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the
// source of the software. NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND,
// EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
// WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE
// CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS
// THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE. You
// are solely responsible for determining the appropriateness of using and distributing the software and you assume
// all risks associated with its use, including but not limited to the risks and costs of program errors, compliance
// with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of
// operation. This software is not intended to be used in any situation where a failure could cause risk of injury or
// damage to property. The software developed by NIST employees is not subject to copyright protection within the
// United States.


#ifndef HH3_MATMUL_CUDA_COPY_IN_GPU_TASK_H
#define HH3_MATMUL_CUDA_COPY_IN_GPU_TASK_H

#include "../data/cuda_matrix_tile.h"

template<class MatrixType, char Id, Order Ord>
class CudaCopyInGpuTask:
    public hh::AbstractCUDATask<1,
        MatrixTile<MatrixType, Id, Ord>,    //inp1
        CudaMatrixTile<MatrixType, Id, Ord> //out1
    > {
public:
    explicit CudaCopyInGpuTask(uint32_t threadCount):
        hh::AbstractCUDATask<1, MatrixTile<MatrixType, Id, Ord>, CudaMatrixTile<MatrixType, Id, Ord>>(
            "Cuda Copy in GPU Task",
            threadCount,
            false,
            false
        ) {}

    void execute(std::shared_ptr<MatrixTile<MatrixType, Id, Ord>> matrixTile) override {
        auto cudaMatrixTile = std::static_pointer_cast<CudaMatrixTile<MatrixType, Id, Ord>>(this->getManagedMemory());
        cudaMatrixTile->matrixTileMetaData(matrixTile->matrixTileMetaData());

        if(Ord == Order::Col) {
            if(matrixTile->leadingDimension() == matrixTile->height()) {
                // contiguous memory
                checkCudaErrors(cudaMemcpyAsync(
                    cudaMatrixTile->cudaMemory(),
                    reinterpret_cast<const void*>(matrixTile->data()),
                    sizeof(MatrixType)*matrixTile->height()*matrixTile->width(),
                    cudaMemcpyHostToDevice,
                    this->stream()
                ));
            }
            else {
                // non contiguous memory
                checkCudaErrors(cublasSetMatrixAsync(
                    int(matrixTile->height()), int(matrixTile->width()),
                    sizeof(MatrixType),
                    matrixTile->data(), int(matrixTile->leadingDimension()),
                    cudaMatrixTile->cudaMemory(), int(cudaMatrixTile->height()),
                    this->stream()
                ));
            }
        }
        else {
            assert(false);
        }

        cudaMatrixTile->recordEvent(this->stream());
        this->addResult(cudaMatrixTile);
    }

    std::shared_ptr<hh::AbstractTask<1, MatrixTile<MatrixType, Id, Ord>, CudaMatrixTile<MatrixType, Id, Ord>>>
    copy() override {
        return std::make_shared<CudaCopyInGpuTask>(this->numberThreads());
    }
};

#endif //HH3_MATMUL_CUDA_COPY_IN_GPU_TASK_H
