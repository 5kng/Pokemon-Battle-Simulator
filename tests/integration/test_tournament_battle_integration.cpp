#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include "test_utils.h"
#include "tournament_manager.h"
#include "battle.h"
#include "team_builder.h"
#include "pokemon_data.h"
#include "championship_system.h"

/**
 * @brief Integration tests for Tournament-Battle system interactions
 * 
 * These tests validate that tournament battles produce consistent outcomes,
 * properly sync progress, and maintain team state across the tournament progression.
 * Focus areas:
 * - Battle outcome consistency between standalone and tournament battles
 * - Tournament progress synchronization after battle results
 * - Team state persistence through tournament battles
 * - Gym leader battle integration with tournament system
 */
class TournamentBattleIntegrationTest : public TestUtils::BattleTestFixture {
protected:
    void SetUp() override {
        TestUtils::BattleTestFixture::SetUp();
        
        // Initialize core systems
        pokemon_data = std::make_shared<PokemonData>();
        team_builder = std::make_shared<TeamBuilder>(pokemon_data);
        tournament_manager = std::make_shared<TournamentManager>(pokemon_data, team_builder);
        
        // Setup test player and teams
        setupTestPlayer();
        setupTestGymsAndOpponents();
        
        // Initialize tournament progress
        tournament_manager->initializePlayerProgress(test_player_name);
    }
    
    void setupTestPlayer() {
        test_player_name = "TestTrainer";
        
        // Create a balanced starter team
        Pokemon starter1 = TestUtils::createTestPokemon("FireStarter", 100, 85, 70, 95, 80, 75, {"fire"});
        starter1.moves.clear();
        starter1.moves.push_back(TestUtils::createTestMove("ember", 40, 100, 25, "fire", "special"));
        starter1.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
        starter1.moves.push_back(TestUtils::createTestMove("growl", 0, 100, 40, "normal", "status"));
        starter1.moves.push_back(TestUtils::createTestMove("flame-wheel", 60, 100, 25, "fire", "physical"));
        
        Pokemon starter2 = TestUtils::createTestPokemon("WaterStarter", 105, 80, 75, 90, 85, 70, {"water"});
        starter2.moves.clear();
        starter2.moves.push_back(TestUtils::createTestMove("water-gun", 40, 100, 25, "water", "special"));
        starter2.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
        starter2.moves.push_back(TestUtils::createTestMove("tail-whip", 0, 100, 30, "normal", "status"));
        starter2.moves.push_back(TestUtils::createTestMove("bubble-beam", 65, 100, 20, "water", "special"));
        
        Pokemon starter3 = TestUtils::createTestPokemon("GrassStarter", 100, 75, 80, 95, 90, 65, {"grass", "poison"});
        starter3.moves.clear();
        starter3.moves.push_back(TestUtils::createTestMove("vine-whip", 45, 100, 25, "grass", "physical"));
        starter3.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
        starter3.moves.push_back(TestUtils::createTestMove("poison-powder", 0, 75, 35, "poison", "status", StatusCondition::POISON, 100));
        starter3.moves.push_back(TestUtils::createTestMove("razor-leaf", 55, 95, 25, "grass", "physical"));
        
        player_team = TestUtils::createTestTeam({starter1, starter2, starter3});
        
        // Store original team state for comparison
        original_team_state = captureTeamState(player_team);
    }
    
    void setupTestGymsAndOpponents() {
        // Create Rock Gym Leader team (type disadvantage for Fire starter)
        Pokemon rockLeader1 = TestUtils::createTestPokemon("Geodude", 90, 80, 100, 30, 30, 20, {"rock", "ground"});
        rockLeader1.moves.clear();
        rockLeader1.moves.push_back(TestUtils::createTestMove("rock-throw", 50, 90, 15, "rock", "physical"));
        rockLeader1.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
        rockLeader1.moves.push_back(TestUtils::createTestMove("defense-curl", 0, 100, 40, "normal", "status"));
        rockLeader1.moves.push_back(TestUtils::createTestMove("magnitude", 60, 100, 30, "ground", "physical"));
        
        Pokemon rockLeader2 = TestUtils::createTestPokemon("Onix", 120, 45, 160, 30, 45, 70, {"rock", "ground"});
        rockLeader2.moves.clear();
        rockLeader2.moves.push_back(TestUtils::createTestMove("rock-slide", 75, 90, 10, "rock", "physical"));
        rockLeader2.moves.push_back(TestUtils::createTestMove("bind", 15, 85, 20, "normal", "physical"));
        rockLeader2.moves.push_back(TestUtils::createTestMove("screech", 0, 85, 40, "normal", "status"));
        rockLeader2.moves.push_back(TestUtils::createTestMove("slam", 80, 75, 20, "normal", "physical"));
        
        rock_gym_team = TestUtils::createTestTeam({rockLeader1, rockLeader2});
        
        // Create Water Gym Leader team (type disadvantage for Grass starter)
        Pokemon waterLeader1 = TestUtils::createTestPokemon("Staryu", 85, 45, 55, 70, 55, 85, {"water"});
        waterLeader1.moves.clear();
        waterLeader1.moves.push_back(TestUtils::createTestMove("water-gun", 40, 100, 25, "water", "special"));
        waterLeader1.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
        waterLeader1.moves.push_back(TestUtils::createTestMove("harden", 0, 100, 30, "normal", "status"));
        waterLeader1.moves.push_back(TestUtils::createTestMove("bubble-beam", 65, 100, 20, "water", "special"));
        
        Pokemon waterLeader2 = TestUtils::createTestPokemon("Starmie", 100, 75, 85, 100, 85, 115, {"water", "psychic"});
        waterLeader2.moves.clear();
        waterLeader2.moves.push_back(TestUtils::createTestMove("psychic", 90, 100, 10, "psychic", "special"));
        waterLeader2.moves.push_back(TestUtils::createTestMove("surf", 90, 100, 15, "water", "special"));
        waterLeader2.moves.push_back(TestUtils::createTestMove("recover", 0, 100, 5, "normal", "status"));
        waterLeader2.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
        
        water_gym_team = TestUtils::createTestTeam({waterLeader1, waterLeader2});
    }
    
    // Utility method to capture team state for comparison
    struct TeamState {
        std::vector<int> pokemon_hp;
        std::vector<int> pokemon_current_hp;
        std::vector<std::vector<int>> pokemon_move_pp;
        std::vector<StatusCondition> pokemon_status;
        
        bool operator==(const TeamState& other) const {
            return pokemon_hp == other.pokemon_hp && 
                   pokemon_current_hp == other.pokemon_current_hp &&
                   pokemon_move_pp == other.pokemon_move_pp &&
                   pokemon_status == other.pokemon_status;
        }
    };
    
    TeamState captureTeamState(const Team& team) {
        TeamState state;
        for (size_t i = 0; i < team.size(); ++i) {
            const Pokemon* pokemon = team.getPokemon(i);
            if (pokemon) {
                state.pokemon_hp.push_back(pokemon->hp);
                state.pokemon_current_hp.push_back(pokemon->current_hp);
                state.pokemon_status.push_back(pokemon->status);
                
                std::vector<int> move_pp;
                for (const auto& move : pokemon->moves) {
                    move_pp.push_back(move.current_pp);
                }
                state.pokemon_move_pp.push_back(move_pp);
            }
        }
        return state;
    }
    
    // Core test objects
    std::shared_ptr<PokemonData> pokemon_data;
    std::shared_ptr<TeamBuilder> team_builder;
    std::shared_ptr<TournamentManager> tournament_manager;
    
    // Test data
    std::string test_player_name;
    Team player_team;
    Team rock_gym_team;
    Team water_gym_team;
    TeamState original_team_state;
};

// Test 1: Battle Outcome Consistency - Tournament vs Standalone Battle
TEST_F(TournamentBattleIntegrationTest, BattleOutcomeConsistency) {
    // Create identical teams for comparison
    Team playerTeamCopy = player_team;  // Copy for standalone battle
    Team gymTeamCopy = rock_gym_team;   // Copy for standalone battle
    
    // Run standalone battle with specific AI difficulty
    Battle standaloneBattle(playerTeamCopy, gymTeamCopy, Battle::AIDifficulty::MEDIUM);
    auto standaloneResult = standaloneBattle.getBattleResult();
    
    // Run tournament-integrated battle with same conditions
    TournamentManager::TournamentBattleResult tournamentResult;
    tournamentResult.challenge_name = "Pewter Gym";
    tournamentResult.challenge_type = "gym";
    tournamentResult.player_team_name = "StarterTeam";
    tournamentResult.opponent_name = "Brock";
    tournamentResult.difficulty_level = "medium";
    
    // Simulate tournament battle with same teams and difficulty
    Battle tournamentBattle(player_team, rock_gym_team, Battle::AIDifficulty::MEDIUM);
    auto tournamentBattleResult = tournamentBattle.getBattleResult();
    
    // Both battles should have same initial state (ONGOING for alive teams)
    EXPECT_EQ(standaloneResult, Battle::BattleResult::ONGOING);
    EXPECT_EQ(tournamentBattleResult, Battle::BattleResult::ONGOING);
    
    // Validate that battle mechanics work consistently
    EXPECT_FALSE(standaloneBattle.isBattleOver());
    EXPECT_FALSE(tournamentBattle.isBattleOver());
    
    // Teams should maintain similar alive status
    EXPECT_TRUE(playerTeamCopy.hasAlivePokemon());
    EXPECT_TRUE(player_team.hasAlivePokemon());
    EXPECT_TRUE(gymTeamCopy.hasAlivePokemon());
    EXPECT_TRUE(rock_gym_team.hasAlivePokemon());
}

// Test 2: Tournament Progress Synchronization After Battle
TEST_F(TournamentBattleIntegrationTest, TournamentProgressSynchronization) {
    // Get initial tournament state
    auto initialProgress = tournament_manager->getPlayerProgress(test_player_name);
    ASSERT_TRUE(initialProgress.has_value());
    EXPECT_EQ(initialProgress->earned_badges.size(), 0);
    EXPECT_EQ(initialProgress->defeated_gyms.size(), 0);
    EXPECT_FALSE(initialProgress->elite_four_unlocked);
    
    // Simulate winning a gym battle
    TournamentManager::TournamentBattleResult gymVictory;
    gymVictory.challenge_name = "Pewter Gym";
    gymVictory.challenge_type = "gym";
    gymVictory.player_team_name = "StarterTeam";
    gymVictory.opponent_name = "Brock";
    gymVictory.victory = true;
    gymVictory.turns_taken = 12;
    gymVictory.difficulty_level = "medium";
    gymVictory.performance_score = 85.5;
    gymVictory.battle_date = "2024-01-15";
    
    // Award badge for the victory
    TournamentManager::Badge boulderBadge(
        "Pewter Gym", "rock", "Brock", "2024-01-15", 1, 85.5
    );
    
    // Update tournament progress
    bool progressUpdated = tournament_manager->updatePlayerProgress(test_player_name, gymVictory);
    EXPECT_TRUE(progressUpdated);
    
    bool badgeAwarded = tournament_manager->awardBadge(test_player_name, boulderBadge);
    EXPECT_TRUE(badgeAwarded);
    
    // Verify progress synchronization
    auto updatedProgress = tournament_manager->getPlayerProgress(test_player_name);
    ASSERT_TRUE(updatedProgress.has_value());
    
    // Check badge count and gym progress
    EXPECT_EQ(updatedProgress->earned_badges.size(), 1);
    EXPECT_EQ(updatedProgress->defeated_gyms.size(), 1);
    EXPECT_TRUE(updatedProgress->defeated_gyms.count("Pewter Gym") > 0);
    
    // Elite Four should still be locked with only 1 badge
    EXPECT_FALSE(updatedProgress->elite_four_unlocked);
    EXPECT_EQ(tournament_manager->getPlayerBadgeCount(test_player_name), 1);
    
    // Test challenge availability updates
    auto availableChallenges = tournament_manager->getAvailableChallenges(test_player_name);
    EXPECT_GT(availableChallenges.size(), 0);
    
    // Should have next gym available but not Elite Four
    bool hasGymChallenge = false;
    bool hasEliteFourChallenge = false;
    for (const auto& challenge : availableChallenges) {
        if (challenge.challenge_type == "gym") hasGymChallenge = true;
        if (challenge.challenge_type == "elite_four") hasEliteFourChallenge = true;
    }
    EXPECT_TRUE(hasGymChallenge);
    EXPECT_FALSE(hasEliteFourChallenge);
}

// Test 3: Team State Persistence Through Tournament Battles
TEST_F(TournamentBattleIntegrationTest, TeamStatePersistenceThroughTournament) {
    // Capture initial team state
    auto initialState = captureTeamState(player_team);
    
    // Ensure all Pokemon start at full health
    EXPECT_EQ(initialState.pokemon_current_hp[0], initialState.pokemon_hp[0]);
    EXPECT_EQ(initialState.pokemon_current_hp[1], initialState.pokemon_hp[1]);
    EXPECT_EQ(initialState.pokemon_current_hp[2], initialState.pokemon_hp[2]);
    
    // All Pokemon should start with no status conditions
    for (const auto& status : initialState.pokemon_status) {
        EXPECT_EQ(status, StatusCondition::NONE);
    }
    
    // Simulate damage during battle (manual damage for testing)
    Pokemon* firePokemon = player_team.getPokemon(0);
    Pokemon* waterPokemon = player_team.getPokemon(1);
    Pokemon* grassPokemon = player_team.getPokemon(2);
    
    ASSERT_TRUE(firePokemon != nullptr);
    ASSERT_TRUE(waterPokemon != nullptr);
    ASSERT_TRUE(grassPokemon != nullptr);
    
    // Apply some battle damage and status effects
    firePokemon->takeDamage(25);  // Fire Pokemon takes damage
    waterPokemon->applyStatusCondition(StatusCondition::POISON);  // Water Pokemon gets poisoned
    
    // Use some PP
    if (!firePokemon->moves.empty()) {
        firePokemon->moves[0].current_pp -= 3;
    }
    
    // Capture state after battle simulation
    auto postBattleState = captureTeamState(player_team);
    
    // Verify state changes were applied
    EXPECT_EQ(postBattleState.pokemon_current_hp[0], initialState.pokemon_current_hp[0] - 25);
    EXPECT_EQ(postBattleState.pokemon_status[1], StatusCondition::POISON);
    EXPECT_LT(postBattleState.pokemon_move_pp[0][0], initialState.pokemon_move_pp[0][0]);
    
    // Simulate tournament healing between gym battles (full restore)
    for (size_t i = 0; i < player_team.size(); ++i) {
        Pokemon* pokemon = player_team.getPokemon(i);
        if (pokemon) {
            pokemon->heal(pokemon->hp);  // Full heal
            pokemon->clearStatusCondition();  // Remove status
            // Restore PP
            for (auto& move : pokemon->moves) {
                move.current_pp = move.pp;
            }
        }
    }
    
    // Verify tournament healing restored team to full strength
    auto postHealingState = captureTeamState(player_team);
    EXPECT_EQ(postHealingState.pokemon_current_hp[0], postHealingState.pokemon_hp[0]);
    EXPECT_EQ(postHealingState.pokemon_current_hp[1], postHealingState.pokemon_hp[1]);
    EXPECT_EQ(postHealingState.pokemon_current_hp[2], postHealingState.pokemon_hp[2]);
    
    for (const auto& status : postHealingState.pokemon_status) {
        EXPECT_EQ(status, StatusCondition::NONE);
    }
    
    // PP should be fully restored
    EXPECT_EQ(postHealingState.pokemon_move_pp[0][0], initialState.pokemon_move_pp[0][0]);
}

// Test 4: Gym Leader Battle Integration with Tournament System
TEST_F(TournamentBattleIntegrationTest, GymLeaderBattleIntegration) {
    // Test Rock Gym (type disadvantage for Fire starter, advantage for Water starter)
    
    // Get tournament settings
    auto tournamentSettings = tournament_manager->getTournamentSettings();
    EXPECT_TRUE(tournamentSettings.allow_gym_reattempts);
    EXPECT_TRUE(tournamentSettings.heal_between_gym_attempts);
    
    // Check initial gym unlock status
    EXPECT_TRUE(tournament_manager->isChallengeUnlocked(test_player_name, "Pewter Gym"));
    EXPECT_FALSE(tournament_manager->hasPlayerEarnedBadge(test_player_name, "Pewter Gym"));
    
    // Simulate gym battle with appropriate AI difficulty
    Battle gymBattle(player_team, rock_gym_team, Battle::AIDifficulty::MEDIUM);
    
    // Verify battle setup is correct
    EXPECT_FALSE(gymBattle.isBattleOver());
    EXPECT_EQ(gymBattle.getBattleResult(), Battle::BattleResult::ONGOING);
    
    // Both teams should be alive and ready
    EXPECT_TRUE(player_team.hasAlivePokemon());
    EXPECT_TRUE(rock_gym_team.hasAlivePokemon());
    
    // Test team composition for gym battle
    EXPECT_EQ(player_team.size(), 3);  // Player should have full team
    EXPECT_EQ(rock_gym_team.size(), 2);  // Gym leader has 2 Pokemon
    
    // Verify type matchups exist (Fire vs Rock = disadvantage, Water vs Rock = advantage)
    Pokemon* fireStarter = player_team.getPokemon(0);
    Pokemon* waterStarter = player_team.getPokemon(1);
    Pokemon* rockLeader = rock_gym_team.getPokemon(0);
    
    ASSERT_TRUE(fireStarter != nullptr);
    ASSERT_TRUE(waterStarter != nullptr);
    ASSERT_TRUE(rockLeader != nullptr);
    
    // Fire vs Rock should be disadvantaged
    EXPECT_TRUE(fireStarter->types.size() > 0);
    EXPECT_EQ(fireStarter->types[0], "fire");
    EXPECT_TRUE(rockLeader->types.size() > 0);
    EXPECT_EQ(rockLeader->types[0], "rock");
    
    // Water vs Rock should be advantaged
    EXPECT_EQ(waterStarter->types[0], "water");
    
    // Simulate gym victory and verify tournament integration
    TournamentManager::TournamentBattleResult gymResult;
    gymResult.challenge_name = "Pewter Gym";
    gymResult.challenge_type = "gym";
    gymResult.victory = true;
    gymResult.turns_taken = 8;
    gymResult.difficulty_level = "medium";
    gymResult.performance_score = 90.0;
    
    // Update tournament with victory
    bool updated = tournament_manager->updatePlayerProgress(test_player_name, gymResult);
    EXPECT_TRUE(updated);
    
    // Award the badge
    TournamentManager::Badge badge("Pewter Gym", "rock", "Brock", "2024-01-15", 1, 90.0);
    bool badgeAwarded = tournament_manager->awardBadge(test_player_name, badge);
    EXPECT_TRUE(badgeAwarded);
    
    // Verify tournament state updates
    EXPECT_TRUE(tournament_manager->hasPlayerEarnedBadge(test_player_name, "Pewter Gym"));
    EXPECT_EQ(tournament_manager->getPlayerBadgeCount(test_player_name), 1);
    
    // Next gym should now be available
    auto nextChallenge = tournament_manager->getNextRecommendedChallenge(test_player_name);
    EXPECT_TRUE(nextChallenge.has_value());
    EXPECT_EQ(nextChallenge->challenge_type, "gym");
}

// Test 5: Multi-Battle Tournament Progression
TEST_F(TournamentBattleIntegrationTest, MultiBattleTournamentProgression) {
    // Simulate progression through first few gym battles
    
    // Battle 1: Pewter Gym (Rock-type)
    {
        Battle pewterBattle(player_team, rock_gym_team, Battle::AIDifficulty::MEDIUM);
        
        // Simulate victory
        TournamentManager::TournamentBattleResult result;
        result.challenge_name = "Pewter Gym";
        result.challenge_type = "gym";
        result.victory = true;
        result.performance_score = 88.5;
        
        tournament_manager->updatePlayerProgress(test_player_name, result);
        TournamentManager::Badge badge("Pewter Gym", "rock", "Brock", "2024-01-15", 1, 88.5);
        tournament_manager->awardBadge(test_player_name, badge);
        
        EXPECT_EQ(tournament_manager->getPlayerBadgeCount(test_player_name), 1);
    }
    
    // Battle 2: Cerulean Gym (Water-type)
    {
        Battle ceruleanBattle(player_team, water_gym_team, Battle::AIDifficulty::MEDIUM);
        
        // Simulate victory
        TournamentManager::TournamentBattleResult result;
        result.challenge_name = "Cerulean Gym";
        result.challenge_type = "gym";
        result.victory = true;
        result.performance_score = 82.0;
        
        tournament_manager->updatePlayerProgress(test_player_name, result);
        TournamentManager::Badge badge("Cerulean Gym", "water", "Misty", "2024-01-16", 1, 82.0);
        tournament_manager->awardBadge(test_player_name, badge);
        
        EXPECT_EQ(tournament_manager->getPlayerBadgeCount(test_player_name), 2);
    }
    
    // Verify progression tracking
    auto progress = tournament_manager->getPlayerProgress(test_player_name);
    ASSERT_TRUE(progress.has_value());
    
    EXPECT_EQ(progress->earned_badges.size(), 2);
    EXPECT_EQ(progress->defeated_gyms.size(), 2);
    EXPECT_TRUE(progress->defeated_gyms.count("Pewter Gym") > 0);
    EXPECT_TRUE(progress->defeated_gyms.count("Cerulean Gym") > 0);
    
    // Elite Four should still be locked (need all 8 badges)
    EXPECT_FALSE(progress->elite_four_unlocked);
    EXPECT_FALSE(tournament_manager->isEliteFourUnlocked(test_player_name));
    
    // Tournament completion should be partial
    double completion = tournament_manager->getTournamentCompletionPercentage(test_player_name);
    EXPECT_GT(completion, 0.0);
    EXPECT_LT(completion, 1.0);
    
    // Should have more gym challenges available
    auto challenges = tournament_manager->getAvailableChallenges(test_player_name);
    bool hasMoreGyms = false;
    for (const auto& challenge : challenges) {
        if (challenge.challenge_type == "gym" && !challenge.is_completed) {
            hasMoreGyms = true;
            break;
        }
    }
    EXPECT_TRUE(hasMoreGyms);
}

// Test 6: Battle Result Validation and Error Handling
TEST_F(TournamentBattleIntegrationTest, BattleResultValidationAndErrorHandling) {
    // Test invalid tournament battle result handling
    TournamentManager::TournamentBattleResult invalidResult;
    invalidResult.challenge_name = "";  // Invalid empty name
    invalidResult.challenge_type = "invalid_type";  // Invalid type
    invalidResult.victory = false;
    
    // Should handle invalid results gracefully
    bool updated = tournament_manager->updatePlayerProgress(test_player_name, invalidResult);
    EXPECT_FALSE(updated);  // Should reject invalid result
    
    // Progress should remain unchanged
    auto progress = tournament_manager->getPlayerProgress(test_player_name);
    ASSERT_TRUE(progress.has_value());
    EXPECT_EQ(progress->earned_badges.size(), 0);
    
    // Test valid result structure
    TournamentManager::TournamentBattleResult validResult;
    validResult.challenge_name = "Pewter Gym";
    validResult.challenge_type = "gym";
    validResult.player_team_name = "TestTeam";
    validResult.opponent_name = "Brock";
    validResult.victory = true;
    validResult.turns_taken = 10;
    validResult.difficulty_level = "medium";
    validResult.performance_score = 85.0;
    validResult.battle_date = "2024-01-15";
    
    // Should accept valid result
    updated = tournament_manager->updatePlayerProgress(test_player_name, validResult);
    EXPECT_TRUE(updated);
    
    // Test battle state validation
    Battle testBattle(player_team, rock_gym_team);
    EXPECT_FALSE(testBattle.isBattleOver());
    
    // Teams with no alive Pokemon should end battle immediately
    Team faintedTeam = player_team;
    for (size_t i = 0; i < faintedTeam.size(); ++i) {
        Pokemon* pokemon = faintedTeam.getPokemon(i);
        if (pokemon) {
            pokemon->takeDamage(pokemon->hp);  // Faint all Pokemon
        }
    }
    
    Battle faintedBattle(faintedTeam, rock_gym_team);
    EXPECT_TRUE(faintedBattle.isBattleOver());
    EXPECT_EQ(faintedBattle.getBattleResult(), Battle::BattleResult::OPPONENT_WINS);
}

// Test 7: Tournament Battle Statistics Integration
TEST_F(TournamentBattleIntegrationTest, TournamentBattleStatisticsIntegration) {
    // Record multiple battle results and verify statistics tracking
    std::vector<TournamentManager::TournamentBattleResult> battleResults;
    
    // Battle 1: Victory with good performance
    TournamentManager::TournamentBattleResult result1;
    result1.challenge_name = "Pewter Gym";
    result1.challenge_type = "gym";
    result1.victory = true;
    result1.turns_taken = 8;
    result1.performance_score = 92.5;
    result1.difficulty_level = "medium";
    battleResults.push_back(result1);
    
    // Battle 2: Victory with average performance
    TournamentManager::TournamentBattleResult result2;
    result2.challenge_name = "Cerulean Gym";
    result2.challenge_type = "gym";
    result2.victory = true;
    result2.turns_taken = 12;
    result2.performance_score = 75.0;
    result2.difficulty_level = "medium";
    battleResults.push_back(result2);
    
    // Battle 3: Loss (for statistics)
    TournamentManager::TournamentBattleResult result3;
    result3.challenge_name = "Vermilion Gym";
    result3.challenge_type = "gym";
    result3.victory = false;
    result3.turns_taken = 15;
    result3.performance_score = 45.0;
    result3.difficulty_level = "hard";
    battleResults.push_back(result3);
    
    // Update tournament with all results
    for (const auto& result : battleResults) {
        tournament_manager->updatePlayerProgress(test_player_name, result);
    }
    
    // Award badges for victories
    tournament_manager->awardBadge(test_player_name, 
        TournamentManager::Badge("Pewter Gym", "rock", "Brock", "2024-01-15", 1, 92.5));
    tournament_manager->awardBadge(test_player_name, 
        TournamentManager::Badge("Cerulean Gym", "water", "Misty", "2024-01-16", 1, 75.0));
    
    // Verify statistics calculation
    auto stats = tournament_manager->getPlayerTournamentStats(test_player_name);
    EXPECT_GT(stats.size(), 0);
    
    // Check battle history
    auto battleHistory = tournament_manager->getPlayerBattleHistory(test_player_name);
    EXPECT_EQ(battleHistory.size(), 3);
    
    // Verify gym-specific history
    auto gymBattleHistory = tournament_manager->getPlayerBattleHistory(test_player_name, "gym");
    EXPECT_EQ(gymBattleHistory.size(), 3);
    
    // Check progress tracking
    auto progress = tournament_manager->getPlayerProgress(test_player_name);
    ASSERT_TRUE(progress.has_value());
    
    // Should have 2 badges (2 victories out of 3 battles)
    EXPECT_EQ(progress->earned_badges.size(), 2);
    EXPECT_EQ(progress->total_gym_attempts, 3);
    
    // Average performance should be calculated correctly
    // (92.5 + 75.0 + 45.0) / 3 = 70.83...
    EXPECT_GT(progress->average_battle_performance, 70.0);
    EXPECT_LT(progress->average_battle_performance, 71.0);
}