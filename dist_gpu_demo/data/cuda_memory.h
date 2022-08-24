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


#ifndef HH3_MATMUL_CUDA_MEMORY_H
#define HH3_MATMUL_CUDA_MEMORY_H

#include <hedgehog/hedgehog.h>

/**
 * Wrapper to allocate/deallocate cuda memory and have the functionality of synchronizing using streams and events.
 * Inherits: hh::ManagedMemory to enable this class be managed by hedgehog's memory manager.
 *
 */
class CudaMemory: public hh::ManagedMemory {
public:
    explicit CudaMemory(uint32_t bytes) {
        checkCudaErrors(cudaMalloc(&pCudaMemory_, bytes));
    }

    ~CudaMemory() {
        checkCudaErrors(cudaFree(pCudaMemory_));
        pCudaMemory_ = nullptr;
        if(cudaEventCreated_) {
            checkCudaErrors(cudaEventDestroy(cudaEvent_));
            cudaEventCreated_ = false;
        }
    }

    /**
     * Record cudaAsync API call using stream for later synchronization.
     *
     * @param stream
     */
    void recordEvent(cudaStream_t stream) {
        if (!cudaEventCreated_) {
            checkCudaErrors(cudaEventCreate(&cudaEvent_));
            cudaEventCreated_ = true;
        }
        checkCudaErrors(cudaEventRecord(cudaEvent_, stream));
    }

    /**
     * Synchronize the cudaAsync API called previously.
     */
    void synchronizeEvent() {
#if not NDEBUG
        if(!cudaEventCreated_) {
            throw std::runtime_error("cudaEvent_t was never created to be synchronized.");
        }
#endif
        checkCudaErrors(cudaEventSynchronize(cudaEvent_));
    }

    // Getters/Setters
    void* getCudaMemory() { return pCudaMemory_; }
    void** getCudaMemoryAddress() { return &pCudaMemory_; }

private:
    bool cudaEventCreated_ = false;
    cudaEvent_t cudaEvent_ = {};
    void* pCudaMemory_     = nullptr;
};

#endif //HH3_MATMUL_CUDA_MEMORY_H