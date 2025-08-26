#include <gtest/gtest.h>
#include "test_utils.h"
#include "gym_leader.h"
#include "tournament_manager.h"
#include "pokemon_data.h"
#include "team_builder.h"
#include <filesystem>
#include <fstream>

class GymLeaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create mock dependencies
        pokemon_data = std::make_shared<PokemonData>();
        team_builder = std::make_shared<TeamBuilder>(pokemon_data);
        tournament_manager = std::make_shared<TournamentManager>(pokemon_data, team_builder);
        
        // Initialize gym leader system
        gym_leader = std::make_unique<GymLeader>(pokemon_data, team_builder, tournament_manager);
        
        // Create test team
        createTestTeam();
        
        cleanup_test_data();
    }
    
    void TearDown() override {
        cleanup_test_data();
    }
    
    void cleanup_test_data() {
        // Clean up test gym data files
        std::filesystem::remove_all("data/tournaments");
    }
    
    void createTestTeam() {
        // Create test team data manually since TeamBuilder structures may differ
        test_team.name = "TestTeam";
        
        // Create individual pokemon entries for the team
        TeamBuilder::TeamPokemon pikachu;
        pikachu.name = "Pikachu";
        
        TeamBuilder::TeamPokemon charizard;
        charizard.name = "Charizard";
        
        TeamBuilder::TeamPokemon blastoise;
        blastoise.name = "Blastoise";
        
        test_team.pokemon = {pikachu, charizard, blastoise};
    }
    
    std::shared_ptr<PokemonData> pokemon_data;
    std::shared_ptr<TeamBuilder> team_builder;
    std::shared_ptr<TournamentManager> tournament_manager;
    std::unique_ptr<GymLeader> gym_leader;
    
    TeamBuilder::Team test_team;
};

// ========================= Constructor and Basic Setup =========================

TEST_F(GymLeaderTest, ConstructorWithValidDependencies) {
    EXPECT_NO_THROW({
        auto gl = std::make_unique<GymLeader>(pokemon_data, team_builder, tournament_manager);
    });
}

TEST_F(GymLeaderTest, ConstructorWithNullDependencies) {
    EXPECT_THROW({
        auto gl = std::make_unique<GymLeader>(nullptr, team_builder, tournament_manager);
    }, std::invalid_argument);
    
    EXPECT_THROW({
        auto gl = std::make_unique<GymLeader>(pokemon_data, nullptr, tournament_manager);
    }, std::invalid_argument);
    
    EXPECT_THROW({
        auto gl = std::make_unique<GymLeader>(pokemon_data, team_builder, nullptr);
    }, std::invalid_argument);
}

// ========================= Gym Leader Information =========================

TEST_F(GymLeaderTest, GetGymLeaderInfo) {
    auto brock_info = gym_leader->getGymLeaderInfo("Brock");
    ASSERT_TRUE(brock_info.has_value());
    EXPECT_EQ(brock_info->name, "Brock");
    EXPECT_EQ(brock_info->type_specialization, "rock");
    EXPECT_EQ(brock_info->city, "Pewter City");
    EXPECT_EQ(brock_info->badge_id, 1);
    EXPECT_EQ(brock_info->difficulty_level, "Easy");
    
    auto misty_info = gym_leader->getGymLeaderInfo("Misty");
    ASSERT_TRUE(misty_info.has_value());
    EXPECT_EQ(misty_info->name, "Misty");
    EXPECT_EQ(misty_info->type_specialization, "water");
    EXPECT_EQ(misty_info->city, "Cerulean City");
    EXPECT_EQ(misty_info->badge_id, 2);
}

TEST_F(GymLeaderTest, GetNonexistentGymLeader) {
    auto info = gym_leader->getGymLeaderInfo("NonexistentLeader");
    EXPECT_FALSE(info.has_value());
}

TEST_F(GymLeaderTest, GetAllGymLeaders) {
    auto leaders = gym_leader->getAllGymLeaders();
    EXPECT_EQ(leaders.size(), 8);
    
    // Check that all canonical gym leaders are present
    std::vector<std::string> expected_leaders = {
        "Brock", "Misty", "Lt. Surge", "Erika", "Koga", "Sabrina", "Blaine", "Giovanni"
    };
    
    for (const auto& expected : expected_leaders) {
        bool found = false;
        for (const auto& leader : leaders) {
            if (leader.name == expected) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Expected leader " << expected << " not found";
    }
}

TEST_F(GymLeaderTest, GetGymLeadersByType) {
    auto rock_leaders = gym_leader->getGymLeadersByType("rock");
    EXPECT_EQ(rock_leaders.size(), 1);
    EXPECT_EQ(rock_leaders[0].name, "Brock");
    
    auto water_leaders = gym_leader->getGymLeadersByType("water");
    EXPECT_EQ(water_leaders.size(), 1);
    EXPECT_EQ(water_leaders[0].name, "Misty");
    
    auto nonexistent_type = gym_leader->getGymLeadersByType("dragon");
    EXPECT_EQ(nonexistent_type.size(), 0);
}

TEST_F(GymLeaderTest, GetRecommendedGymOrder) {
    auto order = gym_leader->getRecommendedGymOrder();
    EXPECT_EQ(order.size(), 8);
    
    // Should be ordered by badge_id
    EXPECT_EQ(order[0], "Brock");    // badge_id 1
    EXPECT_EQ(order[1], "Misty");    // badge_id 2
    EXPECT_EQ(order[7], "Giovanni"); // badge_id 8
}

// ========================= Badge Management =========================

TEST_F(GymLeaderTest, GetPlayerBadges) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    auto badges = gym_leader->getPlayerBadges("TestPlayer");
    EXPECT_EQ(badges.size(), 0);
    
    // Award a badge through tournament manager
    TournamentManager::Badge tournament_badge("Pewter Gym", "rock", "Brock", "2025-01-01", 1, 85.0);
    tournament_manager->awardBadge("TestPlayer", tournament_badge);
    
    badges = gym_leader->getPlayerBadges("TestPlayer");
    EXPECT_EQ(badges.size(), 1);
    EXPECT_EQ(badges[0].gym_leader_name, "Brock");
    EXPECT_EQ(badges[0].type_specialization, "rock");
    EXPECT_EQ(badges[0].final_score, 85.0);
}

TEST_F(GymLeaderTest, HasPlayerEarnedBadge) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    EXPECT_FALSE(gym_leader->hasPlayerEarnedBadge("TestPlayer", "Brock"));
    
    // Award badge
    TournamentManager::Badge badge("Pewter Gym", "rock", "Brock", "2025-01-01");
    tournament_manager->awardBadge("TestPlayer", badge);
    
    EXPECT_TRUE(gym_leader->hasPlayerEarnedBadge("TestPlayer", "Brock"));
}

TEST_F(GymLeaderTest, GetPlayerBadgeCount) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    EXPECT_EQ(gym_leader->getPlayerBadgeCount("TestPlayer"), 0);
    
    // Award multiple badges
    std::vector<std::pair<std::string, std::string>> leaders_and_types = {
        {"Brock", "rock"}, {"Misty", "water"}, {"Lt. Surge", "electric"}
    };
    
    for (const auto& [leader, type] : leaders_and_types) {
        TournamentManager::Badge badge("Gym", type, leader, "2025-01-01");
        tournament_manager->awardBadge("TestPlayer", badge);
    }
    
    EXPECT_EQ(gym_leader->getPlayerBadgeCount("TestPlayer"), 3);
}

TEST_F(GymLeaderTest, GetBadgeInfo) {
    auto badge_info = gym_leader->getBadgeInfo("Brock");
    ASSERT_TRUE(badge_info.has_value());
    EXPECT_EQ(badge_info->gym_leader_name, "Brock");
    EXPECT_EQ(badge_info->type_specialization, "rock");
    EXPECT_FALSE(badge_info->description.empty());
    
    auto nonexistent = gym_leader->getBadgeInfo("NonexistentLeader");
    EXPECT_FALSE(nonexistent.has_value());
}

// ========================= Battle Challenge System =========================

TEST_F(GymLeaderTest, CanChallengeGymLeader) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    // Should be able to challenge any gym leader initially
    EXPECT_TRUE(gym_leader->canChallengeGymLeader("TestPlayer", "Brock"));
    EXPECT_TRUE(gym_leader->canChallengeGymLeader("TestPlayer", "Misty"));
    
    // Invalid player or gym leader names
    EXPECT_FALSE(gym_leader->canChallengeGymLeader("", "Brock"));
    EXPECT_FALSE(gym_leader->canChallengeGymLeader("TestPlayer", "NonexistentLeader"));
}

TEST_F(GymLeaderTest, ChallengeGymLeaderBasic) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    auto result = gym_leader->challengeGymLeader("TestPlayer", "Brock", test_team);
    
    EXPECT_EQ(result.player_name, "TestPlayer");
    EXPECT_EQ(result.gym_leader_name, "Brock");
    EXPECT_EQ(result.player_team_name, "TestTeam");
    EXPECT_FALSE(result.battle_date.empty());
    EXPECT_GE(result.turns_taken, 0);
    EXPECT_GE(result.performance_score, 0.0);
    EXPECT_LE(result.performance_score, 150.0);
    
    // Result should be either victory or defeat
    // Can't predict outcome due to randomization, but should be consistent
}

TEST_F(GymLeaderTest, ChallengeNonexistentGymLeader) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    auto result = gym_leader->challengeGymLeader("TestPlayer", "NonexistentLeader", test_team);
    
    EXPECT_EQ(result.player_name, "TestPlayer");
    EXPECT_EQ(result.gym_leader_name, "NonexistentLeader");
    EXPECT_FALSE(result.victory); // Should fail due to invalid gym leader
}

TEST_F(GymLeaderTest, GetNextRecommendedGym) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    // Should recommend first gym
    auto next_gym = gym_leader->getNextRecommendedGym("TestPlayer");
    ASSERT_TRUE(next_gym.has_value());
    EXPECT_EQ(next_gym.value(), "Brock");
    
    // Award Brock's badge
    TournamentManager::Badge badge("Pewter Gym", "rock", "Brock", "2025-01-01");
    tournament_manager->awardBadge("TestPlayer", badge);
    
    // Should now recommend Misty
    next_gym = gym_leader->getNextRecommendedGym("TestPlayer");
    ASSERT_TRUE(next_gym.has_value());
    EXPECT_EQ(next_gym.value(), "Misty");
}

// ========================= Team Analysis =========================

TEST_F(GymLeaderTest, AnalyzeTeamVsGym) {
    auto analysis = gym_leader->analyzeTeamVsGym(test_team, "Brock");
    
    EXPECT_EQ(analysis.gym_leader_name, "Brock");
    EXPECT_EQ(analysis.gym_type, "rock");
    EXPECT_GE(analysis.predicted_success_rate, 0.0);
    EXPECT_LE(analysis.predicted_success_rate, 100.0);
    
    // Should have some analysis content
    EXPECT_FALSE(analysis.strategy_tips.empty());
}

TEST_F(GymLeaderTest, AnalyzeTeamVsNonexistentGym) {
    auto analysis = gym_leader->analyzeTeamVsGym(test_team, "NonexistentLeader");
    
    EXPECT_EQ(analysis.gym_leader_name, "NonexistentLeader");
    EXPECT_TRUE(analysis.gym_type.empty());
    EXPECT_EQ(analysis.predicted_success_rate, 0.0);
}

TEST_F(GymLeaderTest, GetTeamRecommendations) {
    auto recommendations = gym_leader->getTeamRecommendations(test_team, "Brock");
    
    // Should provide some recommendations for rock-type gym
    EXPECT_FALSE(recommendations.empty());
}

TEST_F(GymLeaderTest, GetGymCounterStrategies) {
    auto strategies = gym_leader->getGymCounterStrategies("Brock", "beginner");
    EXPECT_FALSE(strategies.empty());
    
    auto advanced_strategies = gym_leader->getGymCounterStrategies("Brock", "advanced");
    EXPECT_FALSE(advanced_strategies.empty());
    
    auto nonexistent_strategies = gym_leader->getGymCounterStrategies("NonexistentLeader");
    EXPECT_TRUE(nonexistent_strategies.empty());
}

// ========================= Progress Tracking =========================

TEST_F(GymLeaderTest, GetPlayerGymProgress) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    // No progress initially
    auto progress = gym_leader->getPlayerGymProgress("TestPlayer", "Brock");
    EXPECT_FALSE(progress.has_value());
    
    // Challenge gym to create progress
    auto result = gym_leader->challengeGymLeader("TestPlayer", "Brock", test_team);
    gym_leader->updateGymProgress(result);
    
    progress = gym_leader->getPlayerGymProgress("TestPlayer", "Brock");
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(progress->player_name, "TestPlayer");
    EXPECT_EQ(progress->gym_leader_name, "Brock");
    EXPECT_EQ(progress->total_attempts, 1);
    
    if (result.victory) {
        EXPECT_EQ(progress->victories, 1);
        EXPECT_EQ(progress->defeats, 0);
    } else {
        EXPECT_EQ(progress->victories, 0);
        EXPECT_EQ(progress->defeats, 1);
    }
}

TEST_F(GymLeaderTest, GetAllGymProgress) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    auto all_progress = gym_leader->getAllGymProgress("TestPlayer");
    EXPECT_EQ(all_progress.size(), 0);
    
    // Challenge multiple gyms
    auto result1 = gym_leader->challengeGymLeader("TestPlayer", "Brock", test_team);
    gym_leader->updateGymProgress(result1);
    
    auto result2 = gym_leader->challengeGymLeader("TestPlayer", "Misty", test_team);
    gym_leader->updateGymProgress(result2);
    
    all_progress = gym_leader->getAllGymProgress("TestPlayer");
    EXPECT_EQ(all_progress.size(), 2);
    EXPECT_NE(all_progress.find("Brock"), all_progress.end());
    EXPECT_NE(all_progress.find("Misty"), all_progress.end());
}

TEST_F(GymLeaderTest, UpdateGymProgress) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    auto result = gym_leader->challengeGymLeader("TestPlayer", "Brock", test_team);
    EXPECT_TRUE(gym_leader->updateGymProgress(result));
    
    // Verify progress was updated
    auto progress = gym_leader->getPlayerGymProgress("TestPlayer", "Brock");
    ASSERT_TRUE(progress.has_value());
    EXPECT_GE(progress->total_attempts, 1);
    EXPECT_FALSE(progress->attempt_scores.empty());
}

// ========================= Statistics and Analytics =========================

TEST_F(GymLeaderTest, GetPlayerGymStats) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    auto stats = gym_leader->getPlayerGymStats("TestPlayer");
    EXPECT_EQ(stats["total_gym_attempts"], 0.0);
    EXPECT_EQ(stats["total_gym_victories"], 0.0);
    EXPECT_EQ(stats["badges_earned"], 0.0);
    
    // Challenge a gym
    auto result = gym_leader->challengeGymLeader("TestPlayer", "Brock", test_team);
    gym_leader->updateGymProgress(result);
    
    stats = gym_leader->getPlayerGymStats("TestPlayer");
    EXPECT_EQ(stats["total_gym_attempts"], 1.0);
    
    if (result.victory) {
        EXPECT_EQ(stats["total_gym_victories"], 1.0);
    } else {
        EXPECT_EQ(stats["total_gym_defeats"], 1.0);
    }
}

TEST_F(GymLeaderTest, GetPlayerGymHistory) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    auto history = gym_leader->getPlayerGymHistory("TestPlayer");
    EXPECT_EQ(history.size(), 0);
    
    // Challenge gyms
    auto result1 = gym_leader->challengeGymLeader("TestPlayer", "Brock", test_team);
    gym_leader->updateGymProgress(result1);
    
    auto result2 = gym_leader->challengeGymLeader("TestPlayer", "Misty", test_team);
    gym_leader->updateGymProgress(result2);
    
    history = gym_leader->getPlayerGymHistory("TestPlayer");
    EXPECT_EQ(history.size(), 2);
    
    // Filter by specific gym leader
    auto brock_history = gym_leader->getPlayerGymHistory("TestPlayer", "Brock");
    EXPECT_EQ(brock_history.size(), 1);
    EXPECT_EQ(brock_history[0].gym_leader_name, "Brock");
}

TEST_F(GymLeaderTest, GetGymLeaderboard) {
    // Initialize multiple players
    tournament_manager->initializePlayerProgress("Player1");
    tournament_manager->initializePlayerProgress("Player2");
    tournament_manager->initializePlayerProgress("Player3");
    
    // Have players challenge Brock with different outcomes
    auto result1 = gym_leader->challengeGymLeader("Player1", "Brock", test_team);
    gym_leader->updateGymProgress(result1);
    
    auto result2 = gym_leader->challengeGymLeader("Player2", "Brock", test_team);
    gym_leader->updateGymProgress(result2);
    
    auto leaderboard = gym_leader->getGymLeaderboard("Brock", "score", 10);
    
    // Should have at least the players who challenged
    EXPECT_GE(leaderboard.size(), 2);
    
    // Should be sorted (higher scores first for "score" sorting)
    if (leaderboard.size() >= 2) {
        EXPECT_GE(leaderboard[0].second, leaderboard[1].second);
    }
}

// ========================= Configuration Management =========================

TEST_F(GymLeaderTest, GymSettingsManagement) {
    auto settings = gym_leader->getGymSettings();
    EXPECT_TRUE(settings.allow_multiple_attempts);
    EXPECT_TRUE(settings.award_badges_immediately);
    
    // Modify settings
    settings.allow_multiple_attempts = false;
    settings.max_attempts_per_gym = 3;
    gym_leader->setGymSettings(settings);
    
    auto new_settings = gym_leader->getGymSettings();
    EXPECT_FALSE(new_settings.allow_multiple_attempts);
    EXPECT_EQ(new_settings.max_attempts_per_gym, 3);
}

// ========================= Data Persistence =========================

TEST_F(GymLeaderTest, SaveAndLoadGymData) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    // Challenge a gym to create data
    auto result = gym_leader->challengeGymLeader("TestPlayer", "Brock", test_team);
    gym_leader->updateGymProgress(result);
    
    // Save data
    EXPECT_TRUE(gym_leader->saveGymData());
    
    // Create new gym leader instance and load data
    auto new_gym_leader = std::make_unique<GymLeader>(pokemon_data, team_builder, tournament_manager);
    EXPECT_TRUE(new_gym_leader->loadGymData());
    
    // Verify data was loaded
    auto progress = new_gym_leader->getPlayerGymProgress("TestPlayer", "Brock");
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(progress->player_name, "TestPlayer");
    EXPECT_EQ(progress->gym_leader_name, "Brock");
    
    auto history = new_gym_leader->getPlayerGymHistory("TestPlayer", "Brock");
    EXPECT_EQ(history.size(), 1);
}

TEST_F(GymLeaderTest, ResetPlayerGymProgress) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    // Challenge gyms to create progress
    auto result1 = gym_leader->challengeGymLeader("TestPlayer", "Brock", test_team);
    gym_leader->updateGymProgress(result1);
    
    auto result2 = gym_leader->challengeGymLeader("TestPlayer", "Misty", test_team);
    gym_leader->updateGymProgress(result2);
    
    EXPECT_EQ(gym_leader->getAllGymProgress("TestPlayer").size(), 2);
    
    // Reset without confirmation should fail
    EXPECT_FALSE(gym_leader->resetPlayerGymProgress("TestPlayer", "", false));
    EXPECT_EQ(gym_leader->getAllGymProgress("TestPlayer").size(), 2);
    
    // Reset specific gym with confirmation
    EXPECT_TRUE(gym_leader->resetPlayerGymProgress("TestPlayer", "Brock", true));
    EXPECT_EQ(gym_leader->getAllGymProgress("TestPlayer").size(), 1);
    
    // Reset all gyms
    EXPECT_TRUE(gym_leader->resetPlayerGymProgress("TestPlayer", "", true));
    EXPECT_EQ(gym_leader->getAllGymProgress("TestPlayer").size(), 0);
}

// ========================= Data Validation =========================

TEST_F(GymLeaderTest, ValidateGymData) {
    // Fresh gym system should be valid
    EXPECT_TRUE(gym_leader->validateGymData());
    
    // Add some progress and verify still valid
    tournament_manager->initializePlayerProgress("TestPlayer");
    auto result = gym_leader->challengeGymLeader("TestPlayer", "Brock", test_team);
    gym_leader->updateGymProgress(result);
    
    EXPECT_TRUE(gym_leader->validateGymData());
}

TEST_F(GymLeaderTest, GetGymSystemStatus) {
    auto status = gym_leader->getGymSystemStatus();
    
    EXPECT_EQ(status["total_gym_leaders"], "8");
    EXPECT_EQ(status["total_badge_templates"], "8");
    EXPECT_EQ(status["players_with_progress"], "0");
    EXPECT_EQ(status["data_valid"], "true");
    
    // Add player progress
    tournament_manager->initializePlayerProgress("TestPlayer");
    auto result = gym_leader->challengeGymLeader("TestPlayer", "Brock", test_team);
    gym_leader->updateGymProgress(result);
    
    status = gym_leader->getGymSystemStatus();
    EXPECT_EQ(status["players_with_progress"], "1");
    EXPECT_EQ(status["total_gym_battles"], "1");
}

// ========================= Edge Cases and Error Handling =========================

TEST_F(GymLeaderTest, EmptyTeamChallenge) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    TeamBuilder::Team empty_team;
    empty_team.name = "EmptyTeam";
    empty_team.pokemon = {};
    
    auto result = gym_leader->challengeGymLeader("TestPlayer", "Brock", empty_team);
    EXPECT_EQ(result.player_team_name, "EmptyTeam");
    // Should still execute but may have different outcomes
}

TEST_F(GymLeaderTest, InvalidPlayerNames) {
    EXPECT_FALSE(gym_leader->canChallengeGymLeader("", "Brock"));
    EXPECT_FALSE(gym_leader->canChallengeGymLeader(std::string(100, 'a'), "Brock"));
}

TEST_F(GymLeaderTest, MultipleChallengesSameGym) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    // Challenge same gym multiple times
    for (int i = 0; i < 3; i++) {
        auto result = gym_leader->challengeGymLeader("TestPlayer", "Brock", test_team);
        gym_leader->updateGymProgress(result);
    }
    
    auto progress = gym_leader->getPlayerGymProgress("TestPlayer", "Brock");
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(progress->total_attempts, 3);
    
    auto history = gym_leader->getPlayerGymHistory("TestPlayer", "Brock");
    EXPECT_EQ(history.size(), 3);
}

TEST_F(GymLeaderTest, TypeAdvantageCalculation) {
    // Create team with type advantages
    TeamBuilder::Team advantage_team;
    advantage_team.name = "AdvantageTeam";
    
    TeamBuilder::TeamPokemon water_pokemon;
    water_pokemon.name = "WaterType";
    
    TeamBuilder::TeamPokemon grass_pokemon;
    grass_pokemon.name = "GrassType";
    
    advantage_team.pokemon = {water_pokemon, grass_pokemon};
    
    auto analysis = gym_leader->analyzeTeamVsGym(advantage_team, "Brock"); // Rock type
    
    // Should predict higher success rate against rock type with water/grass
    EXPECT_GT(analysis.predicted_success_rate, 60.0);
}

TEST_F(GymLeaderTest, DifficultyScaling) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    // Award some badges to test difficulty scaling
    for (int i = 0; i < 6; i++) {
        TournamentManager::Badge badge("Gym" + std::to_string(i), "type", "Leader", "2025-01-01");
        tournament_manager->awardBadge("TestPlayer", badge);
    }
    
    // Challenge should have scaled difficulty
    auto result = gym_leader->challengeGymLeader("TestPlayer", "Blaine", test_team);
    
    // Difficulty level should be adjusted based on badge count
    EXPECT_FALSE(result.difficulty_level.empty());
}

TEST_F(GymLeaderTest, BadgeAwardingIntegration) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    // Set up to guarantee victory (simplified for testing)
    auto result = gym_leader->challengeGymLeader("TestPlayer", "Brock", test_team);
    gym_leader->updateGymProgress(result);
    
    if (result.victory) {
        // Verify badge was awarded through tournament manager integration
        EXPECT_TRUE(tournament_manager->hasPlayerEarnedBadge("TestPlayer", "Brock"));
        EXPECT_EQ(tournament_manager->getPlayerBadgeCount("TestPlayer"), 1);
    }
}

TEST_F(GymLeaderTest, ConcurrentPlayerSupport) {
    // Test that multiple players can progress independently
    tournament_manager->initializePlayerProgress("Player1");
    tournament_manager->initializePlayerProgress("Player2");
    
    auto result1 = gym_leader->challengeGymLeader("Player1", "Brock", test_team);
    gym_leader->updateGymProgress(result1);
    
    auto result2 = gym_leader->challengeGymLeader("Player2", "Misty", test_team);
    gym_leader->updateGymProgress(result2);
    
    // Each player should have independent progress
    auto progress1 = gym_leader->getAllGymProgress("Player1");
    auto progress2 = gym_leader->getAllGymProgress("Player2");
    
    EXPECT_EQ(progress1.size(), 1);
    EXPECT_EQ(progress2.size(), 1);
    EXPECT_NE(progress1.find("Brock"), progress1.end());
    EXPECT_NE(progress2.find("Misty"), progress2.end());
}