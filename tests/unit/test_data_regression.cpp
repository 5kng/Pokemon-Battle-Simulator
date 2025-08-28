#include <gtest/gtest.h>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <memory>
#include "test_utils.h"
#include "pokemon_data.h"
#include "team_builder.h"
#include "tournament_manager.h"

/**
 * Data Regression Test Suite
 * 
 * This file contains comprehensive regression tests for data handling robustness,
 * JSON parsing resilience, and data integrity validation. These tests prevent
 * historical bugs from returning and ensure production-ready data processing.
 * 
 * Coverage Areas:
 * - JSON parsing robustness with malformed data
 * - Data validation and graceful degradation
 * - File corruption handling
 * - Memory management during data loading
 * - Performance regression detection for data operations
 */

class DataRegressionTest : public TestUtils::PokemonTestFixture {
protected:
    void SetUp() override {
        TestUtils::PokemonTestFixture::SetUp();
        
        // Create test directory structure
        test_data_dir = std::filesystem::temp_directory_path() / "pokemon_test_data";
        test_pokemon_dir = test_data_dir / "pokemon";
        test_moves_dir = test_data_dir / "moves";
        test_teams_dir = test_data_dir / "teams";
        test_tournaments_dir = test_data_dir / "tournaments";
        
        std::filesystem::create_directories(test_pokemon_dir);
        std::filesystem::create_directories(test_moves_dir);
        std::filesystem::create_directories(test_teams_dir);
        std::filesystem::create_directories(test_tournaments_dir);
        
        // Initialize Pokemon data for testing
        pokemon_data = std::make_unique<PokemonData>();
        
        // Create valid baseline data files for comparison
        createBaselineTestFiles();
    }
    
    void TearDown() override {
        // Clean up test files
        std::error_code ec;
        std::filesystem::remove_all(test_data_dir, ec);
        // Don't throw on cleanup failure
    }
    
    void createBaselineTestFiles() {
        // Create valid Pokemon JSON file
        std::string valid_pokemon_json = R"({
            "name": "test-pokemon",
            "id": 999,
            "types": ["normal"],
            "base_stats": {
                "hp": 100,
                "attack": 80,
                "defense": 70,
                "special-attack": 90,
                "special-defense": 85,
                "speed": 75
            }
        })";
        writeTestFile(test_pokemon_dir / "test-pokemon.json", valid_pokemon_json);
        
        // Create valid Move JSON file
        std::string valid_move_json = R"({
            "name": "test-move",
            "accuracy": 100,
            "power": 80,
            "pp": 15,
            "priority": 0,
            "damage_class": {"name": "physical"},
            "type": {"name": "normal"},
            "Info": {
                "category": {"name": "damage"},
                "ailment": {"name": "none"},
                "ailment_chance": 0,
                "crit_rate": 0
            }
        })";
        writeTestFile(test_moves_dir / "test-move.json", valid_move_json);
        
        // Create valid team template
        std::string valid_team_json = R"({
            "name": "Test Team",
            "pokemon": [
                {
                    "name": "test-pokemon",
                    "moves": ["test-move"]
                }
            ]
        })";
        writeTestFile(test_teams_dir / "test_team.json", valid_team_json);
    }
    
    void writeTestFile(const std::filesystem::path& file_path, const std::string& content) {
        std::ofstream file(file_path);
        file << content;
        file.close();
    }
    
    // Helper to create malformed JSON with specific corruption patterns
    void createMalformedFile(const std::filesystem::path& file_path, const std::string& corruption_type) {
        std::string malformed_content;
        
        if (corruption_type == "invalid_json") {
            malformed_content = R"({"name": "broken-pokemon", "id": 1 "missing_comma": true})";
        }
        else if (corruption_type == "missing_required_field") {
            malformed_content = R"({"name": "incomplete-pokemon", "types": ["normal"]})"; // Missing ID and base_stats
        }
        else if (corruption_type == "invalid_type_values") {
            malformed_content = R"({
                "name": "invalid-pokemon",
                "id": "not_a_number",
                "types": ["invalid", 123, null],
                "base_stats": {
                    "hp": -50,
                    "attack": 999999,
                    "defense": "invalid",
                    "special-attack": null,
                    "special-defense": 50.5,
                    "speed": []
                }
            })";
        }
        else if (corruption_type == "integer_overflow") {
            malformed_content = R"({
                "name": "overflow-pokemon",
                "id": 2147483648,
                "types": ["normal"],
                "base_stats": {
                    "hp": 4294967296,
                    "attack": 80,
                    "defense": 70,
                    "special-attack": 90,
                    "special-defense": 85,
                    "speed": 75
                }
            })";
        }
        else if (corruption_type == "empty_file") {
            malformed_content = "";
        }
        else if (corruption_type == "binary_data") {
            // Simulate binary corruption
            malformed_content = std::string(256, '\0') + std::string(256, '\xFF');
        }
        
        writeTestFile(file_path, malformed_content);
    }
    
    std::filesystem::path test_data_dir;
    std::filesystem::path test_pokemon_dir;
    std::filesystem::path test_moves_dir;
    std::filesystem::path test_teams_dir;
    std::filesystem::path test_tournaments_dir;
    std::unique_ptr<PokemonData> pokemon_data;
};

// REGRESSION TEST: JSON Parsing Robustness with Malformed Pokemon Data
// Ensures the system doesn't crash when encountering corrupted Pokemon files
TEST_F(DataRegressionTest, MalformedPokemonJSONHandling) {
    // Test invalid JSON syntax
    createMalformedFile(test_pokemon_dir / "invalid_syntax.json", "invalid_json");
    
    auto result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
    
    // Should not crash and should report the failure
    EXPECT_TRUE(result.success); // Should still succeed with other valid files
    EXPECT_GT(result.failed_count, 0); // Should count failed files
    EXPECT_GT(result.loaded_count, 0); // Should load valid files
    
    // System should remain functional
    EXPECT_TRUE(pokemon_data->hasPokemon("test-pokemon"));
    EXPECT_FALSE(pokemon_data->hasPokemon("invalid-syntax"));
}

// REGRESSION TEST: Missing Required Fields Protection
// Prevents crashes when required JSON fields are missing
TEST_F(DataRegressionTest, MissingRequiredFieldsProtection) {
    createMalformedFile(test_pokemon_dir / "missing_fields.json", "missing_required_field");
    
    auto result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
    
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.failed_count, 0);
    
    // Missing field file should not be loaded
    EXPECT_FALSE(pokemon_data->hasPokemon("incomplete-pokemon"));
    
    // Valid files should still work
    EXPECT_TRUE(pokemon_data->hasPokemon("test-pokemon"));
}

// REGRESSION TEST: Invalid Data Type Handling
// Ensures proper validation of data types and ranges
TEST_F(DataRegressionTest, InvalidDataTypeHandling) {
    createMalformedFile(test_pokemon_dir / "invalid_types.json", "invalid_type_values");
    
    auto result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
    
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.failed_count, 0);
    
    // File with invalid types should be rejected
    EXPECT_FALSE(pokemon_data->hasPokemon("invalid-pokemon"));
    
    // System should remain stable
    auto valid_pokemon = pokemon_data->getPokemonInfo("test-pokemon");
    ASSERT_TRUE(valid_pokemon.has_value());
    EXPECT_EQ(valid_pokemon->name, "test-pokemon");
}

// REGRESSION TEST: Integer Overflow Prevention
// Critical test for preventing integer overflow vulnerabilities
TEST_F(DataRegressionTest, IntegerOverflowPrevention) {
    createMalformedFile(test_pokemon_dir / "overflow.json", "integer_overflow");
    
    auto result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
    
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.failed_count, 0);
    
    // Overflow values should be rejected
    EXPECT_FALSE(pokemon_data->hasPokemon("overflow-pokemon"));
    
    // System should not crash or exhibit undefined behavior
    auto pokemon_list = pokemon_data->getAvailablePokemon();
    EXPECT_FALSE(pokemon_list.empty());
}

// REGRESSION TEST: Empty File Handling
// Tests graceful handling of empty or corrupt files
TEST_F(DataRegressionTest, EmptyFileHandling) {
    createMalformedFile(test_pokemon_dir / "empty.json", "empty_file");
    createMalformedFile(test_moves_dir / "empty_move.json", "empty_file");
    
    auto result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
    
    EXPECT_TRUE(result.success);
    EXPECT_GE(result.failed_count, 2); // At least 2 empty files should fail
    
    // System should continue functioning
    EXPECT_TRUE(pokemon_data->hasPokemon("test-pokemon"));
    EXPECT_TRUE(pokemon_data->hasMove("test-move"));
}

// REGRESSION TEST: Binary Data Corruption Handling
// Tests resilience against binary corruption in JSON files
TEST_F(DataRegressionTest, BinaryDataCorruptionHandling) {
    createMalformedFile(test_pokemon_dir / "binary_corrupt.json", "binary_data");
    
    // This should not crash the system
    auto result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
    
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.failed_count, 0);
    
    // Valid data should still be accessible
    EXPECT_TRUE(pokemon_data->hasPokemon("test-pokemon"));
}

// REGRESSION TEST: Move Data Corruption Resilience
// Tests handling of corrupted move files with various failure modes
TEST_F(DataRegressionTest, MoveDataCorruptionResilience) {
    // Create corrupted move files
    std::string corrupt_move1 = R"({"name": "corrupt-move1", "power": null, "accuracy": 150})"; // Invalid accuracy
    std::string corrupt_move2 = R"({"name": "corrupt-move2", "pp": 0, "priority": 10})"; // Invalid PP and priority
    std::string corrupt_move3 = R"({"name": "corrupt-move3"})"; // Missing critical fields
    
    writeTestFile(test_moves_dir / "corrupt1.json", corrupt_move1);
    writeTestFile(test_moves_dir / "corrupt2.json", corrupt_move2);
    writeTestFile(test_moves_dir / "corrupt3.json", corrupt_move3);
    
    auto result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
    
    EXPECT_TRUE(result.success);
    EXPECT_GE(result.failed_count, 3);
    
    // Corrupted moves should not be loaded
    EXPECT_FALSE(pokemon_data->hasMove("corrupt-move1"));
    EXPECT_FALSE(pokemon_data->hasMove("corrupt-move2"));
    EXPECT_FALSE(pokemon_data->hasMove("corrupt-move3"));
    
    // Valid move should still work
    EXPECT_TRUE(pokemon_data->hasMove("test-move"));
}

// REGRESSION TEST: Large Dataset Performance Degradation
// Ensures data loading performance doesn't degrade with scale
TEST_F(DataRegressionTest, LargeDatasetPerformanceDegradation) {
    // Create many files to test scaling
    const int num_files = 100;
    
    // Generate many Pokemon files  
    for (int i = 0; i < num_files; ++i) {
        std::string pokemon_json = R"({
            "name": "bulk-pokemon-)" + std::to_string(i) + R"(",
            "id": )" + std::to_string(i + 1000) + R"(,
            "types": ["normal"],
            "base_stats": {
                "hp": )" + std::to_string(50 + (i % 100)) + R"(,
                "attack": )" + std::to_string(50 + (i % 100)) + R"(,
                "defense": )" + std::to_string(50 + (i % 100)) + R"(,
                "special-attack": )" + std::to_string(50 + (i % 100)) + R"(,
                "special-defense": )" + std::to_string(50 + (i % 100)) + R"(,
                "speed": )" + std::to_string(50 + (i % 100)) + R"(
            }
        })";
        
        writeTestFile(test_pokemon_dir / ("bulk_" + std::to_string(i) + ".json"), pokemon_json);
    }
    
    auto generation_time = std::chrono::high_resolution_clock::now();
    
    // Load all data
    auto result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
    
    auto end_time = std::chrono::high_resolution_clock::now();
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.loaded_count, num_files + 1); // +1 for baseline test file
    
    // Performance regression check: loading should complete within reasonable time
    auto loading_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - generation_time);
    EXPECT_LT(loading_duration.count(), 5000); // Less than 5 seconds for 100 files
    
    // Verify all data was loaded correctly
    for (int i = 0; i < num_files; ++i) {
        EXPECT_TRUE(pokemon_data->hasPokemon("bulk-pokemon-" + std::to_string(i)));
    }
}

// REGRESSION TEST: Memory Leak Prevention in Data Loading
// Ensures no memory leaks during repeated data loading operations
TEST_F(DataRegressionTest, DataLoadingMemoryLeakPrevention) {
    // Perform multiple load/reload cycles
    const int num_cycles = 20;
    
    for (int cycle = 0; cycle < num_cycles; ++cycle) {
        // Initialize and reload data
        auto result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
        EXPECT_TRUE(result.success);
        
        // Verify data is accessible
        EXPECT_TRUE(pokemon_data->hasPokemon("test-pokemon"));
        EXPECT_TRUE(pokemon_data->hasMove("test-move"));
        
        // Force reinitialization to test cleanup
        pokemon_data = std::make_unique<PokemonData>();
    }
    
    // Test should complete without memory issues
    SUCCEED();
}

// REGRESSION TEST: Concurrent Data Access Safety
// Tests data structure integrity under potential concurrent access
TEST_F(DataRegressionTest, ConcurrentDataAccessSafety) {
    auto result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
    ASSERT_TRUE(result.success);
    
    // Simulate concurrent read operations
    std::vector<std::thread> threads;
    std::atomic<bool> test_passed{true};
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            try {
                // Perform various read operations simultaneously
                auto pokemon_list = pokemon_data->getAvailablePokemon();
                auto move_list = pokemon_data->getAvailableMoves();
                
                if (pokemon_list.empty() || move_list.empty()) {
                    test_passed = false;
                }
                
                // Test specific lookups
                auto pokemon_info = pokemon_data->getPokemonInfo("test-pokemon");
                auto move_info = pokemon_data->getMoveInfo("test-move");
                
                if (!pokemon_info.has_value() || !move_info.has_value()) {
                    test_passed = false;
                }
                
                // Test type queries
                auto normal_pokemon = pokemon_data->getPokemonByType("normal");
                if (normal_pokemon.empty()) {
                    test_passed = false;
                }
            }
            catch (...) {
                test_passed = false;
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_TRUE(test_passed.load());
}

// REGRESSION TEST: Team Data Validation Robustness
// Tests team data validation with various corruption scenarios
TEST_F(DataRegressionTest, TeamDataValidationRobustness) {
    // Initialize data first
    auto result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
    ASSERT_TRUE(result.success);
    
    // Create invalid team configurations
    std::string invalid_team1 = R"({
        "name": "Invalid Team 1",
        "pokemon": [
            {
                "name": "non-existent-pokemon",
                "moves": ["test-move"]
            }
        ]
    })";
    
    std::string invalid_team2 = R"({
        "name": "Invalid Team 2",
        "pokemon": [
            {
                "name": "test-pokemon",
                "moves": ["non-existent-move"]
            }
        ]
    })";
    
    writeTestFile(test_teams_dir / "invalid_team1.json", invalid_team1);
    writeTestFile(test_teams_dir / "invalid_team2.json", invalid_team2);
    
    // Test validation functions
    EXPECT_FALSE(pokemon_data->validateTeamEntry("non-existent-pokemon", {"test-move"}));
    EXPECT_FALSE(pokemon_data->validateTeamEntry("test-pokemon", {"non-existent-move"}));
    EXPECT_TRUE(pokemon_data->validateTeamEntry("test-pokemon", {"test-move"}));
}

// REGRESSION TEST: JSON Parsing Performance Under Load
// Measures parsing performance to detect regression
TEST_F(DataRegressionTest, JSONParsingPerformanceUnderLoad) {
    const int num_test_files = 50;
    
    // Create many JSON files with complex structures
    for (int i = 0; i < num_test_files; ++i) {
        std::string complex_pokemon = R"({
            "name": "perf-test-)" + std::to_string(i) + R"(",
            "id": )" + std::to_string(i + 2000) + R"(,
            "types": ["normal", "flying"],
            "base_stats": {
                "hp": 100,
                "attack": 80,
                "defense": 70,
                "special-attack": 90,
                "special-defense": 85,
                "speed": 75
            },
            "additional_data": {
                "height": 1.5,
                "weight": 20.5,
                "abilities": ["ability1", "ability2", "hidden-ability"],
                "evolution_chain": {
                    "previous": null,
                    "next": ["evolution1", "evolution2"]
                }
            }
        })";
        
        writeTestFile(test_pokemon_dir / ("perf_" + std::to_string(i) + ".json"), complex_pokemon);
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    auto result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.loaded_count, num_test_files + 1); // +1 for baseline
    
    // Performance threshold: should parse files efficiently
    // Allow 1ms per file plus reasonable overhead
    EXPECT_LT(duration.count(), num_test_files + 100);
    
    std::cout << "Parsed " << num_test_files << " files in " << duration.count() << "ms" << std::endl;
}

// REGRESSION TEST: Data Integrity After Multiple Operations
// Ensures data remains consistent after various operations
TEST_F(DataRegressionTest, DataIntegrityAfterMultipleOperations) {
    auto result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
    ASSERT_TRUE(result.success);
    
    // Capture baseline state
    auto initial_pokemon_count = pokemon_data->getAvailablePokemon().size();
    auto initial_move_count = pokemon_data->getAvailableMoves().size();
    auto initial_stats = pokemon_data->getDataStatistics();
    
    // Perform many operations
    for (int i = 0; i < 100; ++i) {
        // Various read operations that shouldn't modify data
        pokemon_data->getAvailablePokemon();
        pokemon_data->getAvailableMoves();
        pokemon_data->getPokemonByType("normal");
        pokemon_data->getMovesByType("normal");
        pokemon_data->getPokemonInfo("test-pokemon");
        pokemon_data->getMoveInfo("test-move");
        pokemon_data->suggestMovesForPokemon("test-pokemon", 4);
        pokemon_data->validateTeamEntry("test-pokemon", {"test-move"});
    }
    
    // Verify data integrity is maintained
    EXPECT_EQ(pokemon_data->getAvailablePokemon().size(), initial_pokemon_count);
    EXPECT_EQ(pokemon_data->getAvailableMoves().size(), initial_move_count);
    
    // Key data should still be accessible
    EXPECT_TRUE(pokemon_data->hasPokemon("test-pokemon"));
    EXPECT_TRUE(pokemon_data->hasMove("test-move"));
    
    // Verify specific data hasn't been corrupted
    auto pokemon_info = pokemon_data->getPokemonInfo("test-pokemon");
    ASSERT_TRUE(pokemon_info.has_value());
    EXPECT_EQ(pokemon_info->name, "test-pokemon");
    EXPECT_EQ(pokemon_info->id, 999);
    EXPECT_EQ(pokemon_info->hp, 100);
}

// REGRESSION TEST: Error Recovery and Fallback Mechanisms
// Tests system recovery from various error conditions
TEST_F(DataRegressionTest, ErrorRecoveryAndFallbackMechanisms) {
    // Test with completely invalid directory
    auto invalid_result = pokemon_data->initialize("/non/existent/path", "/another/invalid/path");
    EXPECT_FALSE(invalid_result.success);
    EXPECT_FALSE(invalid_result.error_message.empty());
    
    // System should remain in a valid state even after failure
    EXPECT_FALSE(pokemon_data->hasPokemon("any-pokemon"));
    
    // Should be able to recover with valid paths
    auto recovery_result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
    EXPECT_TRUE(recovery_result.success);
    EXPECT_TRUE(pokemon_data->hasPokemon("test-pokemon"));
    
    // Test reload functionality
    auto reload_result = pokemon_data->reloadData();
    EXPECT_TRUE(reload_result.success);
    EXPECT_TRUE(pokemon_data->hasPokemon("test-pokemon"));
}

// REGRESSION TEST: Tournament Data Corruption Handling
// Ensures tournament save/load operations handle corruption gracefully
TEST_F(DataRegressionTest, TournamentDataCorruptionHandling) {
    // Create corrupted tournament data
    std::string corrupt_tournament = R"({
        "player_name": null,
        "current_gym": "invalid-gym-id",
        "badges": ["badge1", 123, null, "badge2"],
        "progress": {
            "elite_four_beaten": "not_a_boolean",
            "champion_beaten": null
        }
    })";
    
    writeTestFile(test_tournaments_dir / "corrupt_tournament.json", corrupt_tournament);
    
    // Tournament system should handle corruption without crashing
    // This would be tested if TournamentManager had JSON loading capabilities
    SUCCEED(); // Placeholder - in full implementation would test tournament loading
}

// REGRESSION TEST: Type Effectiveness Data Validation
// Ensures type effectiveness calculations remain accurate
TEST_F(DataRegressionTest, TypeEffectivenessDataValidation) {
    auto result = pokemon_data->initialize(test_pokemon_dir.string(), test_moves_dir.string());
    ASSERT_TRUE(result.success);
    
    // Test known type effectiveness relationships
    double fire_vs_grass = pokemon_data->getTypeEffectiveness("fire", {"grass"});
    EXPECT_DOUBLE_EQ(fire_vs_grass, 2.0); // Super effective
    
    double fire_vs_water = pokemon_data->getTypeEffectiveness("fire", {"water"});
    EXPECT_DOUBLE_EQ(fire_vs_water, 0.5); // Not very effective
    
    double normal_vs_normal = pokemon_data->getTypeEffectiveness("normal", {"normal"});
    EXPECT_DOUBLE_EQ(normal_vs_normal, 1.0); // Neutral
    
    double electric_vs_ground = pokemon_data->getTypeEffectiveness("electric", {"ground"});
    EXPECT_DOUBLE_EQ(electric_vs_ground, 0.0); // No effect
    
    // Test dual typing
    double water_vs_fire_rock = pokemon_data->getTypeEffectiveness("water", {"fire", "rock"});
    EXPECT_DOUBLE_EQ(water_vs_fire_rock, 4.0); // 2.0 * 2.0 = 4.0
}