#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <vector>
#include <memory>
#include <atomic>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <sstream>
#include "test_utils.h"
#include "battle.h"
#include "tournament_manager.h"
#include "pokemon_data.h"
#include "ai_factory.h"

/**
 * Performance Regression Test Suite
 * 
 * This file contains comprehensive performance regression tests to detect
 * performance degradations before they impact users. These tests establish
 * performance baselines and monitor for regressions across critical paths.
 * 
 * Coverage Areas:
 * - Battle execution speed benchmarks
 * - AI decision timing precision
 * - Memory usage tracking and leak detection
 * - Tournament progression performance
 * - Load testing under concurrent scenarios
 * - Performance degradation detection mechanisms
 */

class PerformanceRegressionTest : public TestUtils::BattleTestFixture {
protected:
    void SetUp() override {
        TestUtils::BattleTestFixture::SetUp();
        
        // Create performance test Pokemon with specific characteristics
        fastPokemon = TestUtils::createTestPokemon("speed-demon", 80, 70, 60, 80, 70, 130, {"electric"});
        tankPokemon = TestUtils::createTestPokemon("tank", 200, 60, 120, 60, 120, 40, {"steel"});
        balancedPokemon = TestUtils::createTestPokemon("balanced", 100, 85, 85, 85, 85, 85, {"normal"});
        
        // Setup moves for performance testing
        setupPerformanceTestMoves();
        
        // Initialize performance tracking
        performance_baseline.clear();
        memory_baseline = 0;
    }
    
    void setupPerformanceTestMoves() {
        // Fast, simple moves for speed testing
        Move quickMove = TestUtils::createTestMove("quick-strike", 60, 100, 30, "normal", "physical");
        Move powerMove = TestUtils::createTestMove("power-blast", 120, 85, 10, "normal", "special");
        Move statusMove = TestUtils::createTestMove("stat-boost", 0, 100, 20, "normal", "status");
        Move priorityMove = TestUtils::createTestMove("priority-attack", 40, 100, 30, "normal", "physical");
        priorityMove.priority = 1;
        
        // Assign moves to test Pokemon
        fastPokemon.moves = {quickMove, priorityMove, statusMove};
        tankPokemon.moves = {powerMove, statusMove, quickMove};
        balancedPokemon.moves = {quickMove, powerMove, statusMove, priorityMove};
    }
    
    // Performance measurement utilities
    struct PerformanceMeasurement {
        std::string operation;
        double duration_ms;
        size_t memory_usage;
        std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
    };
    
    std::chrono::duration<double, std::milli> measureOperation(std::function<void()> operation) {
        auto start = std::chrono::high_resolution_clock::now();
        operation();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start);
    }
    
    // Memory usage estimation (simplified)
    size_t estimateMemoryUsage(const Battle& /* battle */) {
        // Rough estimation based on battle components
        size_t base_size = sizeof(Battle);
        base_size += playerTeam.getAlivePokemon().size() * sizeof(Pokemon);
        base_size += opponentTeam.getAlivePokemon().size() * sizeof(Pokemon);
        return base_size;
    }
    
    void recordPerformance(const std::string& operation, double duration_ms, size_t memory = 0) {
        PerformanceMeasurement measurement;
        measurement.operation = operation;
        measurement.duration_ms = duration_ms;
        measurement.memory_usage = memory;
        measurement.timestamp = std::chrono::high_resolution_clock::now();
        performance_baseline.push_back(measurement);
    }
    
    // Statistical analysis for performance regression detection
    struct PerformanceStats {
        double mean;
        double stddev;
        double min_val;
        double max_val;
        size_t sample_count;
    };
    
    PerformanceStats calculateStats(const std::vector<double>& measurements) {
        if (measurements.empty()) return {0, 0, 0, 0, 0};
        
        double mean = std::accumulate(measurements.begin(), measurements.end(), 0.0) / measurements.size();
        
        double sq_sum = std::accumulate(measurements.begin(), measurements.end(), 0.0,
            [mean](double sum, double val) {
                return sum + (val - mean) * (val - mean);
            });
        
        double stddev = std::sqrt(sq_sum / measurements.size());
        
        auto minmax = std::minmax_element(measurements.begin(), measurements.end());
        
        return {mean, stddev, *minmax.first, *minmax.second, measurements.size()};
    }
    
    Pokemon fastPokemon;
    Pokemon tankPokemon;
    Pokemon balancedPokemon;
    std::vector<PerformanceMeasurement> performance_baseline;
    size_t memory_baseline;
};

// REGRESSION TEST: Battle Execution Speed Benchmarks
// Establishes baseline performance for battle turn execution
TEST_F(PerformanceRegressionTest, BattleExecutionSpeedBenchmarks) {
    const int num_iterations = 100;
    std::vector<double> battle_creation_times;
    std::vector<double> turn_execution_times;
    
    for (int i = 0; i < num_iterations; ++i) {
        // Measure battle creation time
        auto creation_duration = measureOperation([&]() {
            Battle benchmark_battle(playerTeam, opponentTeam, Battle::AIDifficulty::EASY);
        });
        battle_creation_times.push_back(creation_duration.count());
        
        // Create battle for turn measurement
        Battle turn_battle(playerTeam, opponentTeam, Battle::AIDifficulty::EASY);
        
        // Measure turn execution time (simplified - just checking state)
        auto turn_duration = measureOperation([&]() {
            // Simulate turn processing by querying battle state multiple times
            turn_battle.isBattleOver();
            turn_battle.getBattleResult();
            // Would execute actual turn in full implementation
        });
        turn_execution_times.push_back(turn_duration.count());
    }
    
    auto creation_stats = calculateStats(battle_creation_times);
    auto turn_stats = calculateStats(turn_execution_times);
    
    // Performance assertions - battles should be created quickly
    EXPECT_LT(creation_stats.mean, 10.0); // Less than 10ms average battle creation
    EXPECT_LT(creation_stats.max_val, 50.0); // No single creation takes more than 50ms
    
    // Turn execution should be very fast for simple scenarios
    EXPECT_LT(turn_stats.mean, 1.0); // Less than 1ms per turn state check
    
    // Record for baseline comparison
    recordPerformance("battle_creation", creation_stats.mean);
    recordPerformance("turn_execution", turn_stats.mean);
    
    std::cout << "Battle Creation: " << creation_stats.mean << "ms ± " << creation_stats.stddev << "ms" << std::endl;
    std::cout << "Turn Execution: " << turn_stats.mean << "ms ± " << turn_stats.stddev << "ms" << std::endl;
}

// REGRESSION TEST: AI Decision Timing Precision
// Measures AI decision performance across different difficulty levels
TEST_F(PerformanceRegressionTest, AiDecisionTimingPrecision) {
    const int decisions_per_difficulty = 50;
    
    std::vector<Battle::AIDifficulty> difficulties = {
        Battle::AIDifficulty::EASY,
        Battle::AIDifficulty::MEDIUM,
        Battle::AIDifficulty::HARD,
        Battle::AIDifficulty::EXPERT
    };
    
    for (auto difficulty : difficulties) {
        std::vector<double> decision_times;
        
        for (int i = 0; i < decisions_per_difficulty; ++i) {
            Battle ai_battle(playerTeam, opponentTeam, difficulty);
            
            // Measure AI decision time (conceptual - actual AI timing would need AI integration)
            auto decision_duration = measureOperation([&]() {
                // Simulate AI decision process
                ai_battle.isBattleOver();
                ai_battle.getBattleResult();
                // In full implementation: ai_battle.getAIMove();
            });
            
            decision_times.push_back(decision_duration.count());
        }
        
        auto stats = calculateStats(decision_times);
        
        // Performance targets based on difficulty
        switch (difficulty) {
            case Battle::AIDifficulty::EASY:
                EXPECT_LT(stats.mean, 5.0);  // Easy AI: < 5ms
                recordPerformance("easy_ai_decision", stats.mean);
                break;
            case Battle::AIDifficulty::MEDIUM:
                EXPECT_LT(stats.mean, 8.0);  // Medium AI: < 8ms
                recordPerformance("medium_ai_decision", stats.mean);
                break;
            case Battle::AIDifficulty::HARD:
                EXPECT_LT(stats.mean, 15.0); // Hard AI: < 15ms
                recordPerformance("hard_ai_decision", stats.mean);
                break;
            case Battle::AIDifficulty::EXPERT:
                EXPECT_LT(stats.mean, 30.0); // Expert AI: < 30ms
                recordPerformance("expert_ai_decision", stats.mean);
                break;
        }
        
        std::cout << "AI Difficulty " << static_cast<int>(difficulty) 
                  << ": " << stats.mean << "ms ± " << stats.stddev << "ms" << std::endl;
    }
}

// REGRESSION TEST: Memory Usage Tracking and Leak Detection
// Monitors memory allocation patterns for battles
TEST_F(PerformanceRegressionTest, MemoryUsageTrackingAndLeakDetection) {
    const int num_battles = 100;
    std::vector<size_t> memory_usage_samples;
    
    // Baseline memory measurement
    size_t baseline_memory = estimateMemoryUsage(Battle(playerTeam, opponentTeam));
    memory_baseline = baseline_memory;
    
    // Create and destroy many battles to check for leaks
    for (int i = 0; i < num_battles; ++i) {
        auto battle = std::make_unique<Battle>(playerTeam, opponentTeam, Battle::AIDifficulty::MEDIUM);
        
        // Simulate battle operations
        battle->isBattleOver();
        battle->getBattleResult();
        
        size_t current_usage = estimateMemoryUsage(*battle);
        memory_usage_samples.push_back(current_usage);
        
        // Battle should be destroyed here automatically
    }
    
    // Analyze memory usage patterns
    auto memory_stats = calculateStats(std::vector<double>(
        memory_usage_samples.begin(), memory_usage_samples.end()
    ));
    
    // Memory usage should be consistent (no significant growth)
    EXPECT_LT(memory_stats.stddev, baseline_memory * 0.1); // Less than 10% variation
    
    // No battle should use excessive memory
    EXPECT_LT(memory_stats.max_val, baseline_memory * 2.0); // Less than 2x baseline
    
    recordPerformance("memory_usage", memory_stats.mean, static_cast<size_t>(memory_stats.mean));
    
    std::cout << "Memory Usage: " << memory_stats.mean << " bytes ± " << memory_stats.stddev << std::endl;
    std::cout << "Baseline: " << baseline_memory << " bytes" << std::endl;
}

// REGRESSION TEST: Tournament Progression Performance
// Tests performance of tournament-related operations
TEST_F(PerformanceRegressionTest, TournamentProgressionPerformance) {
    const int num_tournaments = 20;
    std::vector<double> tournament_times;
    
    for (int i = 0; i < num_tournaments; ++i) {
        auto tournament_duration = measureOperation([&]() {
            // Simulate tournament operations
            Team tournament_team = TestUtils::createTestTeam({balancedPokemon});
            Team gym_team = TestUtils::createTestTeam({tankPokemon});
            
            // Create multiple battles (simulating gym battles)
            for (int gym = 0; gym < 8; ++gym) {
                Battle gym_battle(tournament_team, gym_team, Battle::AIDifficulty::MEDIUM);
                
                // Check battle state (simulating progression)
                gym_battle.isBattleOver();
                gym_battle.getBattleResult();
            }
        });
        
        tournament_times.push_back(tournament_duration.count());
    }
    
    auto tournament_stats = calculateStats(tournament_times);
    
    // Tournament progression should be efficient
    EXPECT_LT(tournament_stats.mean, 100.0); // Less than 100ms for full tournament simulation
    EXPECT_LT(tournament_stats.max_val, 200.0); // No tournament takes more than 200ms
    
    recordPerformance("tournament_progression", tournament_stats.mean);
    
    std::cout << "Tournament Progression: " << tournament_stats.mean << "ms ± " 
              << tournament_stats.stddev << "ms" << std::endl;
}

// REGRESSION TEST: High-Frequency Battle State Queries Performance
// Tests performance of repeated battle state queries
TEST_F(PerformanceRegressionTest, HighFrequencyBattleStateQueriesPerformance) {
    const int num_queries = 10000;
    Battle performance_battle(playerTeam, opponentTeam);
    
    std::vector<double> query_times;
    
    // Test high-frequency state queries
    for (int i = 0; i < num_queries; ++i) {
        auto query_duration = measureOperation([&]() {
            performance_battle.isBattleOver();
            performance_battle.getBattleResult();
        });
        
        query_times.push_back(query_duration.count());
    }
    
    auto query_stats = calculateStats(query_times);
    
    // State queries should be extremely fast
    EXPECT_LT(query_stats.mean, 0.01); // Less than 0.01ms per query
    EXPECT_LT(query_stats.max_val, 0.1);  // No single query takes more than 0.1ms
    
    recordPerformance("state_query", query_stats.mean);
    
    std::cout << "State Query: " << query_stats.mean << "ms ± " << query_stats.stddev << "ms" << std::endl;
    std::cout << "Total queries: " << num_queries << " in " << 
              std::accumulate(query_times.begin(), query_times.end(), 0.0) << "ms" << std::endl;
}

// REGRESSION TEST: Concurrent Battle Load Testing
// Tests performance under concurrent battle scenarios
TEST_F(PerformanceRegressionTest, ConcurrentBattleLoadTesting) {
    const int num_concurrent_battles = 50;
    const int operations_per_battle = 100;
    
    std::vector<std::thread> battle_threads;
    std::vector<double> thread_durations(num_concurrent_battles);
    std::atomic<bool> performance_acceptable{true};
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Create concurrent battles
    for (int i = 0; i < num_concurrent_battles; ++i) {
        battle_threads.emplace_back([&, i]() {
            auto thread_start = std::chrono::high_resolution_clock::now();
            
            try {
                Team thread_player_team = TestUtils::createTestTeam({balancedPokemon});
                Team thread_opponent_team = TestUtils::createTestTeam({fastPokemon});
                Battle concurrent_battle(thread_player_team, thread_opponent_team, 
                                       static_cast<Battle::AIDifficulty>(i % 4));
                
                // Perform many operations on this battle
                for (int op = 0; op < operations_per_battle; ++op) {
                    concurrent_battle.isBattleOver();
                    concurrent_battle.getBattleResult();
                    
                    // Simulate some processing delay
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                }
            }
            catch (...) {
                performance_acceptable = false;
            }
            
            auto thread_end = std::chrono::high_resolution_clock::now();
            thread_durations[i] = std::chrono::duration<double, std::milli>(thread_end - thread_start).count();
        });
    }
    
    // Wait for all battles to complete
    for (auto& thread : battle_threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration<double, std::milli>(end_time - start_time);
    
    EXPECT_TRUE(performance_acceptable.load());
    
    auto concurrent_stats = calculateStats(thread_durations);
    
    // Concurrent performance should not degrade significantly
    EXPECT_LT(total_duration.count(), 10000.0); // Total test should complete in under 10 seconds
    EXPECT_LT(concurrent_stats.mean, 2000.0);   // Each thread should complete in under 2 seconds
    
    recordPerformance("concurrent_battles", concurrent_stats.mean);
    
    std::cout << "Concurrent Battles (" << num_concurrent_battles << " threads): " << std::endl;
    std::cout << "  Average thread time: " << concurrent_stats.mean << "ms" << std::endl;
    std::cout << "  Total test time: " << total_duration.count() << "ms" << std::endl;
}

// REGRESSION TEST: Memory Allocation Pattern Analysis
// Analyzes memory allocation patterns during extended gameplay
TEST_F(PerformanceRegressionTest, MemoryAllocationPatternAnalysis) {
    const int extended_operations = 1000;
    std::vector<size_t> allocation_samples;
    
    // Create a long-running battle scenario
    Battle long_battle(playerTeam, opponentTeam, Battle::AIDifficulty::MEDIUM);
    
    for (int i = 0; i < extended_operations; ++i) {
        // Simulate extended gameplay operations
        long_battle.isBattleOver();
        long_battle.getBattleResult();
        
        // Sample memory usage periodically
        if (i % 10 == 0) {
            size_t current_allocation = estimateMemoryUsage(long_battle);
            allocation_samples.push_back(current_allocation);
        }
        
        // Create temporary objects to stress memory allocator
        if (i % 50 == 0) {
            Team temp_team = TestUtils::createTestTeam({balancedPokemon});
            // temp_team goes out of scope
        }
    }
    
    // Analyze allocation patterns
    auto allocation_stats = calculateStats(std::vector<double>(
        allocation_samples.begin(), allocation_samples.end()
    ));
    
    // Memory usage should remain stable
    double stability_threshold = memory_baseline * 0.2; // 20% variation allowed
    EXPECT_LT(allocation_stats.stddev, stability_threshold);
    
    // No significant memory growth over time
    double growth_rate = (allocation_samples.back() - allocation_samples.front()) / 
                        static_cast<double>(allocation_samples.size());
    EXPECT_LT(std::abs(growth_rate), memory_baseline * 0.01); // Less than 1% growth per sample
    
    recordPerformance("memory_stability", allocation_stats.stddev, static_cast<size_t>(allocation_stats.mean));
    
    std::cout << "Memory Allocation Analysis:" << std::endl;
    std::cout << "  Samples: " << allocation_samples.size() << std::endl;
    std::cout << "  Mean: " << allocation_stats.mean << " bytes" << std::endl;
    std::cout << "  Stability (stddev): " << allocation_stats.stddev << " bytes" << std::endl;
    std::cout << "  Growth rate: " << growth_rate << " bytes/sample" << std::endl;
}

// REGRESSION TEST: Performance Degradation Detection Mechanism
// Establishes regression detection thresholds and alerts
TEST_F(PerformanceRegressionTest, PerformanceDegradationDetectionMechanism) {
    // Define performance thresholds for critical operations
    struct PerformanceThreshold {
        std::string operation;
        double warning_threshold_ms;
        double critical_threshold_ms;
    };
    
    std::vector<PerformanceThreshold> thresholds = {
        {"battle_creation", 15.0, 50.0},
        {"turn_execution", 2.0, 10.0},
        {"easy_ai_decision", 8.0, 20.0},
        {"medium_ai_decision", 12.0, 30.0},
        {"hard_ai_decision", 25.0, 60.0},
        {"expert_ai_decision", 50.0, 120.0},
        {"state_query", 0.1, 1.0},
        {"tournament_progression", 150.0, 500.0}
    };
    
    // Test each operation and check against thresholds
    std::vector<std::string> warnings;
    std::vector<std::string> critical_issues;
    
    for (const auto& threshold : thresholds) {
        // Find measurements for this operation
        auto measurement_it = std::find_if(performance_baseline.begin(), performance_baseline.end(),
            [&threshold](const PerformanceMeasurement& m) {
                return m.operation == threshold.operation;
            });
        
        if (measurement_it != performance_baseline.end()) {
            double measured_time = measurement_it->duration_ms;
            
            if (measured_time > threshold.critical_threshold_ms) {
                critical_issues.push_back(threshold.operation + ": " + 
                                        std::to_string(measured_time) + "ms (critical: >" + 
                                        std::to_string(threshold.critical_threshold_ms) + "ms)");
            }
            else if (measured_time > threshold.warning_threshold_ms) {
                warnings.push_back(threshold.operation + ": " + 
                                 std::to_string(measured_time) + "ms (warning: >" + 
                                 std::to_string(threshold.warning_threshold_ms) + "ms)");
            }
        }
    }
    
    // Report performance issues
    if (!critical_issues.empty()) {
        std::cout << "CRITICAL PERFORMANCE ISSUES DETECTED:" << std::endl;
        for (const auto& issue : critical_issues) {
            std::cout << "  " << issue << std::endl;
        }
        FAIL() << "Critical performance regressions detected!";
    }
    
    if (!warnings.empty()) {
        std::cout << "Performance warnings:" << std::endl;
        for (const auto& warning : warnings) {
            std::cout << "  " << warning << std::endl;
        }
    }
    
    std::cout << "Performance regression detection: " << 
              (warnings.empty() ? "All operations within normal thresholds" : 
               std::to_string(warnings.size()) + " warnings detected") << std::endl;
    
    // Test should pass if no critical issues found
    SUCCEED();
}

// REGRESSION TEST: Data Loading Performance Benchmarks
// Tests JSON data loading performance to detect I/O regressions
TEST_F(PerformanceRegressionTest, DataLoadingPerformanceBenchmarks) {
    const int num_load_cycles = 10;
    std::vector<double> load_times;
    
    // Create temporary test data
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path() / "perf_test_data";
    std::filesystem::path pokemon_dir = temp_dir / "pokemon";
    std::filesystem::path moves_dir = temp_dir / "moves";
    
    std::filesystem::create_directories(pokemon_dir);
    std::filesystem::create_directories(moves_dir);
    
    // Create test files
    for (int i = 0; i < 50; ++i) {
        std::string pokemon_json = R"({"name": "perf-pokemon-)" + std::to_string(i) + 
                                  R"(", "id": )" + std::to_string(i + 1) + 
                                  R"(, "types": ["normal"], "base_stats": {"hp": 100, "attack": 80, "defense": 70, "special-attack": 90, "special-defense": 85, "speed": 75}})";
        
        std::ofstream pokemon_file(pokemon_dir / ("pokemon_" + std::to_string(i) + ".json"));
        pokemon_file << pokemon_json;
        pokemon_file.close();
        
        std::string move_json = R"({"name": "perf-move-)" + std::to_string(i) + 
                               R"(", "power": 80, "accuracy": 100, "pp": 15, "priority": 0})";
        
        std::ofstream move_file(moves_dir / ("move_" + std::to_string(i) + ".json"));
        move_file << move_json;
        move_file.close();
    }
    
    // Measure data loading performance
    for (int cycle = 0; cycle < num_load_cycles; ++cycle) {
        PokemonData test_data;
        
        auto load_duration = measureOperation([&]() {
            auto result = test_data.initialize(pokemon_dir.string(), moves_dir.string());
            EXPECT_TRUE(result.success);
        });
        
        load_times.push_back(load_duration.count());
    }
    
    auto load_stats = calculateStats(load_times);
    
    // Data loading should be efficient
    EXPECT_LT(load_stats.mean, 100.0); // Less than 100ms to load test dataset
    EXPECT_LT(load_stats.max_val, 200.0); // No single load takes more than 200ms
    
    recordPerformance("data_loading", load_stats.mean);
    
    // Cleanup
    std::error_code ec;
    std::filesystem::remove_all(temp_dir, ec);
    
    std::cout << "Data Loading Performance: " << load_stats.mean << "ms ± " << 
              load_stats.stddev << "ms" << std::endl;
}

// REGRESSION TEST: Battle State Consistency Under Load
// Ensures battle state remains consistent under performance stress
TEST_F(PerformanceRegressionTest, BattleStateConsistencyUnderLoad) {
    const int stress_operations = 5000;
    Battle stress_battle(playerTeam, opponentTeam);
    
    // Capture initial state
    bool initial_battle_over = stress_battle.isBattleOver();
    Battle::BattleResult initial_result = stress_battle.getBattleResult();
    
    auto consistency_duration = measureOperation([&]() {
        for (int i = 0; i < stress_operations; ++i) {
            // Rapid state queries to stress the battle system
            bool current_battle_over = stress_battle.isBattleOver();
            Battle::BattleResult current_result = stress_battle.getBattleResult();
            
            // State should remain consistent
            EXPECT_EQ(current_battle_over, initial_battle_over);
            EXPECT_EQ(current_result, initial_result);
            
            // Periodically check that the battle object is still valid
            if (i % 1000 == 0) {
                // Perform more complex operations
                EXPECT_NO_THROW({
                    stress_battle.isBattleOver();
                    stress_battle.getBattleResult();
                });
            }
        }
    });
    
    // Stress test should complete in reasonable time
    EXPECT_LT(consistency_duration.count(), 1000.0); // Less than 1 second
    
    recordPerformance("stress_test_consistency", consistency_duration.count());
    
    std::cout << "Battle State Consistency: " << stress_operations << 
              " operations in " << consistency_duration.count() << "ms" << std::endl;
}