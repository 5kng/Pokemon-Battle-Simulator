#include <gtest/gtest.h>
#include "../utils/test_utils.h"
#include "pokemon_data.h"
#include <filesystem>
#include <fstream>

class PokemonDataTest : public TestUtils::PokemonTestFixture {
protected:
    void SetUp() override {
        TestUtils::PokemonTestFixture::SetUp();
        
        // Create temporary test data directories
        test_pokemon_dir = "test_data/pokemon";
        test_moves_dir = "test_data/moves";
        
        std::filesystem::create_directories(test_pokemon_dir);
        std::filesystem::create_directories(test_moves_dir);
        
        // Create test Pokemon JSON files
        createTestPokemonFile("pikachu.json", "pikachu", 35, 55, 40, 50, 50, 90, {"electric"});
        createTestPokemonFile("charizard.json", "charizard", 78, 84, 78, 109, 85, 100, {"fire", "flying"});
        createTestPokemonFile("blastoise.json", "blastoise", 79, 83, 100, 85, 105, 78, {"water"});
        
        // Create test Move JSON files
        createTestMoveFile("thunderbolt.json", "thunderbolt", 90, 100, 15, "electric", "special", "damage", 0, "none", 0);
        createTestMoveFile("flamethrower.json", "flamethrower", 90, 100, 15, "fire", "special", "damage", 0, "none", 0);
        createTestMoveFile("hydro-pump.json", "hydro-pump", 110, 80, 5, "water", "special", "damage", 0, "none", 0);
        createTestMoveFile("thunder-wave.json", "thunder-wave", 0, 90, 20, "electric", "status", "ailment", 0, "paralysis", 100);
        
        pokemon_data = std::make_unique<PokemonData>();
    }
    
    void TearDown() override {
        // Clean up test directories
        std::filesystem::remove_all("test_data");
        TestUtils::PokemonTestFixture::TearDown();
    }
    
    void createTestPokemonFile(const std::string& filename, const std::string& name, 
                              int hp, int attack, int defense, int special_attack, 
                              int special_defense, int speed, const std::vector<std::string>& types) {
        nlohmann::json json_data = {
            {"name", name},
            {"id", 1},
            {"types", types},
            {"stats", {
                {"hp", hp},
                {"attack", attack},
                {"defense", defense},
                {"special-attack", special_attack},
                {"special-defense", special_defense},
                {"speed", speed}
            }}
        };
        
        std::ofstream file(test_pokemon_dir + "/" + filename);
        file << json_data.dump(4);
    }
    
    void createTestMoveFile(const std::string& filename, const std::string& name,
                           int power, int accuracy, int pp, const std::string& type,
                           const std::string& damage_class, const std::string& category,
                           int priority, const std::string& ailment, int ailment_chance) {
        nlohmann::json json_data = {
            {"name", name},
            {"power", power},
            {"accuracy", accuracy},
            {"pp", pp},
            {"type", {{"name", type}}},
            {"damage_class", {{"name", damage_class}}},
            {"meta", {
                {"category", {{"name", category}}},
                {"priority", priority},
                {"ailment", {{"name", ailment}}},
                {"ailment_chance", ailment_chance}
            }}
        };
        
        std::ofstream file(test_moves_dir + "/" + filename);
        file << json_data.dump(4);
    }
    
    std::string test_pokemon_dir;
    std::string test_moves_dir;
    std::unique_ptr<PokemonData> pokemon_data;
};

// Test PokemonData initialization
TEST_F(PokemonDataTest, Initialization) {
    auto result = pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.loaded_count, 7); // 3 Pokemon + 4 moves
    EXPECT_EQ(result.failed_count, 0);
    EXPECT_TRUE(result.error_message.empty());
}

// Test initialization with invalid directories
TEST_F(PokemonDataTest, InitializationWithInvalidDirectories) {
    auto result = pokemon_data->initialize("nonexistent/pokemon", "nonexistent/moves");
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error_message.empty());
}

// Test Pokemon data access
TEST_F(PokemonDataTest, PokemonDataAccess) {
    pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    auto available_pokemon = pokemon_data->getAvailablePokemon();
    EXPECT_EQ(available_pokemon.size(), 3);
    
    // Check if all test Pokemon are available
    auto pikachu_info = pokemon_data->getPokemonInfo("pikachu");
    ASSERT_TRUE(pikachu_info.has_value());
    EXPECT_EQ(pikachu_info->name, "pikachu");
    EXPECT_EQ(pikachu_info->hp, 35);
    EXPECT_EQ(pikachu_info->attack, 55);
    EXPECT_EQ(pikachu_info->defense, 40);
    EXPECT_EQ(pikachu_info->special_attack, 50);
    EXPECT_EQ(pikachu_info->special_defense, 50);
    EXPECT_EQ(pikachu_info->speed, 90);
    EXPECT_EQ(pikachu_info->types.size(), 1);
    EXPECT_EQ(pikachu_info->types[0], "electric");
}

// Test case-insensitive Pokemon lookup
TEST_F(PokemonDataTest, CaseInsensitivePokemonLookup) {
    pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    EXPECT_TRUE(pokemon_data->hasPokemon("pikachu"));
    EXPECT_TRUE(pokemon_data->hasPokemon("PIKACHU"));
    EXPECT_TRUE(pokemon_data->hasPokemon("PiKaChU"));
    
    auto pikachu_lower = pokemon_data->getPokemonInfo("pikachu");
    auto pikachu_upper = pokemon_data->getPokemonInfo("PIKACHU");
    auto pikachu_mixed = pokemon_data->getPokemonInfo("PiKaChU");
    
    ASSERT_TRUE(pikachu_lower.has_value());
    ASSERT_TRUE(pikachu_upper.has_value());
    ASSERT_TRUE(pikachu_mixed.has_value());
    
    EXPECT_EQ(pikachu_lower->name, pikachu_upper->name);
    EXPECT_EQ(pikachu_lower->hp, pikachu_mixed->hp);
}

// Test Pokemon by type filtering
TEST_F(PokemonDataTest, PokemonByTypeFiltering) {
    pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    auto electric_pokemon = pokemon_data->getPokemonByType("electric");
    EXPECT_EQ(electric_pokemon.size(), 1);
    EXPECT_EQ(electric_pokemon[0], "pikachu");
    
    auto fire_pokemon = pokemon_data->getPokemonByType("fire");
    EXPECT_EQ(fire_pokemon.size(), 1);
    EXPECT_EQ(fire_pokemon[0], "charizard");
    
    auto water_pokemon = pokemon_data->getPokemonByType("water");
    EXPECT_EQ(water_pokemon.size(), 1);
    EXPECT_EQ(water_pokemon[0], "blastoise");
    
    auto nonexistent_type = pokemon_data->getPokemonByType("grass");
    EXPECT_TRUE(nonexistent_type.empty());
}

// Test dual-type Pokemon
TEST_F(PokemonDataTest, DualTypePokemon) {
    pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    auto charizard_info = pokemon_data->getPokemonInfo("charizard");
    ASSERT_TRUE(charizard_info.has_value());
    EXPECT_EQ(charizard_info->types.size(), 2);
    EXPECT_EQ(charizard_info->types[0], "fire");
    EXPECT_EQ(charizard_info->types[1], "flying");
    
    // Should appear in both type filters
    auto fire_pokemon = pokemon_data->getPokemonByType("fire");
    auto flying_pokemon = pokemon_data->getPokemonByType("flying");
    
    EXPECT_NE(std::find(fire_pokemon.begin(), fire_pokemon.end(), "charizard"), fire_pokemon.end());
    EXPECT_NE(std::find(flying_pokemon.begin(), flying_pokemon.end(), "charizard"), flying_pokemon.end());
}

// Test Move data access
TEST_F(PokemonDataTest, MoveDataAccess) {
    pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    auto available_moves = pokemon_data->getAvailableMoves();
    EXPECT_EQ(available_moves.size(), 4);
    
    auto thunderbolt_info = pokemon_data->getMoveInfo("thunderbolt");
    ASSERT_TRUE(thunderbolt_info.has_value());
    EXPECT_EQ(thunderbolt_info->name, "thunderbolt");
    EXPECT_EQ(thunderbolt_info->power, 90);
    EXPECT_EQ(thunderbolt_info->accuracy, 100);
    EXPECT_EQ(thunderbolt_info->pp, 15);
    EXPECT_EQ(thunderbolt_info->type, "electric");
    EXPECT_EQ(thunderbolt_info->damage_class, "special");
}

// Test case-insensitive Move lookup
TEST_F(PokemonDataTest, CaseInsensitiveMoveLookup) {
    pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    EXPECT_TRUE(pokemon_data->hasMove("thunderbolt"));
    EXPECT_TRUE(pokemon_data->hasMove("THUNDERBOLT"));
    EXPECT_TRUE(pokemon_data->hasMove("ThUnDeRbOlT"));
    
    auto move_lower = pokemon_data->getMoveInfo("thunderbolt");
    auto move_upper = pokemon_data->getMoveInfo("THUNDERBOLT");
    auto move_mixed = pokemon_data->getMoveInfo("ThUnDeRbOlT");
    
    ASSERT_TRUE(move_lower.has_value());
    ASSERT_TRUE(move_upper.has_value());
    ASSERT_TRUE(move_mixed.has_value());
    
    EXPECT_EQ(move_lower->name, move_upper->name);
    EXPECT_EQ(move_lower->power, move_mixed->power);
}

// Test moves by type filtering
TEST_F(PokemonDataTest, MovesByTypeFiltering) {
    pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    auto electric_moves = pokemon_data->getMovesByType("electric");
    EXPECT_EQ(electric_moves.size(), 2);
    
    auto fire_moves = pokemon_data->getMovesByType("fire");
    EXPECT_EQ(fire_moves.size(), 1);
    EXPECT_EQ(fire_moves[0], "flamethrower");
    
    auto water_moves = pokemon_data->getMovesByType("water");
    EXPECT_EQ(water_moves.size(), 1);
    EXPECT_EQ(water_moves[0], "hydro-pump");
}

// Test moves by damage class filtering
TEST_F(PokemonDataTest, MovesByDamageClassFiltering) {
    pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    auto special_moves = pokemon_data->getMovesByDamageClass("special");
    EXPECT_EQ(special_moves.size(), 3);
    
    auto status_moves = pokemon_data->getMovesByDamageClass("status");
    EXPECT_EQ(status_moves.size(), 1);
    EXPECT_EQ(status_moves[0], "thunder-wave");
    
    auto physical_moves = pokemon_data->getMovesByDamageClass("physical");
    EXPECT_TRUE(physical_moves.empty());
}

// Test team entry validation
TEST_F(PokemonDataTest, TeamEntryValidation) {
    pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    // Valid team entry
    std::vector<std::string> valid_moves = {"thunderbolt", "thunder-wave"};
    EXPECT_TRUE(pokemon_data->validateTeamEntry("pikachu", valid_moves));
    
    // Invalid Pokemon name
    EXPECT_FALSE(pokemon_data->validateTeamEntry("nonexistent", valid_moves));
    
    // Invalid move name
    std::vector<std::string> invalid_moves = {"thunderbolt", "nonexistent-move"};
    EXPECT_FALSE(pokemon_data->validateTeamEntry("pikachu", invalid_moves));
    
    // Empty move list (should be valid)
    std::vector<std::string> empty_moves;
    EXPECT_TRUE(pokemon_data->validateTeamEntry("pikachu", empty_moves));
}

// Test move suggestions for Pokemon
TEST_F(PokemonDataTest, MoveSuggestionsForPokemon) {
    pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    auto suggested_moves = pokemon_data->suggestMovesForPokemon("pikachu", 2);
    EXPECT_LE(suggested_moves.size(), 2);
    
    // Should suggest moves that exist in our test data
    for (const auto& move_name : suggested_moves) {
        EXPECT_TRUE(pokemon_data->hasMove(move_name));
    }
}

// Test type effectiveness calculation
TEST_F(PokemonDataTest, TypeEffectivenessCalculation) {
    pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    // Test some basic type effectiveness
    double effectiveness = pokemon_data->getTypeEffectiveness("electric", {"water"});
    EXPECT_GT(effectiveness, 1.0); // Electric is super effective against water
    
    effectiveness = pokemon_data->getTypeEffectiveness("electric", {"ground"});
    EXPECT_EQ(effectiveness, 0.0); // Electric has no effect on ground
    
    effectiveness = pokemon_data->getTypeEffectiveness("normal", {"normal"});
    EXPECT_EQ(effectiveness, 1.0); // Normal effectiveness
}

// Test data statistics
TEST_F(PokemonDataTest, DataStatistics) {
    pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    std::string stats = pokemon_data->getDataStatistics();
    EXPECT_FALSE(stats.empty());
    EXPECT_NE(stats.find("Pokemon"), std::string::npos);
    EXPECT_NE(stats.find("Moves"), std::string::npos);
}

// Test cache clearing
TEST_F(PokemonDataTest, CacheClearing) {
    pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    // Verify data is loaded
    EXPECT_TRUE(pokemon_data->hasPokemon("pikachu"));
    EXPECT_TRUE(pokemon_data->hasMove("thunderbolt"));
    
    // Clear cache
    pokemon_data->clearCache();
    
    // Data should still be accessible (cache clear shouldn't remove data completely)
    EXPECT_TRUE(pokemon_data->hasPokemon("pikachu"));
    EXPECT_TRUE(pokemon_data->hasMove("thunderbolt"));
}

// Test data reloading
TEST_F(PokemonDataTest, DataReloading) {
    pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    auto initial_result = pokemon_data->getAvailablePokemon();
    EXPECT_EQ(initial_result.size(), 3);
    
    // Add another test Pokemon file
    createTestPokemonFile("venusaur.json", "venusaur", 80, 82, 83, 100, 100, 80, {"grass", "poison"});
    
    auto reload_result = pokemon_data->reloadData();
    EXPECT_TRUE(reload_result.success);
    EXPECT_EQ(reload_result.loaded_count, 8); // 4 Pokemon + 4 moves
    
    auto reloaded_pokemon = pokemon_data->getAvailablePokemon();
    EXPECT_EQ(reloaded_pokemon.size(), 4);
    EXPECT_TRUE(pokemon_data->hasPokemon("venusaur"));
}

// Test error handling with malformed JSON
TEST_F(PokemonDataTest, ErrorHandlingMalformedJSON) {
    // Create a malformed JSON file
    std::ofstream bad_file(test_pokemon_dir + "/bad_pokemon.json");
    bad_file << "{ invalid json }";
    bad_file.close();
    
    auto result = pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    
    // Should still succeed but with some failures reported
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.failed_count, 0);
}

// Test performance with larger datasets
TEST_F(PokemonDataTest, PerformanceWithLargeDataset) {
    // Create a larger number of test Pokemon
    for (int i = 0; i < 100; ++i) {
        std::string name = "pokemon" + std::to_string(i);
        createTestPokemonFile(name + ".json", name, 100, 80, 70, 90, 85, 75, {"normal"});
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    auto result = pokemon_data->initialize(test_pokemon_dir, test_moves_dir);
    auto end = std::chrono::high_resolution_clock::now();
    
    EXPECT_TRUE(result.success);
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 5000); // Should complete within 5 seconds
    
    // Test lookup performance
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        std::string name = "pokemon" + std::to_string(i);
        EXPECT_TRUE(pokemon_data->hasPokemon(name));
    }
    end = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 100); // Lookups should be very fast
}