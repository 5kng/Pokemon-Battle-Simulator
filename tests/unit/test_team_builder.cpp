#include <gtest/gtest.h>
#include "../utils/test_utils.h"
#include "team_builder.h"
#include "pokemon_data.h"
#include <memory>
#include <filesystem>

class TeamBuilderTest : public TestUtils::PokemonTestFixture {
protected:
    void SetUp() override {
        TestUtils::PokemonTestFixture::SetUp();
        
        // Create test data directory
        test_data_dir = "test_team_builder_data";
        std::filesystem::create_directories(test_data_dir + "/pokemon");
        std::filesystem::create_directories(test_data_dir + "/moves");
        
        // Create test Pokemon and move data files
        createTestDataFiles();
        
        // Initialize Pokemon data
        pokemonData = std::make_shared<PokemonData>();
        pokemonData->initialize(test_data_dir + "/pokemon", test_data_dir + "/moves");
        
        // Create team builder
        teamBuilder = std::make_unique<TeamBuilder>(pokemonData);
    }
    
    void TearDown() override {
        std::filesystem::remove_all(test_data_dir);
        TestUtils::PokemonTestFixture::TearDown();
    }
    
    void createTestDataFiles() {
        // Create Pokemon data files
        createPokemonFile("pikachu.json", "pikachu", 35, 55, 40, 50, 50, 90, {"electric"});
        createPokemonFile("charizard.json", "charizard", 78, 84, 78, 109, 85, 100, {"fire", "flying"});
        createPokemonFile("blastoise.json", "blastoise", 79, 83, 100, 85, 105, 78, {"water"});
        createPokemonFile("venusaur.json", "venusaur", 80, 82, 83, 100, 100, 80, {"grass", "poison"});
        createPokemonFile("alakazam.json", "alakazam", 55, 50, 45, 135, 95, 120, {"psychic"});
        createPokemonFile("machamp.json", "machamp", 90, 130, 80, 65, 85, 55, {"fighting"});
        
        // Create Move data files
        createMoveFile("thunderbolt.json", "thunderbolt", 90, 100, 15, "electric", "special");
        createMoveFile("flamethrower.json", "flamethrower", 90, 100, 15, "fire", "special");
        createMoveFile("hydro-pump.json", "hydro-pump", 110, 80, 5, "water", "special");
        createMoveFile("solar-beam.json", "solar-beam", 120, 100, 10, "grass", "special");
        createMoveFile("psychic.json", "psychic", 90, 100, 10, "psychic", "special");
        createMoveFile("close-combat.json", "close-combat", 120, 100, 5, "fighting", "physical");
        createMoveFile("tackle.json", "tackle", 40, 100, 35, "normal", "physical");
        createMoveFile("thunder-wave.json", "thunder-wave", 0, 90, 20, "electric", "status");
    }
    
    void createPokemonFile(const std::string& filename, const std::string& name, 
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
        
        std::ofstream file(test_data_dir + "/pokemon/" + filename);
        file << json_data.dump(4);
    }
    
    void createMoveFile(const std::string& filename, const std::string& name,
                       int power, int accuracy, int pp, const std::string& type,
                       const std::string& damage_class) {
        nlohmann::json json_data = {
            {"name", name},
            {"power", power},
            {"accuracy", accuracy},
            {"pp", pp},
            {"type", {{"name", type}}},
            {"damage_class", {{"name", damage_class}}},
            {"meta", {
                {"category", {{"name", "damage"}}},
                {"priority", 0},
                {"ailment", {{"name", "none"}}},
                {"ailment_chance", 0}
            }}
        };
        
        std::ofstream file(test_data_dir + "/moves/" + filename);
        file << json_data.dump(4);
    }
    
    std::string test_data_dir;
    std::shared_ptr<PokemonData> pokemonData;
    std::unique_ptr<TeamBuilder> teamBuilder;
};

// Test team creation
TEST_F(TeamBuilderTest, TeamCreation) {
    auto team = teamBuilder->createTeam("Test Team");
    
    EXPECT_EQ(team.name, "Test Team");
    EXPECT_TRUE(team.isEmpty());
    EXPECT_FALSE(team.isFull());
    EXPECT_EQ(team.size(), 0);
    EXPECT_FALSE(team.is_valid);
}

// Test adding Pokemon to team
TEST_F(TeamBuilderTest, AddPokemonToTeam) {
    auto team = teamBuilder->createTeam("Test Team");
    
    std::vector<std::string> moves = {"thunderbolt", "tackle", "thunder-wave"};
    bool added = teamBuilder->addPokemonToTeam(team, "pikachu", moves);
    
    EXPECT_TRUE(added);
    EXPECT_EQ(team.size(), 1);
    EXPECT_FALSE(team.isEmpty());
    EXPECT_EQ(team.pokemon[0].name, "pikachu");
    EXPECT_EQ(team.pokemon[0].moves, moves);
}

// Test adding invalid Pokemon
TEST_F(TeamBuilderTest, AddInvalidPokemonToTeam) {
    auto team = teamBuilder->createTeam("Test Team");
    
    std::vector<std::string> moves = {"thunderbolt"};
    bool added = teamBuilder->addPokemonToTeam(team, "nonexistent-pokemon", moves);
    
    EXPECT_FALSE(added);
    EXPECT_TRUE(team.isEmpty());
}

// Test removing Pokemon from team
TEST_F(TeamBuilderTest, RemovePokemonFromTeam) {
    auto team = teamBuilder->createTeam("Test Team");
    
    // Add two Pokemon
    teamBuilder->addPokemonToTeam(team, "pikachu", {"thunderbolt"});
    teamBuilder->addPokemonToTeam(team, "charizard", {"flamethrower"});
    EXPECT_EQ(team.size(), 2);
    
    // Remove first Pokemon
    bool removed = teamBuilder->removePokemonFromTeam(team, 0);
    EXPECT_TRUE(removed);
    EXPECT_EQ(team.size(), 1);
    EXPECT_EQ(team.pokemon[0].name, "charizard");
    
    // Try to remove invalid index
    bool invalidRemove = teamBuilder->removePokemonFromTeam(team, 5);
    EXPECT_FALSE(invalidRemove);
}

// Test modifying Pokemon moves
TEST_F(TeamBuilderTest, ModifyPokemonMoves) {
    auto team = teamBuilder->createTeam("Test Team");
    teamBuilder->addPokemonToTeam(team, "pikachu", {"tackle"});
    
    std::vector<std::string> newMoves = {"thunderbolt", "thunder-wave"};
    bool modified = teamBuilder->modifyPokemonMoves(team, 0, newMoves);
    
    EXPECT_TRUE(modified);
    EXPECT_EQ(team.pokemon[0].moves, newMoves);
    
    // Try to modify invalid Pokemon index
    bool invalidModify = teamBuilder->modifyPokemonMoves(team, 10, newMoves);
    EXPECT_FALSE(invalidModify);
}

// Test team validation with default settings
TEST_F(TeamBuilderTest, TeamValidationDefault) {
    auto team = teamBuilder->createTeam("Valid Team");
    
    // Add valid Pokemon
    teamBuilder->addPokemonToTeam(team, "pikachu", {"thunderbolt"});
    teamBuilder->addPokemonToTeam(team, "charizard", {"flamethrower"});
    teamBuilder->addPokemonToTeam(team, "blastoise", {"hydro-pump"});
    
    bool isValid = teamBuilder->validateTeam(team);
    EXPECT_TRUE(isValid);
    EXPECT_TRUE(team.is_valid);
}

// Test team validation with custom settings
TEST_F(TeamBuilderTest, TeamValidationCustomSettings) {
    TeamBuilder::ValidationSettings settings;
    settings.min_team_size = 3;
    settings.enforce_min_team_size = true;
    settings.require_type_diversity = true;
    settings.min_unique_types = 2;
    
    auto team = teamBuilder->createTeam("Custom Valid Team");
    teamBuilder->addPokemonToTeam(team, "pikachu", {"thunderbolt"});
    teamBuilder->addPokemonToTeam(team, "charizard", {"flamethrower"});
    teamBuilder->addPokemonToTeam(team, "blastoise", {"hydro-pump"});
    
    bool isValid = teamBuilder->validateTeam(team, settings);
    EXPECT_TRUE(isValid);
}

// Test team validation failures
TEST_F(TeamBuilderTest, TeamValidationFailures) {
    TeamBuilder::ValidationSettings settings;
    settings.allow_duplicate_pokemon = false;
    
    auto team = teamBuilder->createTeam("Invalid Team");
    teamBuilder->addPokemonToTeam(team, "pikachu", {"thunderbolt"});
    teamBuilder->addPokemonToTeam(team, "pikachu", {"tackle"}); // Duplicate
    
    bool isValid = teamBuilder->validateTeam(team, settings);
    EXPECT_FALSE(isValid);
    EXPECT_FALSE(team.is_valid);
    EXPECT_FALSE(team.validation_errors.empty());
}

// Test team analysis
TEST_F(TeamBuilderTest, TeamAnalysis) {
    auto team = teamBuilder->createTeam("Analysis Team");
    teamBuilder->addPokemonToTeam(team, "pikachu", {"thunderbolt", "tackle"});
    teamBuilder->addPokemonToTeam(team, "charizard", {"flamethrower", "solar-beam"});
    teamBuilder->addPokemonToTeam(team, "blastoise", {"hydro-pump", "tackle"});
    
    auto analysis = teamBuilder->analyzeTeam(team);
    
    EXPECT_GT(analysis.offensive_types.size(), 0);
    EXPECT_GT(analysis.physical_moves + analysis.special_moves + analysis.status_moves, 0);
    EXPECT_GE(analysis.balance_score, 0);
    EXPECT_LE(analysis.balance_score, 100);
}

// Test random team generation
TEST_F(TeamBuilderTest, RandomTeamGeneration) {
    auto randomTeam = teamBuilder->generateRandomTeam("Random Team", 3);
    
    EXPECT_EQ(randomTeam.name, "Random Team");
    EXPECT_EQ(randomTeam.size(), 3);
    
    // Each Pokemon should have moves
    for (const auto& pokemon : randomTeam.pokemon) {
        EXPECT_FALSE(pokemon.name.empty());
        EXPECT_GT(pokemon.moves.size(), 0);
    }
}

// Test type-focused team generation
TEST_F(TeamBuilderTest, TypeFocusedTeamGeneration) {
    auto fireTeam = teamBuilder->generateTypeFocusedTeam("Fire Team", "fire", 2);
    
    EXPECT_EQ(fireTeam.name, "Fire Team");
    EXPECT_LE(fireTeam.size(), 2);
    
    // Should contain fire-type Pokemon if available
    for (const auto& pokemon : fireTeam.pokemon) {
        EXPECT_FALSE(pokemon.name.empty());
    }
}

// Test balanced team generation
TEST_F(TeamBuilderTest, BalancedTeamGeneration) {
    auto balancedTeam = teamBuilder->generateBalancedTeam("Balanced Team", 4);
    
    EXPECT_EQ(balancedTeam.name, "Balanced Team");
    EXPECT_LE(balancedTeam.size(), 4);
    
    for (const auto& pokemon : balancedTeam.pokemon) {
        EXPECT_FALSE(pokemon.name.empty());
        EXPECT_GT(pokemon.moves.size(), 0);
    }
}

// Test team export for battle
TEST_F(TeamBuilderTest, TeamExportForBattle) {
    auto team = teamBuilder->createTeam("Export Team");
    teamBuilder->addPokemonToTeam(team, "pikachu", {"thunderbolt", "tackle"});
    teamBuilder->addPokemonToTeam(team, "charizard", {"flamethrower"});
    
    auto [selectedTeams, selectedMoves] = teamBuilder->exportTeamForBattle(team);
    
    EXPECT_FALSE(selectedTeams.empty());
    EXPECT_FALSE(selectedMoves.empty());
}

// Test team import from battle format
TEST_F(TeamBuilderTest, TeamImportFromBattle) {
    // Create test battle format data
    std::unordered_map<std::string, std::vector<std::string>> selectedTeams;
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>> selectedMoves;
    
    selectedTeams["player"] = {"pikachu", "charizard"};
    selectedMoves["player"] = {
        {"pikachu", {"thunderbolt", "tackle"}},
        {"charizard", {"flamethrower"}}
    };
    
    auto importedTeam = teamBuilder->importTeamFromBattle("Imported Team", selectedTeams, selectedMoves);
    
    EXPECT_EQ(importedTeam.name, "Imported Team");
    EXPECT_EQ(importedTeam.size(), 2);
    EXPECT_EQ(importedTeam.pokemon[0].name, "pikachu");
    EXPECT_EQ(importedTeam.pokemon[1].name, "charizard");
}

// Test team suggestions
TEST_F(TeamBuilderTest, TeamSuggestions) {
    auto team = teamBuilder->createTeam("Suggestion Team");
    teamBuilder->addPokemonToTeam(team, "pikachu", {"thunderbolt"});
    
    auto suggestions = teamBuilder->getTeamSuggestions(team);
    EXPECT_GT(suggestions.size(), 0);
    
    auto pokemonSuggestions = teamBuilder->suggestPokemonForTeam(team, 2);
    EXPECT_LE(pokemonSuggestions.size(), 2);
    
    auto moveSuggestions = teamBuilder->suggestMovesForTeamPokemon(team, 0, 2);
    EXPECT_LE(moveSuggestions.size(), 2);
}

// Test type coverage calculation
TEST_F(TeamBuilderTest, TypeCoverageCalculation) {
    auto team = teamBuilder->createTeam("Coverage Team");
    teamBuilder->addPokemonToTeam(team, "pikachu", {"thunderbolt"});
    teamBuilder->addPokemonToTeam(team, "charizard", {"flamethrower"});
    
    auto coverage = teamBuilder->calculateTypeCoverage(team);
    EXPECT_FALSE(coverage.empty());
}

// Test validation settings
TEST_F(TeamBuilderTest, ValidationSettings) {
    TeamBuilder::ValidationSettings settings;
    settings.min_team_size = 2;
    settings.max_same_type_per_team = 1;
    
    teamBuilder->setValidationSettings(settings);
    
    const auto& currentSettings = teamBuilder->getValidationSettings();
    EXPECT_EQ(currentSettings.min_team_size, 2);
}

// Test team saving and loading
TEST_F(TeamBuilderTest, TeamSaveAndLoad) {
    auto team = teamBuilder->createTeam("Save Test Team");
    teamBuilder->addPokemonToTeam(team, "pikachu", {"thunderbolt", "tackle"});
    
    std::string testFilePath = test_data_dir + "/test_team.json";
    
    bool saved = teamBuilder->saveTeamToFile(team, testFilePath);
    EXPECT_TRUE(saved);
    EXPECT_TRUE(std::filesystem::exists(testFilePath));
    
    auto loadedTeam = teamBuilder->loadTeamFromFile(testFilePath);
    EXPECT_EQ(loadedTeam.name, team.name);
    EXPECT_EQ(loadedTeam.size(), team.size());
    if (!loadedTeam.isEmpty()) {
        EXPECT_EQ(loadedTeam.pokemon[0].name, "pikachu");
    }
}

// Test edge cases for team size limits
TEST_F(TeamBuilderTest, TeamSizeLimits) {
    auto team = teamBuilder->createTeam("Size Test");
    
    // Add maximum Pokemon (6)
    std::vector<std::string> pokemon = {"pikachu", "charizard", "blastoise", "venusaur", "alakazam", "machamp"};
    for (const auto& name : pokemon) {
        teamBuilder->addPokemonToTeam(team, name, {"tackle"});
    }
    
    EXPECT_TRUE(team.isFull());
    EXPECT_EQ(team.size(), 6);
    
    // Try to add one more (should fail with default validation)
    bool extraAdded = teamBuilder->addPokemonToTeam(team, "pikachu", {"tackle"});
    EXPECT_FALSE(extraAdded);
}

// Test move validation
TEST_F(TeamBuilderTest, MoveValidation) {
    auto team = teamBuilder->createTeam("Move Test");
    
    // Add Pokemon with valid moves
    bool validAdded = teamBuilder->addPokemonToTeam(team, "pikachu", {"thunderbolt", "tackle"});
    EXPECT_TRUE(validAdded);
    
    // Try to add Pokemon with invalid moves
    bool invalidAdded = teamBuilder->addPokemonToTeam(team, "charizard", {"nonexistent-move"});
    EXPECT_FALSE(invalidAdded);
}

// Test team with empty moves
TEST_F(TeamBuilderTest, TeamWithEmptyMoves) {
    auto team = teamBuilder->createTeam("Empty Moves Test");
    
    // Add Pokemon with empty move list
    bool added = teamBuilder->addPokemonToTeam(team, "pikachu", {});
    
    // Should handle empty moves appropriately based on validation settings
    TeamBuilder::ValidationSettings settings;
    settings.enforce_min_moves = false;
    
    bool isValid = teamBuilder->validateTeam(team, settings);
    // Validation result depends on settings
}

// Test team analysis with empty team
TEST_F(TeamBuilderTest, EmptyTeamAnalysis) {
    auto team = teamBuilder->createTeam("Empty Team");
    
    auto analysis = teamBuilder->analyzeTeam(team);
    
    EXPECT_EQ(analysis.physical_moves, 0);
    EXPECT_EQ(analysis.special_moves, 0);
    EXPECT_EQ(analysis.status_moves, 0);
    EXPECT_EQ(analysis.balance_score, 0);
}

// Test random team generation with size limits
TEST_F(TeamBuilderTest, RandomTeamGenerationSizeLimits) {
    // Test various team sizes
    for (int size = 1; size <= 6; ++size) {
        auto team = teamBuilder->generateRandomTeam("Random " + std::to_string(size), size);
        EXPECT_LE(team.size(), size);
        EXPECT_GT(team.size(), 0); // Should generate at least one Pokemon if data is available
    }
}

// Test error handling with null/invalid data
TEST_F(TeamBuilderTest, ErrorHandlingNullData) {
    // This test verifies robust error handling
    auto team = teamBuilder->createTeam("Error Test");
    
    // Try operations that might cause errors
    bool added = teamBuilder->addPokemonToTeam(team, "", {});
    EXPECT_FALSE(added);
    
    bool removed = teamBuilder->removePokemonFromTeam(team, 999);
    EXPECT_FALSE(removed);
    
    bool modified = teamBuilder->modifyPokemonMoves(team, 999, {});
    EXPECT_FALSE(modified);
}

// Test performance with large operations
TEST_F(TeamBuilderTest, PerformanceLargeOperations) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Generate multiple teams
    for (int i = 0; i < 10; ++i) {
        auto team = teamBuilder->generateRandomTeam("Perf Team " + std::to_string(i), 3);
        teamBuilder->analyzeTeam(team);
        teamBuilder->validateTeam(team);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete within reasonable time
    EXPECT_LT(duration.count(), 5000); // Less than 5 seconds
}

// Test team comparison functionality (if available in basic interface)
TEST_F(TeamBuilderTest, BasicTeamComparison) {
    auto team1 = teamBuilder->createTeam("Team 1");
    teamBuilder->addPokemonToTeam(team1, "pikachu", {"thunderbolt"});
    teamBuilder->addPokemonToTeam(team1, "charizard", {"flamethrower"});
    
    auto team2 = teamBuilder->createTeam("Team 2");
    teamBuilder->addPokemonToTeam(team2, "blastoise", {"hydro-pump"});
    teamBuilder->addPokemonToTeam(team2, "venusaur", {"solar-beam"});
    
    // Analyze both teams
    auto analysis1 = teamBuilder->analyzeTeam(team1);
    auto analysis2 = teamBuilder->analyzeTeam(team2);
    
    // Both should have valid analyses
    EXPECT_GE(analysis1.balance_score, 0);
    EXPECT_GE(analysis2.balance_score, 0);
}

// Test team builder with different PokemonData configurations
TEST_F(TeamBuilderTest, DifferentDataConfigurations) {
    // Test team builder behavior when PokemonData has limited data
    auto limitedData = std::make_shared<PokemonData>();
    // Initialize with limited data (empty directories)
    std::filesystem::create_directories("limited_test_data/pokemon");
    std::filesystem::create_directories("limited_test_data/moves");
    limitedData->initialize("limited_test_data/pokemon", "limited_test_data/moves");
    
    TeamBuilder limitedBuilder(limitedData);
    
    auto team = limitedBuilder.createTeam("Limited Team");
    bool added = limitedBuilder.addPokemonToTeam(team, "nonexistent", {"move"});
    EXPECT_FALSE(added);
    
    // Clean up
    std::filesystem::remove_all("limited_test_data");
}