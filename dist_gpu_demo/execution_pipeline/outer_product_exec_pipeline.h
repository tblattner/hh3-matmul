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


#ifndef HH3_MATMUL_OUTER_PRODUCT_EXEC_PIPELINE_H
#define HH3_MATMUL_OUTER_PRODUCT_EXEC_PIPELINE_H

#include "../data/matrix_tile.h"
#include <hedgehog/hedgehog.h>

template<class MatrixType, char InpIdA, char InpIdB, char OutId, Order Ord,
    class MatrixTileA = MatrixTile<MatrixType, InpIdA, Ord>,
    class MatrixTileB = MatrixTile<MatrixType, InpIdB, Ord>,
    class MatrixTileP = MatrixTile<MatrixType, OutId, Ord>
>
class OuterProductExecPipeline:
    public hh::AbstractExecutionPipeline<2,
        MatrixTileA, //inp1
        MatrixTileB, //inp2
        MatrixTileP  //out1
    > {
public:
    explicit OuterProductExecPipeline(
        std::shared_ptr<hh::Graph<2, MatrixTileA, MatrixTileB, MatrixTileP>> const &graph,
        std::vector<int> const &deviceIds
    ): hh::AbstractExecutionPipeline<2, MatrixTileA, MatrixTileB, MatrixTileP>(graph, deviceIds) {
        deviceCount_ = deviceIds.size();
    }

    bool sendToGraph(std::shared_ptr<MatrixTileA> &matrixTileA, size_t const &graphId) override {
        return (matrixTileA->colIdx() % deviceCount_) == graphId;
    }

    bool sendToGraph(std::shared_ptr<MatrixTileB> &matrixTileB, size_t const &graphId) override {
        return (matrixTileB->rowIdx() % deviceCount_) == graphId;
    }

private:
    uint32_t deviceCount_ = 0;
};

#endif //HH3_MATMUL_OUTER_PRODUCT_EXEC_PIPELINE_H
