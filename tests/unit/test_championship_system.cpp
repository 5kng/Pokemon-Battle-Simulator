#include <gtest/gtest.h>
#include <memory>
#include <filesystem>
#include <fstream>
#include "test_utils.h"
#include "championship_system.h"
#include "tournament_manager.h"
#include "team_builder.h"
#include "pokemon_data.h"

namespace fs = std::filesystem;

// Mock Tournament Manager for testing
class MockTournamentManager : public TournamentManager {
public:
    MockTournamentManager() : TournamentManager(nullptr, nullptr) {}
    
    // Override key methods for testing
    bool isChampionshipUnlocked(const std::string& player_name) const {
        return championshipUnlocked.find(player_name) != championshipUnlocked.end() &&
               championshipUnlocked.at(player_name);
    }
    
    std::optional<TournamentProgress> getPlayerProgress(const std::string& player_name) const {
        auto it = playerProgress.find(player_name);
        if (it != playerProgress.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    bool updatePlayerProgress(const std::string& player_name, const TournamentBattleResult& result) {
        // Store the result
        battleResults[player_name].push_back(result);
        
        // Update progress if champion defeated
        if (result.challenge_type == "champion" && result.victory) {
            if (playerProgress.find(player_name) != playerProgress.end()) {
                playerProgress[player_name].champion_defeated = true;
            }
        }
        
        return true;
    }
    
    // Test helper methods
    void setChampionshipUnlocked(const std::string& player_name, bool unlocked) {
        championshipUnlocked[player_name] = unlocked;
    }
    
    void setPlayerProgress(const std::string& player_name, const TournamentProgress& progress) {
        playerProgress[player_name] = progress;
    }
    
    std::vector<TournamentBattleResult> getBattleResults(const std::string& player_name) const {
        auto it = battleResults.find(player_name);
        if (it != battleResults.end()) {
            return it->second;
        }
        return {};
    }

private:
    std::unordered_map<std::string, bool> championshipUnlocked;
    std::unordered_map<std::string, TournamentProgress> playerProgress;
    std::unordered_map<std::string, std::vector<TournamentBattleResult>> battleResults;
};

// Mock Team Builder for testing
class MockTeamBuilder : public TeamBuilder {
public:
    MockTeamBuilder() : TeamBuilder(nullptr) {}
    
    Team generateTeamFromTemplate(const std::string& category, const std::string& template_name) {
        Team team;
        team.name = template_name + " Team";
        
        // Create mock team members based on template
        if (template_name == "ice_team") {
            team.pokemon.push_back(TeamPokemon("Lapras", {"ice-beam", "surf", "psychic", "thunderbolt"}));
            team.pokemon.push_back(TeamPokemon("Articuno", {"blizzard", "aerial-ace", "ice-shard", "roost"}));
        } else if (template_name == "fighting_team") {
            team.pokemon.push_back(TeamPokemon("Machamp", {"close-combat", "earthquake", "stone-edge", "fire-punch"}));
            team.pokemon.push_back(TeamPokemon("Hitmonlee", {"high-jump-kick", "stone-edge", "earthquake", "blaze-kick"}));
        } else if (template_name == "dark_team") {
            team.pokemon.push_back(TeamPokemon("Gengar", {"shadow-ball", "sludge-bomb", "thunderbolt", "hypnosis"}));
            team.pokemon.push_back(TeamPokemon("Crobat", {"air-slash", "poison-fang", "u-turn", "roost"}));
        } else {
            // Default balanced team
            team.pokemon.push_back(TeamPokemon("Charizard", {"flamethrower", "air-slash", "dragon-claw", "roost"}));
            team.pokemon.push_back(TeamPokemon("Blastoise", {"surf", "ice-beam", "earthquake", "rapid-spin"}));
        }
        
        return team;
    }
};

class ChampionshipSystemTest : public TestUtils::PokemonTestFixture {
protected:
    void SetUp() override {
        TestUtils::PokemonTestFixture::SetUp();
        
        // Create mock dependencies
        pokemonData = std::make_shared<PokemonData>();
        pokemonData->initialize("data/pokemon", "data/moves"); // Initialize with test data
        teamBuilder = std::make_shared<MockTeamBuilder>();
        tournamentManager = std::make_shared<MockTournamentManager>();
        
        // Create championship system
        championshipSystem = std::make_unique<ChampionshipSystem>(pokemonData, teamBuilder, tournamentManager);
        
        // Clean up any existing test data
        cleanupTestData();
        
        // Set up test player with championship access
        setupTestPlayer();
    }
    
    void TearDown() override {
        cleanupTestData();
    }
    
    void setupTestPlayer() {
        // Set up player with tournament progress
        TournamentManager::TournamentProgress progress;
        progress.earned_badges = std::vector<TournamentManager::Badge>(8);
        progress.defeated_elite_four = {"Lorelei", "Bruno", "Agatha", "Lance"};
        progress.elite_four_completed = true;
        progress.champion_defeated = false;
        
        tournamentManager->setPlayerProgress(testPlayerName, progress);
        tournamentManager->setChampionshipUnlocked(testPlayerName, true);
    }
    
    void cleanupTestData() {
        // Remove any test data files
        const std::string testDataPath = "data/tournaments/championship_data.json";
        if (fs::exists(testDataPath)) {
            fs::remove(testDataPath);
        }
    }
    
    TeamBuilder::Team createTestPlayerTeam() {
        TeamBuilder::Team team;
        team.name = "Test Team";
        team.pokemon.push_back(TeamBuilder::TeamPokemon("Pikachu", {"thunderbolt", "quick-attack", "thunder-wave", "agility"}));
        team.pokemon.push_back(TeamBuilder::TeamPokemon("Charizard", {"flamethrower", "air-slash", "dragon-claw", "roost"}));
        team.is_valid = true;
        return team;
    }
    
    std::unique_ptr<ChampionshipSystem> championshipSystem;
    std::shared_ptr<PokemonData> pokemonData;
    std::shared_ptr<MockTeamBuilder> teamBuilder;
    std::shared_ptr<MockTournamentManager> tournamentManager;
    
    const std::string testPlayerName = "TestPlayer";
    const std::string testTeamName = "TestTeam";
};

// Test championship system construction and initialization
TEST_F(ChampionshipSystemTest, ConstructionAndInitialization) {
    EXPECT_NE(championshipSystem, nullptr);
    
    // Test that Elite Four opponents are initialized
    auto opponents = championshipSystem->getChampionshipOpponents();
    EXPECT_EQ(opponents.size(), 5); // 4 Elite Four + 1 Champion
    
    // Verify Elite Four structure
    for (int i = 0; i < 4; ++i) {
        EXPECT_FALSE(opponents[i].is_champion);
        EXPECT_EQ(opponents[i].position_in_sequence, i + 1);
        EXPECT_EQ(opponents[i].title, "Elite Four Member");
    }
    
    // Verify Champion
    EXPECT_TRUE(opponents[4].is_champion);
    EXPECT_EQ(opponents[4].position_in_sequence, 5);
    EXPECT_EQ(opponents[4].title, "Pokemon Champion");
}

// Test championship eligibility checking
TEST_F(ChampionshipSystemTest, ChampionshipEligibility) {
    // Test eligible player
    EXPECT_TRUE(championshipSystem->isChampionshipEligible(testPlayerName));
    
    // Test ineligible player (no tournament progress)
    EXPECT_FALSE(championshipSystem->isChampionshipEligible("UnknownPlayer"));
    
    // Test player with tournament progress but no championship unlock
    tournamentManager->setChampionshipUnlocked(testPlayerName, false);
    EXPECT_FALSE(championshipSystem->isChampionshipEligible(testPlayerName));
    
    // Restore eligibility
    tournamentManager->setChampionshipUnlocked(testPlayerName, true);
    EXPECT_TRUE(championshipSystem->isChampionshipEligible(testPlayerName));
}

// Test starting a championship run
TEST_F(ChampionshipSystemTest, StartChampionshipRun) {
    // Test successful run start
    EXPECT_TRUE(championshipSystem->startChampionshipRun(testPlayerName, testTeamName));
    
    // Verify run was created
    auto run = championshipSystem->getCurrentRun(testPlayerName);
    ASSERT_TRUE(run.has_value());
    EXPECT_EQ(run->player_name, testPlayerName);
    EXPECT_EQ(run->player_team_name, testTeamName);
    EXPECT_EQ(run->current_position, 1);
    EXPECT_TRUE(run->is_active);
    EXPECT_FALSE(run->is_completed);
    EXPECT_EQ(run->defeated_opponents.size(), 0);
    
    // Test cannot start second run for same player
    EXPECT_FALSE(championshipSystem->startChampionshipRun(testPlayerName, "AnotherTeam"));
    
    // Test ineligible player cannot start run
    EXPECT_FALSE(championshipSystem->startChampionshipRun("IneligiblePlayer", testTeamName));
}

// Test championship run progression through Elite Four
TEST_F(ChampionshipSystemTest, EliteFourProgression) {
    // Start championship run
    ASSERT_TRUE(championshipSystem->startChampionshipRun(testPlayerName, testTeamName));
    
    // Test progression through Elite Four
    for (int position = 1; position <= 4; ++position) {
        auto run = championshipSystem->getCurrentRun(testPlayerName);
        ASSERT_TRUE(run.has_value());
        EXPECT_EQ(run->current_position, position);
        
        // Get next opponent
        auto opponent = championshipSystem->getNextOpponent(testPlayerName);
        ASSERT_TRUE(opponent.has_value());
        EXPECT_FALSE(opponent->is_champion);
        EXPECT_EQ(opponent->position_in_sequence, position);
        
        // Execute battle (simulate victory)
        auto playerTeam = createTestPlayerTeam();
        auto battleResult = championshipSystem->executeBattle(testPlayerName, playerTeam);
        battleResult.victory = true; // Force victory for progression test
        
        // Record battle result
        EXPECT_TRUE(championshipSystem->recordBattleResult(testPlayerName, battleResult));
        
        // Check progression
        run = championshipSystem->getCurrentRun(testPlayerName);
        if (position < 4) {
            ASSERT_TRUE(run.has_value());
            EXPECT_EQ(run->current_position, position + 1);
            EXPECT_EQ(run->defeated_opponents.size(), position);
            EXPECT_TRUE(run->is_active);
        }
    }
    
    // After defeating all Elite Four, should be at Champion position
    auto finalRun = championshipSystem->getCurrentRun(testPlayerName);
    ASSERT_TRUE(finalRun.has_value());
    EXPECT_EQ(finalRun->current_position, 5);
    EXPECT_EQ(finalRun->defeated_opponents.size(), 4);
}

// Test Champion battle mechanics
TEST_F(ChampionshipSystemTest, ChampionBattleMechanics) {
    // Start and progress through Elite Four
    ASSERT_TRUE(championshipSystem->startChampionshipRun(testPlayerName, testTeamName));
    
    // Skip to Champion battle
    auto playerTeam = createTestPlayerTeam();
    for (int i = 1; i <= 4; ++i) {
        auto battleResult = championshipSystem->executeBattle(testPlayerName, playerTeam);
        battleResult.victory = true;
        championshipSystem->recordBattleResult(testPlayerName, battleResult);
    }
    
    // Test Champion battle
    auto run = championshipSystem->getCurrentRun(testPlayerName);
    ASSERT_TRUE(run.has_value());
    EXPECT_EQ(run->current_position, 5);
    
    // Get Champion opponent
    auto champion = championshipSystem->getNextOpponent(testPlayerName);
    ASSERT_TRUE(champion.has_value());
    EXPECT_TRUE(champion->is_champion);
    EXPECT_EQ(champion->position_in_sequence, 5);
    EXPECT_EQ(champion->name, "Champion");
    
    // Execute Champion battle
    auto championBattle = championshipSystem->executeBattle(testPlayerName, playerTeam);
    EXPECT_EQ(championBattle.opponent_type, "champion");
    EXPECT_EQ(championBattle.opponent_position, 5);
    
    // Test victory
    championBattle.victory = true;
    EXPECT_TRUE(championshipSystem->recordBattleResult(testPlayerName, championBattle));
    
    // Verify championship completion
    run = championshipSystem->getCurrentRun(testPlayerName);
    EXPECT_FALSE(run.has_value()); // Run should be completed and no longer active
    
    EXPECT_TRUE(championshipSystem->isPlayerChampion(testPlayerName));
    EXPECT_DOUBLE_EQ(championshipSystem->getChampionshipProgress(testPlayerName), 1.0);
}

// Test championship run failure and restart
TEST_F(ChampionshipSystemTest, ChampionshipRunFailure) {
    // Start championship run
    ASSERT_TRUE(championshipSystem->startChampionshipRun(testPlayerName, testTeamName));
    
    // Execute battle and lose
    auto playerTeam = createTestPlayerTeam();
    auto battleResult = championshipSystem->executeBattle(testPlayerName, playerTeam);
    battleResult.victory = false;
    
    EXPECT_TRUE(championshipSystem->recordBattleResult(testPlayerName, battleResult));
    
    // Run should be ended
    auto run = championshipSystem->getCurrentRun(testPlayerName);
    EXPECT_FALSE(run.has_value()); // Run ended due to defeat
    
    // Should be able to start new run
    EXPECT_TRUE(championshipSystem->startChampionshipRun(testPlayerName, testTeamName));
}

// Test healing mechanics between battles
TEST_F(ChampionshipSystemTest, HealingMechanics) {
    // Start championship run
    ASSERT_TRUE(championshipSystem->startChampionshipRun(testPlayerName, testTeamName));
    
    auto playerTeam = createTestPlayerTeam();
    
    // Test healing allowed between Elite Four members
    EXPECT_TRUE(championshipSystem->isHealingAllowed(testPlayerName));
    
    // Execute battle
    auto battleResult = championshipSystem->executeBattle(testPlayerName, playerTeam);
    battleResult.victory = true;
    battleResult.team_needs_healing = true;
    championshipSystem->recordBattleResult(testPlayerName, battleResult);
    
    // Still should allow healing between Elite Four
    EXPECT_TRUE(championshipSystem->isHealingAllowed(testPlayerName));
    
    // Test healing
    EXPECT_TRUE(championshipSystem->healPlayerTeam(testPlayerName, playerTeam));
    
    // Test team needs healing detection
    EXPECT_TRUE(championshipSystem->doesTeamNeedHealing(testPlayerName, playerTeam));
}

// Test championship statistics tracking
TEST_F(ChampionshipSystemTest, StatisticsTracking) {
    // Initial stats should be empty
    auto stats = championshipSystem->getChampionshipStats(testPlayerName);
    EXPECT_DOUBLE_EQ(stats["total_attempts"], 0.0);
    EXPECT_DOUBLE_EQ(stats["victories"], 0.0);
    EXPECT_DOUBLE_EQ(stats["defeats"], 0.0);
    
    // Complete a championship run
    ASSERT_TRUE(championshipSystem->startChampionshipRun(testPlayerName, testTeamName));
    
    auto playerTeam = createTestPlayerTeam();
    
    // Progress through Elite Four and Champion
    for (int i = 1; i <= 5; ++i) {
        auto battleResult = championshipSystem->executeBattle(testPlayerName, playerTeam);
        battleResult.victory = true;
        battleResult.performance_score = 85.0 + i * 2; // Increasing scores
        championshipSystem->recordBattleResult(testPlayerName, battleResult);
    }
    
    // Check updated stats
    stats = championshipSystem->getChampionshipStats(testPlayerName);
    EXPECT_DOUBLE_EQ(stats["total_attempts"], 1.0);
    EXPECT_DOUBLE_EQ(stats["victories"], 1.0);
    EXPECT_DOUBLE_EQ(stats["defeats"], 0.0);
    EXPECT_GT(stats["average_performance_score"], 85.0);
    EXPECT_GT(stats["best_completion_time"], 0.0);
}

// Test championship history tracking
TEST_F(ChampionshipSystemTest, HistoryTracking) {
    // Initial history should be empty
    auto history = championshipSystem->getChampionshipHistory(testPlayerName);
    EXPECT_EQ(history.size(), 0);
    
    // Start and complete partial run
    ASSERT_TRUE(championshipSystem->startChampionshipRun(testPlayerName, testTeamName));
    
    auto playerTeam = createTestPlayerTeam();
    
    // Win first two battles
    for (int i = 1; i <= 2; ++i) {
        auto battleResult = championshipSystem->executeBattle(testPlayerName, playerTeam);
        battleResult.victory = true;
        battleResult.opponent_position = i;
        championshipSystem->recordBattleResult(testPlayerName, battleResult);
    }
    
    // Lose third battle
    auto lostBattle = championshipSystem->executeBattle(testPlayerName, playerTeam);
    lostBattle.victory = false;
    lostBattle.opponent_position = 3;
    championshipSystem->recordBattleResult(testPlayerName, lostBattle);
    
    // Check history
    history = championshipSystem->getChampionshipHistory(testPlayerName);
    EXPECT_EQ(history.size(), 3);
    
    // Verify battle details
    EXPECT_TRUE(history[0].victory);
    EXPECT_TRUE(history[1].victory);
    EXPECT_FALSE(history[2].victory);
    
    EXPECT_EQ(history[0].opponent_position, 1);
    EXPECT_EQ(history[1].opponent_position, 2);
    EXPECT_EQ(history[2].opponent_position, 3);
}

// Test championship leaderboard functionality
TEST_F(ChampionshipSystemTest, LeaderboardFunctionality) {
    // Add multiple players with different performances
    std::vector<std::string> players = {"Player1", "Player2", "Player3"};
    std::vector<double> completionTimes = {45.5, 38.2, 52.1};
    
    for (size_t i = 0; i < players.size(); ++i) {
        // Set up player eligibility
        tournamentManager->setChampionshipUnlocked(players[i], true);
        TournamentManager::TournamentProgress progress;
        progress.earned_badges = std::vector<TournamentManager::Badge>(8);
        tournamentManager->setPlayerProgress(players[i], progress);
        
        // Complete championship
        ASSERT_TRUE(championshipSystem->startChampionshipRun(players[i], "Team" + std::to_string(i)));
        
        auto playerTeam = createTestPlayerTeam();
        for (int j = 1; j <= 5; ++j) {
            auto battleResult = championshipSystem->executeBattle(players[i], playerTeam);
            battleResult.victory = true;
            battleResult.performance_score = 80.0 + i * 5;
            championshipSystem->recordBattleResult(players[i], battleResult);
        }
    }
    
    // Test leaderboard sorting by score
    auto leaderboard = championshipSystem->getChampionshipLeaderboard("score", 10);
    EXPECT_GE(leaderboard.size(), 3);
    
    // Test leaderboard sorting by time (not fully implemented in mock, but should not crash)
    auto timeLeaderboard = championshipSystem->getChampionshipLeaderboard("time", 5);
    EXPECT_GE(timeLeaderboard.size(), 0);
}

// Test data persistence
TEST_F(ChampionshipSystemTest, DataPersistence) {
    // Start championship run
    ASSERT_TRUE(championshipSystem->startChampionshipRun(testPlayerName, testTeamName));
    
    // Execute some battles
    auto playerTeam = createTestPlayerTeam();
    for (int i = 1; i <= 2; ++i) {
        auto battleResult = championshipSystem->executeBattle(testPlayerName, playerTeam);
        battleResult.victory = true;
        championshipSystem->recordBattleResult(testPlayerName, battleResult);
    }
    
    // Save data
    EXPECT_TRUE(championshipSystem->saveChampionshipData());
    
    // Verify data file exists
    EXPECT_TRUE(fs::exists("data/tournaments/championship_data.json"));
    
    // Create new championship system instance
    auto newChampionshipSystem = std::make_unique<ChampionshipSystem>(pokemonData, teamBuilder, tournamentManager);
    
    // Load data
    EXPECT_TRUE(newChampionshipSystem->loadChampionshipData());
    
    // Verify run was restored
    auto restoredRun = newChampionshipSystem->getCurrentRun(testPlayerName);
    ASSERT_TRUE(restoredRun.has_value());
    EXPECT_EQ(restoredRun->current_position, 3); // Should be at position 3 after 2 victories
    EXPECT_EQ(restoredRun->defeated_opponents.size(), 2);
}

// Test championship progress tracking
TEST_F(ChampionshipSystemTest, ProgressTracking) {
    // Initial progress should be 0
    EXPECT_DOUBLE_EQ(championshipSystem->getChampionshipProgress(testPlayerName), 0.0);
    
    // Start run
    ASSERT_TRUE(championshipSystem->startChampionshipRun(testPlayerName, testTeamName));
    EXPECT_DOUBLE_EQ(championshipSystem->getChampionshipProgress(testPlayerName), 0.0);
    
    auto playerTeam = createTestPlayerTeam();
    
    // Progress through battles
    for (int i = 1; i <= 4; ++i) {
        auto battleResult = championshipSystem->executeBattle(testPlayerName, playerTeam);
        battleResult.victory = true;
        championshipSystem->recordBattleResult(testPlayerName, battleResult);
        
        double expectedProgress = static_cast<double>(i) / 5.0;
        EXPECT_DOUBLE_EQ(championshipSystem->getChampionshipProgress(testPlayerName), expectedProgress);
    }
    
    // Complete championship
    auto finalBattle = championshipSystem->executeBattle(testPlayerName, playerTeam);
    finalBattle.victory = true;
    championshipSystem->recordBattleResult(testPlayerName, finalBattle);
    
    // Should be 100% complete
    EXPECT_DOUBLE_EQ(championshipSystem->getChampionshipProgress(testPlayerName), 1.0);
}

// Test championship system status and validation
TEST_F(ChampionshipSystemTest, SystemStatusAndValidation) {
    // Test initial system status
    auto status = championshipSystem->getChampionshipSystemStatus();
    EXPECT_EQ(status["active_runs"], "0");
    EXPECT_EQ(status["total_players_with_history"], "0");
    EXPECT_EQ(status["data_valid"], "true");
    EXPECT_EQ(status["total_champions"], "0");
    
    // Start run and check status
    ASSERT_TRUE(championshipSystem->startChampionshipRun(testPlayerName, testTeamName));
    
    status = championshipSystem->getChampionshipSystemStatus();
    EXPECT_EQ(status["active_runs"], "1");
    
    // Test data validation
    EXPECT_TRUE(championshipSystem->validateChampionshipData());
}

// Test championship run resumption
TEST_F(ChampionshipSystemTest, RunResumption) {
    // Start championship run
    ASSERT_TRUE(championshipSystem->startChampionshipRun(testPlayerName, testTeamName));
    
    // Execute one battle
    auto playerTeam = createTestPlayerTeam();
    auto battleResult = championshipSystem->executeBattle(testPlayerName, playerTeam);
    battleResult.victory = true;
    championshipSystem->recordBattleResult(testPlayerName, battleResult);
    
    // Save and reload to simulate resumption
    championshipSystem->saveChampionshipData();
    auto newChampionshipSystem = std::make_unique<ChampionshipSystem>(pokemonData, teamBuilder, tournamentManager);
    newChampionshipSystem->loadChampionshipData();
    
    // Test resumption
    EXPECT_TRUE(newChampionshipSystem->resumeChampionshipRun(testPlayerName));
    
    auto resumedRun = newChampionshipSystem->getCurrentRun(testPlayerName);
    ASSERT_TRUE(resumedRun.has_value());
    EXPECT_TRUE(resumedRun->is_active);
    EXPECT_EQ(resumedRun->current_position, 2);
}

// Test championship reset functionality
TEST_F(ChampionshipSystemTest, ChampionshipReset) {
    // Complete championship
    ASSERT_TRUE(championshipSystem->startChampionshipRun(testPlayerName, testTeamName));
    
    auto playerTeam = createTestPlayerTeam();
    for (int i = 1; i <= 5; ++i) {
        auto battleResult = championshipSystem->executeBattle(testPlayerName, playerTeam);
        battleResult.victory = true;
        championshipSystem->recordBattleResult(testPlayerName, battleResult);
    }
    
    // Verify championship completion
    EXPECT_TRUE(championshipSystem->isPlayerChampion(testPlayerName));
    EXPECT_GT(championshipSystem->getChampionshipHistory(testPlayerName).size(), 0);
    
    // Reset without confirmation should fail
    EXPECT_FALSE(championshipSystem->resetPlayerChampionshipProgress(testPlayerName, false));
    
    // Reset with confirmation should succeed
    EXPECT_TRUE(championshipSystem->resetPlayerChampionshipProgress(testPlayerName, true));
    
    // Verify reset
    EXPECT_FALSE(championshipSystem->isPlayerChampion(testPlayerName));
    EXPECT_EQ(championshipSystem->getChampionshipHistory(testPlayerName).size(), 0);
    EXPECT_DOUBLE_EQ(championshipSystem->getChampionshipProgress(testPlayerName), 0.0);
}

// Test battle execution with different outcomes
TEST_F(ChampionshipSystemTest, BattleExecutionOutcomes) {
    // Start championship run
    ASSERT_TRUE(championshipSystem->startChampionshipRun(testPlayerName, testTeamName));
    
    auto playerTeam = createTestPlayerTeam();
    
    // Execute battle
    auto battleResult = championshipSystem->executeBattle(testPlayerName, playerTeam);
    
    // Verify battle result structure
    EXPECT_EQ(battleResult.player_name, testPlayerName);
    EXPECT_FALSE(battleResult.opponent_name.empty());
    EXPECT_EQ(battleResult.opponent_type, "elite_four");
    EXPECT_EQ(battleResult.opponent_position, 1);
    EXPECT_GT(battleResult.turns_taken, 0);
    EXPECT_FALSE(battleResult.difficulty_level.empty());
    EXPECT_GE(battleResult.performance_score, 0.0);
    EXPECT_FALSE(battleResult.battle_duration.empty());
}

// Test championship settings configuration
TEST_F(ChampionshipSystemTest, ChampionshipSettings) {
    // Get default settings
    auto settings = championshipSystem->getSettings();
    EXPECT_TRUE(settings.require_sequential_battles);
    EXPECT_TRUE(settings.allow_healing_between_elite_four);
    EXPECT_TRUE(settings.progressive_difficulty);
    
    // Modify settings
    ChampionshipSystem::ChampionshipSettings newSettings = settings;
    newSettings.require_sequential_battles = false;
    newSettings.allow_healing_between_elite_four = false;
    newSettings.max_championship_attempts = 3;
    
    championshipSystem->setSettings(newSettings);
    
    // Verify settings updated
    auto updatedSettings = championshipSystem->getSettings();
    EXPECT_FALSE(updatedSettings.require_sequential_battles);
    EXPECT_FALSE(updatedSettings.allow_healing_between_elite_four);
    EXPECT_EQ(updatedSettings.max_championship_attempts, 3);
}

// Test error handling and edge cases
TEST_F(ChampionshipSystemTest, ErrorHandlingAndEdgeCases) {
    // Test operations on non-existent player
    EXPECT_FALSE(championshipSystem->getCurrentRun("NonExistentPlayer").has_value());
    EXPECT_FALSE(championshipSystem->resumeChampionshipRun("NonExistentPlayer"));
    EXPECT_FALSE(championshipSystem->endChampionshipRun("NonExistentPlayer", true));
    EXPECT_FALSE(championshipSystem->isPlayerChampion("NonExistentPlayer"));
    
    // Test operations without active run
    EXPECT_FALSE(championshipSystem->getNextOpponent(testPlayerName).has_value());
    {
        auto tempTeam = createTestPlayerTeam();
        EXPECT_FALSE(championshipSystem->doesTeamNeedHealing(testPlayerName, tempTeam));
    }
    {
        auto tempTeam = createTestPlayerTeam();
        EXPECT_FALSE(championshipSystem->healPlayerTeam(testPlayerName, tempTeam));
    }
    
    // Test championship system with invalid dependencies
    EXPECT_THROW(
        ChampionshipSystem(nullptr, teamBuilder, tournamentManager),
        std::invalid_argument
    );
    
    EXPECT_THROW(
        ChampionshipSystem(pokemonData, nullptr, tournamentManager),
        std::invalid_argument
    );
    
    EXPECT_THROW(
        ChampionshipSystem(pokemonData, teamBuilder, nullptr),
        std::invalid_argument
    );
}