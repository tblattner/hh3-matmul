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


#ifndef HH3_MATMUL_CUDA_INPUT_BLOCK_STATE_H
#define HH3_MATMUL_CUDA_INPUT_BLOCK_STATE_H

#include "../data/cuda_matrix_block_data.h"

template<class MatrixType, Order Ord>
class CudaInputBlockState: public hh::State<2,
        CudaMatrixBlockData<MatrixType, 'a', Ord>,                                                                                          //inp1
        CudaMatrixBlockData<MatrixType, 'b', Ord>,                                                                                          //inp2
        std::pair<std::shared_ptr<CudaMatrixBlockData<MatrixType, 'a', Ord>>, std::shared_ptr<CudaMatrixBlockData<MatrixType, 'b', Ord>>>   //out1
    > {
private:
    size_t
            M_ = 0,
            K_ = 0,
            N_ = 0;

    std::vector<std::shared_ptr<CudaMatrixBlockData<MatrixType, 'a', Ord>>> gridMatrixA_ = {};
    std::vector<std::shared_ptr<CudaMatrixBlockData<MatrixType, 'b', Ord>>> gridMatrixB_ = {};
    std::vector<size_t> ttlA_ = {}, ttlB_ = {};

public:
    CudaInputBlockState(size_t M, size_t K, size_t N): M_(M), K_(K), N_(N) {
        gridMatrixA_ = std::vector<std::shared_ptr<CudaMatrixBlockData<MatrixType, 'a', Ord>>>(M_ * K_, nullptr);
        gridMatrixB_ = std::vector<std::shared_ptr<CudaMatrixBlockData<MatrixType, 'b', Ord>>>(N_ * K_, nullptr);
        ttlA_ = std::vector<size_t>(M_ * K_, N_);
        ttlB_ = std::vector<size_t>(N_ * K_, M_);
    }

    virtual ~CudaInputBlockState() = default;

    void execute([[maybe_unused]]std::shared_ptr<CudaMatrixBlockData<MatrixType, 'a', Ord>> ptr) override {
        matrixA(ptr);
        for(size_t jB = 0; jB < N_; ++jB) {
            if(auto bB = matrixB(ptr->colIdx(), jB)) {
                ttlA_[ptr->rowIdx() * K_ + ptr->colIdx()]--;
                if(ttlA_[ptr->rowIdx() * K_ + ptr->colIdx()] == 0) {
                    gridMatrixA_[ptr->rowIdx() * K_ + ptr->colIdx()] = nullptr;
                }
                auto res = std::make_shared<std::pair<std::shared_ptr<CudaMatrixBlockData<MatrixType, 'a', Ord>>,
                std::shared_ptr<CudaMatrixBlockData<MatrixType, 'b', Ord>>>>();
                res->first = ptr;
                res->second = bB;
                this->addResult(res);
            }
        }
    }

    void execute([[maybe_unused]]std::shared_ptr<CudaMatrixBlockData<MatrixType, 'b', Ord>> ptr) override {
        matrixB(ptr);
        for(size_t iA = 0; iA < M_; ++iA) {
            if(auto bA = matrixA(iA, ptr->rowIdx())) {
                ttlB_[ptr->rowIdx() * N_ + ptr->colIdx()]--;
                if(ttlB_[ptr->rowIdx() * N_ + ptr->colIdx()] == 0) {
                    gridMatrixB_[ptr->rowIdx() * N_ + ptr->colIdx()] = nullptr;
                }
                auto res = std::make_shared<std::pair<std::shared_ptr<CudaMatrixBlockData<MatrixType, 'a', Ord>>,
                std::shared_ptr<CudaMatrixBlockData<MatrixType, 'b', Ord>>>>();
                res->first = bA;
                res->second = ptr;
                this->addResult(res);
            }
        }
    }

private:
    std::shared_ptr<CudaMatrixBlockData<MatrixType, 'a', Ord>> matrixA(size_t i, size_t j) {
        std::shared_ptr<CudaMatrixBlockData<MatrixType, 'a', Ord>> res = nullptr;
        if ((res = gridMatrixA_[i * K_ + j])) {
            ttlA_[i * K_ + j] = ttlA_[i * K_ + j] - 1;
            if (ttlA_[i * K_ + j] == 0) {
                gridMatrixA_[i * K_ + j] = nullptr;
            }
        }
        return res;
    }

    std::shared_ptr<CudaMatrixBlockData<MatrixType, 'b', Ord>> matrixB(size_t i, size_t j) {
        std::shared_ptr<CudaMatrixBlockData<MatrixType, 'b', Ord>> res = nullptr;
        if ((res = gridMatrixB_[i * N_ + j])) {
            ttlB_[i * N_ + j] = ttlB_[i * N_ + j] - 1;
            if (ttlB_[i * N_ + j] == 0) { gridMatrixB_[i * N_ + j] = nullptr; }
        }
        return res;
    }

    void matrixA(std::shared_ptr<CudaMatrixBlockData<MatrixType, 'a', Ord>> blockA) {
        gridMatrixA_[blockA->rowIdx() * K_ + blockA->colIdx()] = blockA;
    }

    void matrixB(std::shared_ptr<CudaMatrixBlockData<MatrixType, 'b', Ord>> blockB) {
        gridMatrixB_[blockB->rowIdx() * N_ + blockB->colIdx()] = blockB;
    }

    public:
    friend std::ostream &operator<<(std::ostream &os, const CudaInputBlockState &state) {
        os << "State Input Block: " << std::endl;
        os << "Grid Matrix A" << std::endl;
        for (size_t i = 0; i < state.M_; ++i) {
            for (size_t j = 0; j < state.K_; ++j) {
                os << std::setw(14) << state.gridMatrixA_[i * state.K_ + j] << ", ";
            }
            os << std::endl;
        }
        os << "TTL Matrix A" << std::endl;
        for (size_t i = 0; i < state.M_; ++i) {
            for (size_t j = 0; j < state.K_; ++j) {
                os << state.ttlA_[i * state.K_ + j] << ", ";
            }
            os << std::endl;
        }
        os << "Grid Matrix B" << std::endl;
        for (size_t i = 0; i < state.K_; ++i) {
            for (size_t j = 0; j < state.N_; ++j) {
                os << std::setw(14) << state.gridMatrixB_[i * state.N_ + j] << ", ";
            }
            os << std::endl;
        }
        os << "TTL Matrix B" << std::endl;
        for (size_t i = 0; i < state.K_; ++i) {
            for (size_t j = 0; j < state.N_; ++j) {
                os << state.ttlB_[i * state.N_ + j] << ", ";
            }
            os << std::endl;
        }
        return os;
    }
};

#endif //HH3_MATMUL_CUDA_INPUT_BLOCK_STATE_H
