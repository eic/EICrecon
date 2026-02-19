// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 GitHub Copilot

#pragma once

#include <Eigen/Core>
#include <algorithm>
#include <limits>
#include <vector>

namespace eicrecon {

/**
 * @brief Hungarian algorithm for solving the linear assignment problem
 * 
 * Finds the minimum cost assignment of rows to columns in a cost matrix.
 * Allows for rectangular matrices (different numbers of rows and columns).
 * Uses the Kuhn-Munkres algorithm implementation.
 * 
 * Time complexity: O(n^3) where n is max(rows, cols)
 * Space complexity: O(n^2)
 * 
 * @note For typical track-cluster matching use cases (n < 1000), this implementation
 *       is efficient and safe. Very large matrices (n > 10000) may encounter stack
 *       depth limitations due to the recursive augmenting path algorithm, but would
 *       also be prohibitively slow due to O(n^3) complexity.
 */
class HungarianAlgorithm {
public:
  /**
   * @brief Solve the assignment problem
   * 
   * @param cost_matrix Input cost matrix (rows x cols)
   * @return Vector of column assignments for each row (-1 if unassigned)
   * 
   * Example:
   *   cost_matrix = [[1, 2],
   *                  [2, 1]]
   *   returns [0, 1] (row 0 assigned to col 0, row 1 assigned to col 1)
   */
  static std::vector<int> solve(const Eigen::MatrixXd& cost_matrix);

private:
  // Step 1: Subtract row minima
  static void reduceRows(Eigen::MatrixXd& cost);
  
  // Step 2: Subtract column minima
  static void reduceColumns(Eigen::MatrixXd& cost);
  
  // Step 3: Find minimum number of lines to cover all zeros
  static int coverZeros(const Eigen::MatrixXd& cost, std::vector<bool>& row_covered,
                        std::vector<bool>& col_covered);
  
  // Step 4: Create additional zeros
  static void createZeros(Eigen::MatrixXd& cost, const std::vector<bool>& row_covered,
                          const std::vector<bool>& col_covered);
  
  // Helper: Find optimal assignment from zero positions
  static std::vector<int> findAssignment(const Eigen::MatrixXd& cost);
  
  // Helper: Try to assign a row using DFS
  static bool tryAssign(int row, const Eigen::MatrixXd& cost, std::vector<int>& assignment,
                        std::vector<bool>& visited);
};

} // namespace eicrecon
