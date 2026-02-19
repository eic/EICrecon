// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2025 GitHub Copilot

#include <Eigen/Core>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <vector>

#include "algorithms/reco/HungarianAlgorithm.h"

using Catch::Matchers::WithinAbs;

TEST_CASE("Hungarian algorithm solves simple assignment", "[HungarianAlgorithm]") {
  eicrecon::HungarianAlgorithm algo;

  SECTION("2x2 matrix - basic case") {
    Eigen::MatrixXd cost(2, 2);
    cost << 1.0, 2.0,
            2.0, 1.0;
    
    auto assignment = algo.solve(cost);
    
    REQUIRE(assignment.size() == 2);
    REQUIRE(assignment[0] == 0);
    REQUIRE(assignment[1] == 1);
    
    // Total cost should be 1 + 1 = 2
    double total_cost = cost(0, assignment[0]) + cost(1, assignment[1]);
    REQUIRE_THAT(total_cost, WithinAbs(2.0, 1e-9));
  }

  SECTION("3x3 matrix - optimal assignment") {
    Eigen::MatrixXd cost(3, 3);
    cost << 1.0, 2.0, 3.0,
            2.0, 4.0, 6.0,
            3.0, 6.0, 9.0;
    
    auto assignment = algo.solve(cost);
    
    REQUIRE(assignment.size() == 3);
    // Should assign: 0->0, 1->1, 2->2 with cost 1+4+9=14
    // or 0->0, 1->2, 2->1 with cost 1+6+6=13 (not minimal)
    // or 0->1, 1->0, 2->2 with cost 2+2+9=13 (not minimal)
    // or 0->2, 1->1, 2->0 with cost 3+4+3=10 (minimal)
    // or 0->1, 1->2, 2->0 with cost 2+6+3=11 (not minimal)
    // or 0->2, 1->0, 2->1 with cost 3+2+6=11 (not minimal)
    
    double total_cost = cost(0, assignment[0]) + cost(1, assignment[1]) + cost(2, assignment[2]);
    REQUIRE_THAT(total_cost, WithinAbs(10.0, 1e-9));
  }

  SECTION("Different-sized matrix (3 rows x 2 cols)") {
    Eigen::MatrixXd cost(3, 2);
    cost << 1.0, 4.0,
            2.0, 3.0,
            5.0, 6.0;
    
    auto assignment = algo.solve(cost);
    
    REQUIRE(assignment.size() == 3);
    // Best assignment: row 0->col 0 (cost 1), row 1->col 1 (cost 3), row 2->unassigned
    // One row will be unassigned (marked as -1)
    
    int assigned_count = 0;
    double total_cost = 0.0;
    for (size_t i = 0; i < assignment.size(); i++) {
      if (assignment[i] != -1) {
        assigned_count++;
        total_cost += cost(i, assignment[i]);
      }
    }
    
    REQUIRE(assigned_count == 2);
    REQUIRE_THAT(total_cost, WithinAbs(4.0, 1e-9)); // 1 + 3 = 4
  }

  SECTION("Different-sized matrix (2 rows x 3 cols)") {
    Eigen::MatrixXd cost(2, 3);
    cost << 1.0, 2.0, 3.0,
            4.0, 5.0, 6.0;
    
    auto assignment = algo.solve(cost);
    
    REQUIRE(assignment.size() == 2);
    REQUIRE(assignment[0] == 0);  // row 0 -> col 0 (cost 1)
    REQUIRE(assignment[1] == 1);  // row 1 -> col 1 (cost 5)
    
    double total_cost = cost(0, assignment[0]) + cost(1, assignment[1]);
    REQUIRE_THAT(total_cost, WithinAbs(6.0, 1e-9));
  }

  SECTION("Empty matrix") {
    Eigen::MatrixXd cost(0, 0);
    auto assignment = algo.solve(cost);
    REQUIRE(assignment.empty());
  }

  SECTION("Matrix with equal costs") {
    Eigen::MatrixXd cost(2, 2);
    cost << 1.0, 1.0,
            1.0, 1.0;
    
    auto assignment = algo.solve(cost);
    
    REQUIRE(assignment.size() == 2);
    REQUIRE(assignment[0] != -1);
    REQUIRE(assignment[1] != -1);
    REQUIRE(assignment[0] != assignment[1]); // Different assignments
    
    double total_cost = cost(0, assignment[0]) + cost(1, assignment[1]);
    REQUIRE_THAT(total_cost, WithinAbs(2.0, 1e-9));
  }

  SECTION("Practical track-cluster matching example") {
    // Simulate distances between 3 clusters and 3 tracks
    // Cluster 0 is closest to track 1
    // Cluster 1 is closest to track 0
    // Cluster 2 is closest to track 2
    Eigen::MatrixXd distances(3, 3);
    distances << 0.5, 0.1, 0.9,  // Cluster 0: closest to track 1
                 0.2, 0.8, 0.7,  // Cluster 1: closest to track 0
                 0.6, 0.9, 0.05; // Cluster 2: closest to track 2
    
    auto assignment = algo.solve(distances);
    
    REQUIRE(assignment.size() == 3);
    REQUIRE(assignment[0] == 1);  // Cluster 0 -> Track 1 (distance 0.1)
    REQUIRE(assignment[1] == 0);  // Cluster 1 -> Track 0 (distance 0.2)
    REQUIRE(assignment[2] == 2);  // Cluster 2 -> Track 2 (distance 0.05)
    
    double total_distance = distances(0, assignment[0]) + 
                           distances(1, assignment[1]) + 
                           distances(2, assignment[2]);
    REQUIRE_THAT(total_distance, WithinAbs(0.35, 1e-9)); // 0.1 + 0.2 + 0.05
  }

  SECTION("Order-independence test") {
    // Original order
    Eigen::MatrixXd cost1(3, 3);
    cost1 << 0.5, 0.1, 0.9,
             0.2, 0.8, 0.7,
             0.6, 0.9, 0.05;
    
    // Different row order (swapped rows 0 and 2)
    Eigen::MatrixXd cost2(3, 3);
    cost2 << 0.6, 0.9, 0.05,
             0.2, 0.8, 0.7,
             0.5, 0.1, 0.9;
    
    auto assignment1 = algo.solve(cost1);
    auto assignment2 = algo.solve(cost2);
    
    // Calculate total costs
    double total1 = cost1(0, assignment1[0]) + cost1(1, assignment1[1]) + cost1(2, assignment1[2]);
    double total2 = cost2(0, assignment2[0]) + cost2(1, assignment2[1]) + cost2(2, assignment2[2]);
    
    // Both should achieve the same minimum total cost
    REQUIRE_THAT(total1, WithinAbs(total2, 1e-9));
    REQUIRE_THAT(total1, WithinAbs(0.35, 1e-9));
  }
}
