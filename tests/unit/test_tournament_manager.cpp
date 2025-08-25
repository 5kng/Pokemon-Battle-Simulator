#include <gtest/gtest.h>
#include "test_utils.h"
#include "tournament_manager.h"
#include "pokemon_data.h"
#include "team_builder.h"
#include <filesystem>
#include <fstream>

class TournamentManagerTest : public TestUtils::PokemonTestFixture {
protected:
    void SetUp() override {
        TestUtils::PokemonTestFixture::SetUp();
        
        // Create mock dependencies
        pokemon_data = std::make_shared<PokemonData>();
        team_builder = std::make_shared<TeamBuilder>(pokemon_data);
        
        // Initialize tournament manager
        tournament_manager = std::make_unique<TournamentManager>(pokemon_data, team_builder);
        
        // Clean up any existing test data
        cleanup_test_data();
    }
    
    void TearDown() override {
        cleanup_test_data();
    }
    
    void cleanup_test_data() {
        // Clean up test tournament data files
        std::filesystem::remove_all("data/tournaments");
    }
    
    std::shared_ptr<PokemonData> pokemon_data;
    std::shared_ptr<TeamBuilder> team_builder;
    std::unique_ptr<TournamentManager> tournament_manager;
};

// ========================= Constructor and Basic Setup =========================

TEST_F(TournamentManagerTest, ConstructorWithValidDependencies) {
    EXPECT_NO_THROW({
        auto tm = std::make_unique<TournamentManager>(pokemon_data, team_builder);
    });
}

TEST_F(TournamentManagerTest, ConstructorWithNullDependencies) {
    EXPECT_THROW({
        auto tm = std::make_unique<TournamentManager>(nullptr, team_builder);
    }, std::invalid_argument);
    
    EXPECT_THROW({
        auto tm = std::make_unique<TournamentManager>(pokemon_data, nullptr);
    }, std::invalid_argument);
    
    EXPECT_THROW({
        auto tm = std::make_unique<TournamentManager>(nullptr, nullptr);
    }, std::invalid_argument);
}

// ========================= Player Progress Management =========================

TEST_F(TournamentManagerTest, InitializePlayerProgress) {
    EXPECT_TRUE(tournament_manager->initializePlayerProgress("TestPlayer"));
    
    // Check that progress was created
    auto progress = tournament_manager->getPlayerProgress("TestPlayer");
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(progress->player_name, "TestPlayer");
    EXPECT_EQ(progress->earned_badges.size(), 0);
    EXPECT_FALSE(progress->elite_four_unlocked);
    EXPECT_FALSE(progress->champion_defeated);
}

TEST_F(TournamentManagerTest, InitializePlayerProgressWithInvalidNames) {
    EXPECT_FALSE(tournament_manager->initializePlayerProgress(""));
    EXPECT_FALSE(tournament_manager->initializePlayerProgress(std::string(100, 'a'))); // Too long
}

TEST_F(TournamentManagerTest, InitializeExistingPlayer) {
    EXPECT_TRUE(tournament_manager->initializePlayerProgress("TestPlayer"));
    EXPECT_TRUE(tournament_manager->initializePlayerProgress("TestPlayer")); // Should not fail
    
    // Should only have one entry
    auto players = tournament_manager->getAllPlayers();
    EXPECT_EQ(std::count(players.begin(), players.end(), "TestPlayer"), 1);
}

TEST_F(TournamentManagerTest, GetNonexistentPlayerProgress) {
    auto progress = tournament_manager->getPlayerProgress("NonexistentPlayer");
    EXPECT_FALSE(progress.has_value());
}

// ========================= Badge Management =========================

TEST_F(TournamentManagerTest, UpdatePlayerProgressWithGymVictory) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    TournamentManager::TournamentBattleResult battle_result;
    battle_result.challenge_type = "gym";
    battle_result.challenge_name = "Pewter City Gym";
    battle_result.opponent_name = "Brock";
    battle_result.player_team_name = "TestTeam";
    battle_result.victory = true;
    battle_result.performance_score = 85.0;
    battle_result.turns_taken = 15;
    battle_result.difficulty_level = "Medium";
    battle_result.battle_date = "2025-01-01 12:00:00";
    
    EXPECT_TRUE(tournament_manager->updatePlayerProgress("TestPlayer", battle_result));
    
    // Check that badge was awarded
    auto progress = tournament_manager->getPlayerProgress("TestPlayer");
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(progress->earned_badges.size(), 1);
    EXPECT_EQ(progress->earned_badges[0].gym_leader_name, "Brock");
    EXPECT_EQ(progress->earned_badges[0].gym_type, "rock");
    EXPECT_EQ(progress->total_gym_attempts, 1);
}

TEST_F(TournamentManagerTest, UpdatePlayerProgressWithGymDefeat) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    TournamentManager::TournamentBattleResult battle_result;
    battle_result.challenge_type = "gym";
    battle_result.challenge_name = "Pewter City Gym";
    battle_result.opponent_name = "Brock";
    battle_result.player_team_name = "TestTeam";
    battle_result.victory = false;
    battle_result.performance_score = 45.0;
    battle_result.turns_taken = 20;
    battle_result.difficulty_level = "Easy";
    battle_result.battle_date = "2025-01-01 12:00:00";
    
    EXPECT_TRUE(tournament_manager->updatePlayerProgress("TestPlayer", battle_result));
    
    // Check that no badge was awarded but attempt was recorded
    auto progress = tournament_manager->getPlayerProgress("TestPlayer");
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(progress->earned_badges.size(), 0);
    EXPECT_EQ(progress->total_gym_attempts, 1);
}

TEST_F(TournamentManagerTest, AwardBadge) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    TournamentManager::Badge badge("Pewter Gym", "rock", "Brock", "2025-01-01", 1, 90.0);
    EXPECT_TRUE(tournament_manager->awardBadge("TestPlayer", badge));
    
    auto badges = tournament_manager->getPlayerBadges("TestPlayer");
    EXPECT_EQ(badges.size(), 1);
    EXPECT_EQ(badges[0].gym_name, "Pewter Gym");
    EXPECT_EQ(badges[0].gym_type, "rock");
    EXPECT_EQ(badges[0].final_battle_score, 90.0);
}

TEST_F(TournamentManagerTest, HasPlayerEarnedBadge) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    EXPECT_FALSE(tournament_manager->hasPlayerEarnedBadge("TestPlayer", "Brock"));
    
    TournamentManager::Badge badge("Pewter Gym", "rock", "Brock", "2025-01-01");
    tournament_manager->awardBadge("TestPlayer", badge);
    
    EXPECT_TRUE(tournament_manager->hasPlayerEarnedBadge("TestPlayer", "Brock"));
}

TEST_F(TournamentManagerTest, GetPlayerBadgeCount) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    EXPECT_EQ(tournament_manager->getPlayerBadgeCount("TestPlayer"), 0);
    
    // Award multiple badges
    for (int i = 1; i <= 3; i++) {
        TournamentManager::Badge badge("Gym" + std::to_string(i), "type", "Leader" + std::to_string(i), "2025-01-01");
        tournament_manager->awardBadge("TestPlayer", badge);
    }
    
    EXPECT_EQ(tournament_manager->getPlayerBadgeCount("TestPlayer"), 3);
}

// ========================= Elite Four and Champion Progress =========================

TEST_F(TournamentManagerTest, EliteFourUnlocked) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    // Should not be unlocked initially
    EXPECT_FALSE(tournament_manager->isEliteFourUnlocked("TestPlayer"));
    
    // Award all 8 badges
    std::vector<std::string> leaders = {"Brock", "Misty", "Lt. Surge", "Erika", "Koga", "Sabrina", "Blaine", "Giovanni"};
    std::vector<std::string> types = {"rock", "water", "electric", "grass", "poison", "psychic", "fire", "ground"};
    
    for (size_t i = 0; i < leaders.size(); ++i) {
        TournamentManager::Badge badge("Gym", types[i], leaders[i], "2025-01-01");
        tournament_manager->awardBadge("TestPlayer", badge);
    }
    
    EXPECT_TRUE(tournament_manager->isEliteFourUnlocked("TestPlayer"));
}

TEST_F(TournamentManagerTest, UpdatePlayerProgressWithEliteFourVictory) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    // Award 8 badges first to unlock Elite Four
    std::vector<std::string> leaders = {"Brock", "Misty", "Lt. Surge", "Erika", "Koga", "Sabrina", "Blaine", "Giovanni"};
    std::vector<std::string> types = {"rock", "water", "electric", "grass", "poison", "psychic", "fire", "ground"};
    
    for (size_t i = 0; i < leaders.size(); ++i) {
        TournamentManager::Badge badge("Gym", types[i], leaders[i], "2025-01-01");
        tournament_manager->awardBadge("TestPlayer", badge);
    }
    
    TournamentManager::TournamentBattleResult battle_result;
    battle_result.challenge_type = "elite_four";
    battle_result.opponent_name = "Lorelei";
    battle_result.victory = true;
    battle_result.performance_score = 90.0;
    
    EXPECT_TRUE(tournament_manager->updatePlayerProgress("TestPlayer", battle_result));
    
    auto progress = tournament_manager->getPlayerProgress("TestPlayer");
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(progress->defeated_elite_four.size(), 1);
    EXPECT_EQ(progress->defeated_elite_four[0], "Lorelei");
    EXPECT_EQ(progress->total_elite_four_attempts, 1);
}

TEST_F(TournamentManagerTest, ChampionshipUnlocked) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    // Should not be unlocked initially
    EXPECT_FALSE(tournament_manager->isChampionshipUnlocked("TestPlayer"));
    
    // Award badges and complete Elite Four
    std::vector<std::string> leaders = {"Brock", "Misty", "Lt. Surge", "Erika", "Koga", "Sabrina", "Blaine", "Giovanni"};
    std::vector<std::string> types = {"rock", "water", "electric", "grass", "poison", "psychic", "fire", "ground"};
    
    for (size_t i = 0; i < leaders.size(); ++i) {
        TournamentManager::Badge badge("Gym", types[i], leaders[i], "2025-01-01");
        tournament_manager->awardBadge("TestPlayer", badge);
    }
    
    std::vector<std::string> elite_four = {"Lorelei", "Bruno", "Agatha", "Lance"};
    for (const auto& member : elite_four) {
        TournamentManager::TournamentBattleResult battle_result;
        battle_result.challenge_type = "elite_four";
        battle_result.opponent_name = member;
        battle_result.victory = true;
        tournament_manager->updatePlayerProgress("TestPlayer", battle_result);
    }
    
    EXPECT_TRUE(tournament_manager->isChampionshipUnlocked("TestPlayer"));
}

TEST_F(TournamentManagerTest, UpdatePlayerProgressWithChampionVictory) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    // Complete prerequisites (badges and Elite Four)
    std::vector<std::string> leaders = {"Brock", "Misty", "Lt. Surge", "Erika", "Koga", "Sabrina", "Blaine", "Giovanni"};
    std::vector<std::string> types = {"rock", "water", "electric", "grass", "poison", "psychic", "fire", "ground"};
    
    for (size_t i = 0; i < leaders.size(); ++i) {
        TournamentManager::Badge badge("Gym", types[i], leaders[i], "2025-01-01");
        tournament_manager->awardBadge("TestPlayer", badge);
    }
    
    std::vector<std::string> elite_four = {"Lorelei", "Bruno", "Agatha", "Lance"};
    for (const auto& member : elite_four) {
        TournamentManager::TournamentBattleResult battle_result;
        battle_result.challenge_type = "elite_four";
        battle_result.opponent_name = member;
        battle_result.victory = true;
        tournament_manager->updatePlayerProgress("TestPlayer", battle_result);
    }
    
    TournamentManager::TournamentBattleResult champion_battle;
    champion_battle.challenge_type = "champion";
    champion_battle.opponent_name = "Champion";
    champion_battle.victory = true;
    champion_battle.performance_score = 95.0;
    
    EXPECT_TRUE(tournament_manager->updatePlayerProgress("TestPlayer", champion_battle));
    
    auto progress = tournament_manager->getPlayerProgress("TestPlayer");
    ASSERT_TRUE(progress.has_value());
    EXPECT_TRUE(progress->champion_defeated);
    EXPECT_EQ(progress->total_champion_attempts, 1);
}

// ========================= Challenge Management =========================

TEST_F(TournamentManagerTest, GetAvailableChallenges) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    auto challenges = tournament_manager->getAvailableChallenges("TestPlayer");
    
    // Should have 8 gym challenges initially
    int gym_challenges = 0;
    for (const auto& challenge : challenges) {
        if (challenge.challenge_type == "gym") {
            gym_challenges++;
            EXPECT_TRUE(challenge.is_unlocked);
            EXPECT_FALSE(challenge.is_completed);
        }
    }
    EXPECT_EQ(gym_challenges, 8);
}

TEST_F(TournamentManagerTest, IsChallengeUnlocked) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    EXPECT_TRUE(tournament_manager->isChallengeUnlocked("TestPlayer", "Pewter City Gym"));
    EXPECT_FALSE(tournament_manager->isChallengeUnlocked("TestPlayer", "Lorelei of the Elite Four"));
}

TEST_F(TournamentManagerTest, GetNextRecommendedChallenge) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    auto next_challenge = tournament_manager->getNextRecommendedChallenge("TestPlayer");
    ASSERT_TRUE(next_challenge.has_value());
    EXPECT_EQ(next_challenge->challenge_type, "gym");
    
    // Complete all gyms
    std::vector<std::string> leaders = {"Brock", "Misty", "Lt. Surge", "Erika", "Koga", "Sabrina", "Blaine", "Giovanni"};
    std::vector<std::string> types = {"rock", "water", "electric", "grass", "poison", "psychic", "fire", "ground"};
    
    for (size_t i = 0; i < leaders.size(); ++i) {
        TournamentManager::Badge badge("Gym", types[i], leaders[i], "2025-01-01");
        tournament_manager->awardBadge("TestPlayer", badge);
    }
    
    next_challenge = tournament_manager->getNextRecommendedChallenge("TestPlayer");
    ASSERT_TRUE(next_challenge.has_value());
    EXPECT_EQ(next_challenge->challenge_type, "elite_four");
}

// ========================= Tournament Completion and Statistics =========================

TEST_F(TournamentManagerTest, GetTournamentCompletionPercentage) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    EXPECT_FLOAT_EQ_EPSILON(tournament_manager->getTournamentCompletionPercentage("TestPlayer"), 0.0, 0.01);
    
    // Complete 4 gyms (50% of gyms, ~30.8% of total)
    for (int i = 0; i < 4; i++) {
        TournamentManager::Badge badge("Gym" + std::to_string(i), "type", "Leader", "2025-01-01");
        tournament_manager->awardBadge("TestPlayer", badge);
    }
    
    double completion = tournament_manager->getTournamentCompletionPercentage("TestPlayer");
    EXPECT_GT(completion, 0.3);
    EXPECT_LT(completion, 0.4);
}

TEST_F(TournamentManagerTest, GetPlayerTournamentStats) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    auto stats = tournament_manager->getPlayerTournamentStats("TestPlayer");
    
    EXPECT_EQ(stats["badges_earned"], 0.0);
    EXPECT_EQ(stats["elite_four_defeated"], 0.0);
    EXPECT_EQ(stats["champion_defeated"], 0.0);
    EXPECT_EQ(stats["total_gym_attempts"], 0.0);
    
    // Add some progress
    TournamentManager::Badge badge("Pewter Gym", "rock", "Brock", "2025-01-01");
    tournament_manager->awardBadge("TestPlayer", badge);
    
    stats = tournament_manager->getPlayerTournamentStats("TestPlayer");
    EXPECT_EQ(stats["badges_earned"], 1.0);
}

TEST_F(TournamentManagerTest, GetTournamentLeaderboard) {
    // Initialize multiple players with different progress
    tournament_manager->initializePlayerProgress("Player1");
    tournament_manager->initializePlayerProgress("Player2");
    tournament_manager->initializePlayerProgress("Player3");
    
    // Give Player1 more badges
    for (int i = 0; i < 3; i++) {
        TournamentManager::Badge badge("Gym" + std::to_string(i), "type", "Leader", "2025-01-01");
        tournament_manager->awardBadge("Player1", badge);
    }
    
    // Give Player2 fewer badges
    TournamentManager::Badge badge("Gym0", "type", "Leader", "2025-01-01");
    tournament_manager->awardBadge("Player2", badge);
    
    auto leaderboard = tournament_manager->getTournamentLeaderboard("badges", 10);
    
    EXPECT_GE(leaderboard.size(), 2);
    // Player1 should be ahead of Player2
    bool found_player1 = false, found_player2 = false;
    int player1_pos = -1, player2_pos = -1;
    
    for (size_t i = 0; i < leaderboard.size(); ++i) {
        if (leaderboard[i].first == "Player1") {
            found_player1 = true;
            player1_pos = static_cast<int>(i);
        }
        if (leaderboard[i].first == "Player2") {
            found_player2 = true;
            player2_pos = static_cast<int>(i);
        }
    }
    
    EXPECT_TRUE(found_player1);
    EXPECT_TRUE(found_player2);
    EXPECT_LT(player1_pos, player2_pos); // Player1 should be ranked higher (lower index)
}

// ========================= Battle History =========================

TEST_F(TournamentManagerTest, GetPlayerBattleHistory) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    auto history = tournament_manager->getPlayerBattleHistory("TestPlayer");
    EXPECT_EQ(history.size(), 0);
    
    // Add battle results
    TournamentManager::TournamentBattleResult battle1;
    battle1.challenge_type = "gym";
    battle1.opponent_name = "Brock";
    battle1.victory = true;
    tournament_manager->updatePlayerProgress("TestPlayer", battle1);
    
    TournamentManager::TournamentBattleResult battle2;
    battle2.challenge_type = "gym";
    battle2.opponent_name = "Misty";
    battle2.victory = false;
    tournament_manager->updatePlayerProgress("TestPlayer", battle2);
    
    history = tournament_manager->getPlayerBattleHistory("TestPlayer");
    EXPECT_EQ(history.size(), 2);
    
    // Filter by challenge type
    auto gym_history = tournament_manager->getPlayerBattleHistory("TestPlayer", "gym");
    EXPECT_EQ(gym_history.size(), 2);
    
    auto elite_history = tournament_manager->getPlayerBattleHistory("TestPlayer", "elite_four");
    EXPECT_EQ(elite_history.size(), 0);
}

// ========================= Data Persistence =========================

TEST_F(TournamentManagerTest, SaveAndLoadTournamentProgress) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    // Create directories first
    std::filesystem::create_directories("data/tournaments");
    
    // Add some progress
    TournamentManager::Badge badge("Pewter Gym", "rock", "Brock", "2025-01-01");
    tournament_manager->awardBadge("TestPlayer", badge);
    
    TournamentManager::TournamentBattleResult battle_result;
    battle_result.challenge_type = "gym";
    battle_result.challenge_name = "Pewter City Gym";
    battle_result.opponent_name = "Brock";
    battle_result.player_team_name = "TestTeam";
    battle_result.victory = true;
    battle_result.performance_score = 85.0;
    battle_result.turns_taken = 15;
    battle_result.difficulty_level = "Easy";
    battle_result.battle_date = "2025-01-01 12:00:00";
    tournament_manager->updatePlayerProgress("TestPlayer", battle_result);
    
    // Save the progress
    EXPECT_TRUE(tournament_manager->saveTournamentProgress());
    
    // Create new tournament manager and load the data
    auto new_tournament_manager = std::make_unique<TournamentManager>(pokemon_data, team_builder);
    EXPECT_TRUE(new_tournament_manager->loadTournamentProgress());
    
    // Verify data was loaded
    auto progress = new_tournament_manager->getPlayerProgress("TestPlayer");
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(progress->earned_badges.size(), 1);
    EXPECT_EQ(progress->earned_badges[0].gym_leader_name, "Brock");
    
    auto history = new_tournament_manager->getPlayerBattleHistory("TestPlayer");
    EXPECT_EQ(history.size(), 1);
}

TEST_F(TournamentManagerTest, SaveSpecificPlayer) {
    tournament_manager->initializePlayerProgress("Player1");
    tournament_manager->initializePlayerProgress("Player2");
    
    TournamentManager::Badge badge1("Gym1", "rock", "Leader1", "2025-01-01");
    TournamentManager::Badge badge2("Gym2", "water", "Leader2", "2025-01-01");
    tournament_manager->awardBadge("Player1", badge1);
    tournament_manager->awardBadge("Player2", badge2);
    
    // Save only Player1
    EXPECT_TRUE(tournament_manager->saveTournamentProgress("Player1"));
    
    // Create new manager and load
    auto new_tournament_manager = std::make_unique<TournamentManager>(pokemon_data, team_builder);
    EXPECT_TRUE(new_tournament_manager->loadTournamentProgress());
    
    // Should have loaded all players (since load doesn't filter by specific player in the current implementation)
    auto players = new_tournament_manager->getAllPlayers();
    EXPECT_GE(players.size(), 1);
}

// ========================= Player Management =========================

TEST_F(TournamentManagerTest, ResetPlayerProgress) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    // Add some progress
    TournamentManager::Badge badge("Pewter Gym", "rock", "Brock", "2025-01-01");
    tournament_manager->awardBadge("TestPlayer", badge);
    
    EXPECT_EQ(tournament_manager->getPlayerBadgeCount("TestPlayer"), 1);
    
    // Reset without confirmation should fail
    EXPECT_FALSE(tournament_manager->resetPlayerProgress("TestPlayer", false));
    EXPECT_EQ(tournament_manager->getPlayerBadgeCount("TestPlayer"), 1);
    
    // Reset with confirmation should succeed
    EXPECT_TRUE(tournament_manager->resetPlayerProgress("TestPlayer", true));
    EXPECT_EQ(tournament_manager->getPlayerBadgeCount("TestPlayer"), 0);
}

TEST_F(TournamentManagerTest, GetAllPlayers) {
    auto players = tournament_manager->getAllPlayers();
    EXPECT_EQ(players.size(), 0);
    
    tournament_manager->initializePlayerProgress("Player1");
    tournament_manager->initializePlayerProgress("Player2");
    tournament_manager->initializePlayerProgress("Player3");
    
    players = tournament_manager->getAllPlayers();
    EXPECT_EQ(players.size(), 3);
    EXPECT_NE(std::find(players.begin(), players.end(), "Player1"), players.end());
    EXPECT_NE(std::find(players.begin(), players.end(), "Player2"), players.end());
    EXPECT_NE(std::find(players.begin(), players.end(), "Player3"), players.end());
}

// ========================= Data Validation =========================

TEST_F(TournamentManagerTest, ValidateTournamentData) {
    // Empty tournament should be valid
    EXPECT_TRUE(tournament_manager->validateTournamentData());
    
    tournament_manager->initializePlayerProgress("TestPlayer");
    EXPECT_TRUE(tournament_manager->validateTournamentData());
    
    // Add valid progress
    TournamentManager::Badge badge("Pewter Gym", "rock", "Brock", "2025-01-01");
    tournament_manager->awardBadge("TestPlayer", badge);
    
    EXPECT_TRUE(tournament_manager->validateTournamentData());
}

TEST_F(TournamentManagerTest, GetTournamentSystemStatus) {
    auto status = tournament_manager->getTournamentSystemStatus();
    
    EXPECT_EQ(status["total_players"], "0");
    EXPECT_EQ(status["data_valid"], "true");
    
    tournament_manager->initializePlayerProgress("TestPlayer");
    TournamentManager::Badge badge("Pewter Gym", "rock", "Brock", "2025-01-01");
    tournament_manager->awardBadge("TestPlayer", badge);
    
    status = tournament_manager->getTournamentSystemStatus();
    EXPECT_EQ(status["total_players"], "1");
    EXPECT_EQ(status["total_badges_earned"], "1");
}

// ========================= Edge Cases and Error Handling =========================

TEST_F(TournamentManagerTest, UpdateProgressForNonexistentPlayer) {
    TournamentManager::TournamentBattleResult battle_result;
    battle_result.challenge_type = "gym";
    battle_result.opponent_name = "Brock";
    battle_result.victory = true;
    
    // Should initialize player automatically
    EXPECT_TRUE(tournament_manager->updatePlayerProgress("NewPlayer", battle_result));
    
    auto progress = tournament_manager->getPlayerProgress("NewPlayer");
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(progress->player_name, "NewPlayer");
}

TEST_F(TournamentManagerTest, DuplicateBadgeAwarding) {
    tournament_manager->initializePlayerProgress("TestPlayer");
    
    TournamentManager::Badge badge("Pewter Gym", "rock", "Brock", "2025-01-01");
    EXPECT_TRUE(tournament_manager->awardBadge("TestPlayer", badge));
    
    // Trying to award the same badge again should not duplicate
    EXPECT_FALSE(tournament_manager->awardBadge("TestPlayer", badge));
    
    EXPECT_EQ(tournament_manager->getPlayerBadgeCount("TestPlayer"), 1);
}

TEST_F(TournamentManagerTest, PlayerNameNormalization) {
    // Test that player names are normalized correctly
    EXPECT_TRUE(tournament_manager->initializePlayerProgress("  TestPlayer  "));
    
    auto progress = tournament_manager->getPlayerProgress("TestPlayer");
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(progress->player_name, "TestPlayer");
    
    // Should find the same player with whitespace
    auto progress2 = tournament_manager->getPlayerProgress("  TestPlayer  ");
    ASSERT_TRUE(progress2.has_value());
    EXPECT_EQ(progress2->player_name, "TestPlayer");
}

TEST_F(TournamentManagerTest, TournamentSettings) {
    auto settings = tournament_manager->getTournamentSettings();
    EXPECT_TRUE(settings.require_all_badges);
    EXPECT_TRUE(settings.require_elite_four_completion);
    
    // Modify settings
    settings.require_all_badges = false;
    tournament_manager->setTournamentSettings(settings);
    
    auto new_settings = tournament_manager->getTournamentSettings();
    EXPECT_FALSE(new_settings.require_all_badges);
    
    // With modified settings, Elite Four should be unlocked immediately
    tournament_manager->initializePlayerProgress("TestPlayer");
    EXPECT_TRUE(tournament_manager->isEliteFourUnlocked("TestPlayer"));
}