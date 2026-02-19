// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 GitHub Copilot

#include "HungarianAlgorithm.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace eicrecon {

std::vector<int> HungarianAlgorithm::solve(const Eigen::MatrixXd& cost_matrix) {
  const int rows = cost_matrix.rows();
  const int cols = cost_matrix.cols();
  
  if (rows == 0 || cols == 0) {
    return std::vector<int>(rows, -1);
  }
  
  // Make a square matrix by padding with large values
  const int n = std::max(rows, cols);
  const double inf = std::numeric_limits<double>::infinity();
  
  Eigen::MatrixXd cost = Eigen::MatrixXd::Constant(n, n, inf);
  cost.block(0, 0, rows, cols) = cost_matrix;
  
  // Step 1: Reduce rows
  reduceRows(cost);
  
  // Step 2: Reduce columns
  reduceColumns(cost);
  
  // Step 3 & 4: Cover zeros and create additional zeros until optimal
  std::vector<bool> row_covered(n, false);
  std::vector<bool> col_covered(n, false);
  
  const int max_iterations = 100 * n; // Safety limit
  int iterations = 0;
  
  while (iterations++ < max_iterations) {
    int num_lines = coverZeros(cost, row_covered, col_covered);
    
    if (num_lines >= n) {
      // Found optimal solution
      break;
    }
    
    // Create additional zeros
    createZeros(cost, row_covered, col_covered);
  }
  
  // Find the assignment
  std::vector<int> assignment = findAssignment(cost);
  
  // Truncate to original row count and mark out-of-bounds assignments
  assignment.resize(rows);
  for (int i = 0; i < rows; i++) {
    if (assignment[i] >= cols) {
      assignment[i] = -1;
    }
  }
  
  return assignment;
}

void HungarianAlgorithm::reduceRows(Eigen::MatrixXd& cost) {
  const int rows = cost.rows();
  const int cols = cost.cols();
  
  for (int i = 0; i < rows; i++) {
    double min_val = std::numeric_limits<double>::infinity();
    for (int j = 0; j < cols; j++) {
      if (std::isfinite(cost(i, j))) {
        min_val = std::min(min_val, cost(i, j));
      }
    }
    
    if (std::isfinite(min_val)) {
      for (int j = 0; j < cols; j++) {
        if (std::isfinite(cost(i, j))) {
          cost(i, j) -= min_val;
        }
      }
    }
  }
}

void HungarianAlgorithm::reduceColumns(Eigen::MatrixXd& cost) {
  const int rows = cost.rows();
  const int cols = cost.cols();
  
  for (int j = 0; j < cols; j++) {
    double min_val = std::numeric_limits<double>::infinity();
    for (int i = 0; i < rows; i++) {
      if (std::isfinite(cost(i, j))) {
        min_val = std::min(min_val, cost(i, j));
      }
    }
    
    if (std::isfinite(min_val)) {
      for (int i = 0; i < rows; i++) {
        if (std::isfinite(cost(i, j))) {
          cost(i, j) -= min_val;
        }
      }
    }
  }
}

int HungarianAlgorithm::coverZeros(const Eigen::MatrixXd& cost, std::vector<bool>& row_covered,
                                    std::vector<bool>& col_covered) {
  const int n = cost.rows();
  
  // Reset coverage
  std::fill(row_covered.begin(), row_covered.end(), false);
  std::fill(col_covered.begin(), col_covered.end(), false);
  
  // Create a preliminary assignment
  std::vector<int> row_assignment(n, -1);
  std::vector<int> col_assignment(n, -1);
  
  // Greedy assignment of zeros
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (cost(i, j) == 0 && row_assignment[i] == -1 && col_assignment[j] == -1) {
        row_assignment[i] = j;
        col_assignment[j] = i;
      }
    }
  }
  
  // Mark assigned columns
  for (int j = 0; j < n; j++) {
    if (col_assignment[j] != -1) {
      col_covered[j] = true;
    }
  }
  
  // Count covered lines (columns in this case)
  int num_covered = 0;
  for (int j = 0; j < n; j++) {
    if (col_covered[j]) {
      num_covered++;
    }
  }
  
  return num_covered;
}

void HungarianAlgorithm::createZeros(Eigen::MatrixXd& cost, const std::vector<bool>& row_covered,
                                      const std::vector<bool>& col_covered) {
  const int n = cost.rows();
  
  // Find minimum uncovered value
  double min_uncovered = std::numeric_limits<double>::infinity();
  for (int i = 0; i < n; i++) {
    if (!row_covered[i]) {
      for (int j = 0; j < n; j++) {
        if (!col_covered[j] && std::isfinite(cost(i, j))) {
          min_uncovered = std::min(min_uncovered, cost(i, j));
        }
      }
    }
  }
  
  if (!std::isfinite(min_uncovered) || min_uncovered == 0) {
    return;
  }
  
  // Add minimum to covered rows
  for (int i = 0; i < n; i++) {
    if (row_covered[i]) {
      for (int j = 0; j < n; j++) {
        if (std::isfinite(cost(i, j))) {
          cost(i, j) += min_uncovered;
        }
      }
    }
  }
  
  // Subtract minimum from uncovered columns
  for (int j = 0; j < n; j++) {
    if (!col_covered[j]) {
      for (int i = 0; i < n; i++) {
        if (std::isfinite(cost(i, j))) {
          cost(i, j) -= min_uncovered;
        }
      }
    }
  }
}

std::vector<int> HungarianAlgorithm::findAssignment(const Eigen::MatrixXd& cost) {
  const int n = cost.rows();
  std::vector<int> assignment(n, -1);
  
  // Use augmenting path method for maximum matching in bipartite graph
  for (int row = 0; row < n; row++) {
    std::vector<bool> visited(n, false);
    tryAssign(row, cost, assignment, visited);
  }
  
  return assignment;
}

bool HungarianAlgorithm::tryAssign(int row, const Eigen::MatrixXd& cost,
                                    std::vector<int>& assignment, std::vector<bool>& visited) {
  const int n = cost.cols();
  
  for (int col = 0; col < n; col++) {
    if (cost(row, col) == 0 && !visited[col]) {
      visited[col] = true;
      
      // Find which row is currently assigned to this column
      int assigned_row = -1;
      for (int r = 0; r < n; r++) {
        if (assignment[r] == col) {
          assigned_row = r;
          break;
        }
      }
      
      // If column is unassigned or we can reassign the current assignment
      if (assigned_row == -1 || tryAssign(assigned_row, cost, assignment, visited)) {
        assignment[row] = col;
        return true;
      }
    }
  }
  
  return false;
}

} // namespace eicrecon
