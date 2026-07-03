#include <gtest/gtest.h>
#include "SimModel/Rcm.h"
#include <ErrorData.h>
#include <set>

using namespace SimModelNative;

// Provide access to the protected createAdjacencyInfo method via public inheritance.
class RcmExtender : public Rcm
{
public:
   void createAdjacencyInfo(std::vector<std::vector<bool>>& matrix, int& adjacencyNumber, int*& adj, int*& adj_row)
   {
      Rcm::createAdjacencyInfo(matrix, adjacencyNumber, adj, adj_row);
   }
};

class when_testing_rcm : public ::testing::Test
{
protected:
   Rcm rcm;

   std::vector<std::vector<bool>> createMatrix(int n) const
   {
      return std::vector<std::vector<bool>>(n, std::vector<bool>(n, false));
   }

   // The 10x10 example documented in Rcm.h, also used by the original C++/CLI specs.
   std::vector<std::vector<bool>> createTenNodeMatrix() const
   {
      auto matrix = createMatrix(10);
      matrix[0][3] = true; matrix[0][5] = true;
      matrix[1][2] = true; matrix[1][4] = true; matrix[1][6] = true; matrix[1][9] = true;
      matrix[2][1] = true; matrix[2][3] = true; matrix[2][4] = true;
      matrix[3][0] = true; matrix[3][2] = true; matrix[3][5] = true; matrix[3][8] = true;
      matrix[4][1] = true; matrix[4][2] = true; matrix[4][6] = true;
      matrix[5][0] = true; matrix[5][3] = true; matrix[5][6] = true; matrix[5][7] = true;
      matrix[6][1] = true; matrix[6][4] = true; matrix[6][5] = true; matrix[6][7] = true;
      matrix[7][5] = true; matrix[7][6] = true;
      matrix[8][3] = true;
      matrix[9][1] = true;
      return matrix;
   }

   void verifyPermutation(const std::vector<unsigned int>& perm, unsigned int n) const
   {
      ASSERT_EQ(n, perm.size());
      std::set<unsigned int> seen;
      for (auto idx : perm)
      {
         EXPECT_LT(idx, n);
         seen.insert(idx);
      }
      EXPECT_EQ(n, seen.size());
   }
};

TEST_F(when_testing_rcm, should_return_empty_permutation_for_empty_matrix)
{
   auto matrix = createMatrix(0);
   auto perm = rcm.GenRcm(matrix);
   EXPECT_TRUE(perm.empty());
}

TEST_F(when_testing_rcm, should_return_trivial_permutation_for_single_node)
{
   auto matrix = createMatrix(1);
   auto perm = rcm.GenRcm(matrix);

   ASSERT_EQ(1u, perm.size());
   EXPECT_EQ(0u, perm[0]);
}

TEST_F(when_testing_rcm, should_return_valid_permutation_for_two_disconnected_nodes)
{
   auto matrix = createMatrix(2);
   auto perm = rcm.GenRcm(matrix);
   verifyPermutation(perm, 2);
}

TEST_F(when_testing_rcm, should_return_valid_permutation_for_two_connected_nodes)
{
   auto matrix = createMatrix(2);
   matrix[0][1] = true;
   matrix[1][0] = true;

   auto perm = rcm.GenRcm(matrix);
   verifyPermutation(perm, 2);
}

TEST_F(when_testing_rcm, should_return_valid_permutation_for_line_graph)
{
   // 0 -- 1 -- 2 -- 3
   auto matrix = createMatrix(4);
   matrix[0][1] = true; matrix[1][0] = true;
   matrix[1][2] = true; matrix[2][1] = true;
   matrix[2][3] = true; matrix[3][2] = true;

   auto perm = rcm.GenRcm(matrix);
   verifyPermutation(perm, 4);
}

TEST_F(when_testing_rcm, should_return_valid_permutation_for_ten_node_example)
{
   // The 10x10 example from the header documentation
   auto matrix = createTenNodeMatrix();

   auto perm = rcm.GenRcm(matrix);
   verifyPermutation(perm, 10);
}

TEST_F(when_testing_rcm, should_handle_directed_graph_by_making_it_symmetric)
{
   // Provide a directed (non-symmetric) adjacency and verify GenRcm handles it
   auto matrix = createMatrix(3);
   matrix[0][1] = true;  // 0->1 only (not symmetric)
   matrix[1][2] = true;  // 1->2 only

   auto perm = rcm.GenRcm(matrix);
   verifyPermutation(perm, 3);
}

TEST_F(when_testing_rcm, should_return_valid_permutation_for_complete_graph)
{
   auto matrix = createMatrix(4);
   for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
         if (i != j)
            matrix[i][j] = true;

   auto perm = rcm.GenRcm(matrix);
   verifyPermutation(perm, 4);
}

TEST_F(when_testing_rcm, should_return_valid_permutation_for_star_graph)
{
   // Node 0 connected to all others; others not connected to each other
   auto matrix = createMatrix(5);
   for (int i = 1; i < 5; i++)
   {
      matrix[0][i] = true;
      matrix[i][0] = true;
   }

   auto perm = rcm.GenRcm(matrix);
   verifyPermutation(perm, 5);
}

// --- Migrated from the original C++/CLI RcmSpecs.cpp ---------------------------

TEST_F(when_testing_rcm, should_create_proper_rcm_inputs)
{
   auto matrix = createTenNodeMatrix();

   RcmExtender rcmExtender;
   int adjacencyNumber = 0;
   int* adj = nullptr;
   int* adj_row = nullptr;

   rcmExtender.createAdjacencyInfo(matrix, adjacencyNumber, adj, adj_row);

   // expected outputs for the adjacency info (1-based indices)
   const int adj_save[28] = {
      4, 6,
      3, 5, 7, 10,
      2, 4, 5,
      1, 3, 6, 9,
      2, 3, 7,
      1, 4, 7, 8,
      2, 5, 6, 8,
      6, 7,
      4,
      2};

   const int adj_row_save[11] = {1, 3, 7, 10, 14, 17, 21, 25, 27, 28, 29};

   EXPECT_EQ(28, adjacencyNumber);

   for (int i = 0; i < 28; i++)
      EXPECT_EQ(adj_save[i], adj[i]);

   for (int i = 0; i < 11; i++)
      EXPECT_EQ(adj_row_save[i], adj_row[i]);

   delete[] adj;
   delete[] adj_row;
}

TEST_F(when_testing_rcm, should_return_correct_permutation_for_ten_node_example)
{
   auto matrix = createTenNodeMatrix();

   auto permutation = rcm.GenRcm(matrix);

   const unsigned int permutation_save[10] = {8, 0, 7, 5, 3, 6, 4, 2, 1, 9};

   ASSERT_EQ(10u, permutation.size());
   for (int i = 0; i < 10; i++)
      EXPECT_EQ(permutation_save[i], permutation[i]);
}

