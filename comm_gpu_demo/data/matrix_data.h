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


#ifndef HH3_MATMUL_MATRIX_DATA_H
#define HH3_MATMUL_MATRIX_DATA_H

#include <cereal/archives/binary.hpp>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <cassert>
#include "data_type.h"
#include "serialization.h"


/**
 * Stores just the meta data corresponding the matrix
 * Doesn't allocate new memory for the matrix data
 *
 * @tparam Type
 * @tparam Id
 * @tparam Ord
 */
template <class Type, char Id, Order Ord>
class MatrixData: public Serialization {
private:
    size_t matrixHeight_ = 0;
    size_t matrixWidth_ = 0;
    size_t blockSize_ = 0;
    size_t numBlocksRows_ = 0;
    size_t numBlocksCols_ = 0;
    size_t leadingDimension_ = 0;
    Type *matrixData_ = nullptr;
    bool ownMemory_ = false;

public:
    explicit MatrixData() = default;

    explicit MatrixData(size_t matrixHeight, size_t matrixWidth, size_t blockSize, Type &matrixData, bool transferOwnerShip = false):
        matrixHeight_(matrixHeight), matrixWidth_(matrixWidth), blockSize_(blockSize),
        numBlocksRows_(std::ceil(matrixHeight_ / blockSize_) + (matrixHeight_ % blockSize_ == 0 ? 0: 1)),
        numBlocksCols_(std::ceil(matrixWidth_ / blockSize_) + (matrixWidth_ % blockSize_ == 0 ? 0: 1)),
        matrixData_(&matrixData),
        ownMemory_(transferOwnerShip) {

        if(blockSize_ == 0) {
            blockSize_ = 1;
        }

        if(matrixHeight_ == 0 || matrixWidth_ == 0) {
            std::cout << "Can't compute an empty matrix" << std::endl;
        }

        if constexpr(Ord == Order::Row) {
            this->leadingDimension_ = matrixWidth_;
        } else{
            this->leadingDimension_ = matrixHeight_;
        }
    }

    ~MatrixData() {
        if(ownMemory_ and matrixData_ != nullptr) {
            delete[] matrixData_;
            matrixData_ = nullptr;
        }
    }

    MatrixData(const MatrixData&) = delete;
    MatrixData& operator=(const MatrixData&) = delete;
    MatrixData(MatrixData&&) noexcept = delete;
    MatrixData& operator=(MatrixData&&) noexcept = delete;

    [[nodiscard]] size_t matrixHeight() const { return matrixHeight_; }
    [[nodiscard]] size_t matrixWidth() const { return matrixWidth_; }
    [[nodiscard]] size_t blockSize() const { return blockSize_; }
    [[nodiscard]] size_t numBlocksRows() const { return numBlocksRows_; }
    [[nodiscard]] size_t numBlocksCols() const { return numBlocksCols_; }
    [[nodiscard]] size_t leadingDimension() const { return leadingDimension_; }
    Type *matrixData() const { return matrixData_; }
    Type* data() const { return matrixData_; }
    Type* data() { return matrixData_; }
    size_t sizeInBytes() const { return sizeof(Type) * matrixWidth_ * matrixHeight_; }

    std::string serialize() const override {
        // FIXME: ss.str() copies data
        std::stringstream ss;
        cereal::BinaryOutputArchive ar(ss);
        ar(cereal::binary_data(&matrixWidth_, sizeof(matrixWidth_)));
        ar(cereal::binary_data(&matrixHeight_, sizeof(matrixHeight_)));
        ar(cereal::binary_data(&blockSize_, sizeof(blockSize_)));
        ar(cereal::binary_data(&numBlocksRows_, sizeof(numBlocksRows_)));
        ar(cereal::binary_data(&numBlocksCols_, sizeof(numBlocksCols_)));
        ar(cereal::binary_data(&leadingDimension_, sizeof(leadingDimension_)));
        ar(cereal::binary_data(matrixData_, sizeInBytes()));
        return std::move(ss).str();
    }

    void deserialize(std::istream &&istream) override {
        cereal::BinaryInputArchive ar(istream);
        ar(cereal::binary_data(&matrixWidth_, sizeof(matrixWidth_)));
        ar(cereal::binary_data(&matrixHeight_, sizeof(matrixHeight_)));
        ar(cereal::binary_data(&blockSize_, sizeof(blockSize_)));
        ar(cereal::binary_data(&numBlocksRows_, sizeof(numBlocksRows_)));
        ar(cereal::binary_data(&numBlocksCols_, sizeof(numBlocksCols_)));
        ar(cereal::binary_data(&leadingDimension_, sizeof(leadingDimension_)));
        matrixData_ = new Type[matrixWidth_*matrixHeight_];
        ownMemory_ = true;
        ar(cereal::binary_data(matrixData_, sizeInBytes()));
    }

    friend std::ostream &operator<<(std::ostream &os, const MatrixData &data) {
        os << "MatrixData " << Id
            << " size: (" << data.matrixHeight() << ", " << data.matrixWidth() << ")"
            << " size Grid: (" << data.numBlocksRows() << ", " << data.numBlocksCols() << ")"
            << " blockSize: " << data.blockSize() << " leading Dimension: " << data.leadingDimension()
            << std::endl;

        if constexpr(Ord == Order::Row) {
            for(size_t i = 0; i < data.matrixHeight(); ++i) {
                for(size_t j = 0; j < data.matrixWidth(); ++j) {
                    os << std::setprecision(std::numeric_limits<Type >::digits10 + 1)
                        << data.matrixData_[i * data.leadingDimension() + j] << " ";
                }
                os << std::endl;
            }
        } else {
            for(size_t i = 0; i < data.matrixHeight(); ++i) {
                for(size_t j = 0; j < data.matrixWidth(); ++j) {
                    os << std::setprecision(std::numeric_limits<Type >::digits10 + 1)
                        << data.matrixData_[j * data.leadingDimension() + i] << " ";
                }
                os << std::endl;
            }
        }
        os << std::endl;

        return os;
    }
};

#endif //HH3_MATMUL_MATRIX_DATA_H
