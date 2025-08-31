#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

#include "integration_test_utilities.h"
#include "test_utils.h"
#include "tournament_manager.h"
#include "championship_system.h"
#include "battle.h"
#include "team_builder.h"
#include "pokemon_data.h"

/**
 * @brief Tournament Progression Scenario Integration Tests
 * 
 * These tests validate specific progression scenarios including:
 * - Non-linear gym progression where permitted
 * - Failure and recovery workflows with retry mechanics
 * - Tournament state persistence and resume functionality
 * - Edge cases and boundary conditions in tournament progression
 * 
 * These tests complement the main end-to-end gameplay flow tests by focusing
 * on specific progression patterns and edge cases.
 */
class TournamentProgressionScenariosTest : public IntegrationTestUtils::MultiSystemIntegrationFixture {
protected:
    void SetUp() override {
        IntegrationTestUtils::MultiSystemIntegrationFixture::SetUp();
        
        // Setup test data generator for progression scenario testing
        test_data_generator = std::make_unique<IntegrationTestUtils::IntegrationTestDataGenerator>(rng);
        
        // Initialize specialized test configurations
        setupProgressionTestConfigurations();
        
        // Setup failure simulation parameters
        setupFailureScenarios();
    }
    
    void setupProgressionTestConfigurations() {
        // Non-linear progression configuration
        TournamentManager::TournamentSettings nonlinear_config;
        nonlinear_config.require_sequential_gyms = false;
        nonlinear_config.allow_gym_reattempts = true;
        nonlinear_config.max_attempts_per_gym = 0; // Unlimited
        nonlinear_config.heal_between_gym_attempts = true;
        nonlinear_config.require_all_badges = true;
        nonlinear_config.sequential_elite_four = false; // Allow non-sequential Elite Four
        nonlinear_config.heal_between_elite_battles = true;
        progression_configs["nonlinear"] = nonlinear_config;
        
        // Strict progression configuration  
        TournamentManager::TournamentSettings strict_config;
        strict_config.require_sequential_gyms = true;
        strict_config.allow_gym_reattempts = false;
        strict_config.max_attempts_per_gym = 1;
        strict_config.heal_between_gym_attempts = false;
        strict_config.require_all_badges = true;
        strict_config.sequential_elite_four = true;
        strict_config.heal_between_elite_battles = false;
        progression_configs["strict"] = strict_config;
        
        // Recovery-focused configuration
        TournamentManager::TournamentSettings recovery_config;
        recovery_config.require_sequential_gyms = false;
        recovery_config.allow_gym_reattempts = true;
        recovery_config.max_attempts_per_gym = 5; // Limited retries
        recovery_config.heal_between_gym_attempts = true;
        recovery_config.require_all_badges = true;
        recovery_config.sequential_elite_four = true;
        recovery_config.heal_between_elite_battles = true;
        progression_configs["recovery"] = recovery_config;
    }
    
    void setupFailureScenarios() {
        // Early failure scenario - fails at first few gyms
        FailureScenario early_failure;
        early_failure.name = "early_failure";
        early_failure.failure_points = {0, 1, 2}; // First 3 gyms
        early_failure.failure_probability = 0.8; // High failure rate
        early_failure.recovery_pattern = "gradual"; // Gradually improves
        failure_scenarios["early"] = early_failure;
        
        // Mid-tournament failure - struggles in middle
        FailureScenario mid_failure;
        mid_failure.name = "mid_failure";
        mid_failure.failure_points = {3, 4, 5}; // Middle gyms
        mid_failure.failure_probability = 0.6;
        mid_failure.recovery_pattern = "quick"; // Quick recovery
        failure_scenarios["mid"] = mid_failure;
        
        // Elite Four failure - struggles at championship level
        FailureScenario elite_failure;
        elite_failure.name = "elite_failure";
        elite_failure.failure_points = {8, 9, 10, 11}; // Elite Four
        elite_failure.failure_probability = 0.7;
        elite_failure.recovery_pattern = "persistent"; // Requires many attempts
        failure_scenarios["elite"] = elite_failure;
    }
    
    // Test scenario structures
    struct FailureScenario {
        std::string name;
        std::vector<int> failure_points; // Challenge indices where failure occurs
        double failure_probability; // Probability of failure at failure points
        std::string recovery_pattern; // How recovery happens
    };
    
    struct ProgressionResult {
        bool completed;
        int total_attempts;
        std::vector<int> gym_completion_order;
        std::vector<int> attempts_per_challenge;
        double average_performance;
        std::chrono::milliseconds total_time;
    };
    
    // Test components
    std::unique_ptr<IntegrationTestUtils::IntegrationTestDataGenerator> test_data_generator;
    std::unordered_map<std::string, TournamentManager::TournamentSettings> progression_configs;
    std::unordered_map<std::string, FailureScenario> failure_scenarios;
    
    // Helper methods
    ProgressionResult simulateNonLinearProgression(
        const std::string& player_name,
        const TournamentManager::TournamentSettings& config,
        const std::vector<int>& gym_order
    );
    
    ProgressionResult simulateFailureRecoveryProgression(
        const std::string& player_name,
        const FailureScenario& scenario,
        const TournamentManager::TournamentSettings& config
    );
    
    bool validateNonLinearProgressionOrder(
        const std::vector<int>& completion_order,
        const std::vector<int>& expected_order
    );
    
    bool simulateProgressionPersistence(
        const std::string& player_name,
        int checkpoint_at_challenge
    );
    
    std::vector<int> generateRandomGymOrder();
    
    IntegrationTestUtils::IntegrationBattleResult simulateGymBattle(
        const std::string& player_name,
        Team& player_team,
        int gym_index,
        const FailureScenario& scenario,
        int attempt_number = 1
    );
};

// Test 1: Non-Linear Gym Progression - Player Choice Order
TEST_F(TournamentProgressionScenariosTest, NonLinearGymProgressionPlayerChoice) {
    // Test non-sequential gym progression when configuration allows it
    tournament_manager->setTournamentSettings(progression_configs["nonlinear"]);
    
    std::string player_name = "NonLinearTrainer";
    tournament_manager->initializePlayerProgress(player_name);
    
    // Generate a non-sequential gym order (e.g., 3, 1, 7, 2, 5, 8, 4, 6)
    std::vector<int> custom_gym_order = {2, 0, 6, 1, 4, 7, 3, 5}; // 0-indexed
    
    auto progression_result = simulateNonLinearProgression(
        player_name, progression_configs["nonlinear"], custom_gym_order
    );
    
    // Validate non-linear progression was successful
    EXPECT_TRUE(progression_result.completed);
    EXPECT_EQ(progression_result.gym_completion_order.size(), 8);
    
    // Verify actual completion order matches custom order
    EXPECT_TRUE(validateNonLinearProgressionOrder(
        progression_result.gym_completion_order, custom_gym_order
    ));
    
    // Tournament manager should track all badges regardless of order
    EXPECT_EQ(tournament_manager->getPlayerBadgeCount(player_name), 8);
    EXPECT_TRUE(tournament_manager->isEliteFourUnlocked(player_name));
    
    // Verify tournament completion
    auto final_progress = tournament_manager->getPlayerProgress(player_name);
    ASSERT_TRUE(final_progress.has_value());
    EXPECT_EQ(final_progress->earned_badges.size(), 8);
    EXPECT_EQ(final_progress->defeated_gyms.size(), 8);
}

// Test 2: Strategic Gym Order Based on Type Advantages  
TEST_F(TournamentProgressionScenariosTest, StrategicGymOrderBasedOnTypeAdvantages) {
    tournament_manager->setTournamentSettings(progression_configs["nonlinear"]);
    
    std::string player_name = "StrategicTrainer";
    tournament_manager->initializePlayerProgress(player_name);
    
    // Create a Fire-type focused team
    Team fire_team = IntegrationTestUtils::createBalancedTournamentTeam("FireTeam");
    
    // Strategic order: Rock(weak) → Grass(strong) → Ice(strong) → Water(weak) → etc.
    // Prioritizing type advantages early for confidence building
    std::vector<int> strategic_order = {0, 3, 5, 1, 2, 6, 4, 7}; // Rock, Grass, Psychic, Water, Electric, Fire, Poison, Ground
    
    auto progression_result = simulateNonLinearProgression(
        player_name, progression_configs["nonlinear"], strategic_order
    );
    
    // Strategic progression should be successful
    EXPECT_TRUE(progression_result.completed);
    
    // Should have good performance due to strategic type matchups early
    EXPECT_GT(progression_result.average_performance, 75.0);
    
    // Should require fewer total attempts due to strategic planning
    EXPECT_LT(progression_result.total_attempts, 15); // Less than average due to strategy
    
    // Verify battle history reflects strategic order
    auto battle_history = tournament_manager->getPlayerBattleHistory(player_name, "gym");
    
    // First few battles should be easier (type advantages)
    if (battle_history.size() >= 3) {
        double early_performance = 0.0;
        for (int i = 0; i < 3; ++i) {
            early_performance += battle_history[i].performance_score;
        }
        early_performance /= 3.0;
        EXPECT_GT(early_performance, 80.0); // Good early performance from strategic planning
    }
}

// Test 3: Failure Recovery Workflow - Early Tournament Struggles
TEST_F(TournamentProgressionScenariosTest, FailureRecoveryWorkflowEarlyTournamentStruggles) {
    tournament_manager->setTournamentSettings(progression_configs["recovery"]);
    
    std::string player_name = "StrugglingTrainer";
    tournament_manager->initializePlayerProgress(player_name);
    
    // Simulate early failure scenario
    const auto& early_failure = failure_scenarios["early"];
    
    auto progression_result = simulateFailureRecoveryProgression(
        player_name, early_failure, progression_configs["recovery"]
    );
    
    // Should eventually succeed despite early struggles
    EXPECT_TRUE(progression_result.completed);
    
    // Should have more attempts due to early failures
    EXPECT_GT(progression_result.total_attempts, 20);
    
    // Early challenges should show multiple attempts
    EXPECT_GT(progression_result.attempts_per_challenge[0], 1); // First gym multiple attempts
    EXPECT_GT(progression_result.attempts_per_challenge[1], 1); // Second gym multiple attempts
    
    // Later challenges should show improvement (fewer attempts)
    if (progression_result.attempts_per_challenge.size() >= 8) {
        double early_avg_attempts = (progression_result.attempts_per_challenge[0] + 
                                    progression_result.attempts_per_challenge[1] + 
                                    progression_result.attempts_per_challenge[2]) / 3.0;
        double late_avg_attempts = (progression_result.attempts_per_challenge[5] + 
                                   progression_result.attempts_per_challenge[6] + 
                                   progression_result.attempts_per_challenge[7]) / 3.0;
        
        EXPECT_GT(early_avg_attempts, late_avg_attempts); // Recovery pattern
    }
    
    // Verify tournament manager recorded all attempts
    auto final_progress = tournament_manager->getPlayerProgress(player_name);
    ASSERT_TRUE(final_progress.has_value());
    EXPECT_GT(final_progress->total_gym_attempts, 13); // More attempts than just victories
}

// Test 4: Mid-Tournament Plateau and Recovery
TEST_F(TournamentProgressionScenariosTest, MidTournamentPlateauAndRecovery) {
    tournament_manager->setTournamentSettings(progression_configs["recovery"]);
    
    std::string player_name = "PlateauTrainer";
    tournament_manager->initializePlayerProgress(player_name);
    
    // Simulate mid-tournament failure scenario
    const auto& mid_failure = failure_scenarios["mid"];
    
    auto progression_result = simulateFailureRecoveryProgression(
        player_name, mid_failure, progression_configs["recovery"]
    );
    
    // Should complete despite mid-tournament struggles
    EXPECT_TRUE(progression_result.completed);
    
    // Should show good early performance, struggles in middle, recovery at end
    if (progression_result.attempts_per_challenge.size() >= 8) {
        // Early gyms should be easier (fewer attempts)
        double early_attempts = (progression_result.attempts_per_challenge[0] + 
                               progression_result.attempts_per_challenge[1]) / 2.0;
        
        // Middle gyms should have more attempts (plateau)
        double mid_attempts = (progression_result.attempts_per_challenge[3] + 
                             progression_result.attempts_per_challenge[4] + 
                             progression_result.attempts_per_challenge[5]) / 3.0;
        
        // Late gyms should recover (fewer attempts than middle)
        double late_attempts = (progression_result.attempts_per_challenge[6] + 
                              progression_result.attempts_per_challenge[7]) / 2.0;
        
        EXPECT_LT(early_attempts, mid_attempts); // Plateau in middle
        EXPECT_LT(late_attempts, mid_attempts);  // Recovery at end
    }
    
    // Overall should still achieve tournament completion
    auto final_progress = tournament_manager->getPlayerProgress(player_name);
    ASSERT_TRUE(final_progress.has_value());
    EXPECT_EQ(final_progress->earned_badges.size(), 8);
    EXPECT_TRUE(tournament_manager->isEliteFourUnlocked(player_name));
}

// Test 5: Elite Four Failure and Persistence
TEST_F(TournamentProgressionScenariosTest, EliteFourFailureAndPersistence) {
    tournament_manager->setTournamentSettings(progression_configs["recovery"]);
    
    std::string player_name = "PersistentTrainer";
    tournament_manager->initializePlayerProgress(player_name);
    
    // First complete gym progression normally
    Team persistent_team = IntegrationTestUtils::createBalancedTournamentTeam("PersistentTeam");
    
    // Award all gym badges to unlock Elite Four
    std::vector<std::string> gym_names = {"Pewter", "Cerulean", "Vermilion", "Celadon", 
                                         "Fuchsia", "Saffron", "Cinnabar", "Viridian"};
    std::vector<std::string> gym_types = {"rock", "water", "electric", "grass", 
                                         "poison", "psychic", "fire", "ground"};
    
    for (size_t i = 0; i < gym_names.size(); ++i) {
        TournamentManager::Badge badge(gym_names[i] + " Gym", gym_types[i], 
                                     gym_names[i] + "Leader", "2024-01-15", 1, 85.0);
        tournament_manager->awardBadge(player_name, badge);
    }
    
    ASSERT_TRUE(tournament_manager->isEliteFourUnlocked(player_name));
    
    // Now simulate Elite Four failure scenario
    const auto& elite_failure = failure_scenarios["elite"];
    
    // Simulate Elite Four battles with high failure rate initially
    std::vector<std::string> elite_members = {"Lorelei", "Bruno", "Agatha", "Lance"};
    int total_elite_attempts = 0;
    
    for (size_t i = 0; i < elite_members.size(); ++i) {
        bool member_defeated = false;
        int attempt = 1;
        const int max_attempts = 4; // Persistent attempts
        
        while (!member_defeated && attempt <= max_attempts) {
            // Create Elite Four battle
            Team elite_team = test_data_generator->generateEliteFourTeam(
                (i == 0) ? "ice" : (i == 1) ? "fighting" : (i == 2) ? "ghost" : "dragon", 3
            );
            
            auto battle_result = IntegrationTestUtils::simulateBattle(
                persistent_team, elite_team, Battle::AIDifficulty::EXPERT,
                elite_members[i] + "_Attempt" + std::to_string(attempt)
            );
            
            total_elite_attempts++;
            
            // Determine success based on elite failure scenario
            bool should_fail = (attempt <= 2) && (rng() % 100 < (elite_failure.failure_probability * 100));
            
            if (!should_fail || attempt == max_attempts) { // Success or final attempt
                battle_result.victory = true;
                member_defeated = true;
                
                TournamentManager::TournamentBattleResult tournament_result;
                tournament_result.challenge_name = elite_members[i] + " of the Elite Four";
                tournament_result.challenge_type = "elite_four";
                tournament_result.victory = true;
                tournament_result.performance_score = 70.0 + (attempt * 5.0); // Better with more attempts
                tournament_result.turns_taken = battle_result.turns_taken;
                tournament_result.difficulty_level = "expert";
                
                tournament_manager->updatePlayerProgress(player_name, tournament_result);
            } else {
                battle_result.victory = false;
                
                TournamentManager::TournamentBattleResult failed_result;
                failed_result.challenge_name = elite_members[i] + " of the Elite Four";
                failed_result.challenge_type = "elite_four";
                failed_result.victory = false;
                failed_result.performance_score = 40.0 + (rng() % 20);
                failed_result.difficulty_level = "expert";
                
                tournament_manager->updatePlayerProgress(player_name, failed_result);
            }
            
            attempt++;
        }
    }
    
    // Should have completed Elite Four after persistence
    auto final_progress = tournament_manager->getPlayerProgress(player_name);
    ASSERT_TRUE(final_progress.has_value());
    EXPECT_TRUE(final_progress->elite_four_completed);
    EXPECT_TRUE(tournament_manager->isChampionshipUnlocked(player_name));
    
    // Should have required multiple attempts
    EXPECT_GT(total_elite_attempts, 4); // More than minimum
    EXPECT_GT(final_progress->total_elite_four_attempts, 4);
    
    // Elite Four history should show persistence pattern
    auto elite_history = tournament_manager->getPlayerBattleHistory(player_name, "elite_four");
    EXPECT_GT(elite_history.size(), 4); // Multiple attempts recorded
}

// Test 6: Tournament State Persistence During Long Progression
TEST_F(TournamentProgressionScenariosTest, TournamentStatePersistenceDuringLongProgression) {
    std::string player_name = "PersistenceTestTrainer";
    tournament_manager->initializePlayerProgress(player_name);
    
    // Test persistence at various checkpoints
    std::vector<int> checkpoint_positions = {2, 5, 8, 11}; // After gym 3, gym 6, all gyms, Elite Four
    
    for (int checkpoint : checkpoint_positions) {
        bool persistence_success = simulateProgressionPersistence(player_name, checkpoint);
        EXPECT_TRUE(persistence_success) << "Persistence failed at checkpoint " << checkpoint;
    }
    
    // Final verification after all persistence tests
    auto final_progress = tournament_manager->getPlayerProgress(player_name);
    ASSERT_TRUE(final_progress.has_value());
    
    // Tournament data should remain consistent after multiple save/load cycles
    EXPECT_TRUE(tournament_manager->validateTournamentData());
    
    // Progress should be maintainable across system restarts
    EXPECT_GE(final_progress->earned_badges.size(), 3); // At least first checkpoint
}

// Test 7: Boundary Condition Testing - Edge Cases
TEST_F(TournamentProgressionScenariosTest, BoundaryConditionTestingEdgeCases) {
    // Test various edge cases in tournament progression
    
    // Edge Case 1: Attempting Elite Four with exactly 8 badges
    {
        std::string edge_player1 = "EdgeCase_ExactBadges";
        tournament_manager->initializePlayerProgress(edge_player1);
        
        // Award exactly 8 badges
        for (int i = 0; i < 8; ++i) {
            TournamentManager::Badge badge("EdgeGym" + std::to_string(i), "normal", 
                                         "EdgeLeader", "2024-01-15", 1, 75.0);
            tournament_manager->awardBadge(edge_player1, badge);
        }
        
        // Should unlock Elite Four with exactly 8 badges
        EXPECT_TRUE(tournament_manager->isEliteFourUnlocked(edge_player1));
        EXPECT_FALSE(tournament_manager->isChampionshipUnlocked(edge_player1));
    }
    
    // Edge Case 2: Maximum retry attempts
    {
        std::string edge_player2 = "EdgeCase_MaxRetries";
        tournament_manager->initializePlayerProgress(edge_player2);
        
        TournamentManager::TournamentSettings limited_retries = progression_configs["recovery"];
        limited_retries.max_attempts_per_gym = 3; // Limited to 3 attempts
        tournament_manager->setTournamentSettings(limited_retries);
        
        Team retry_team = IntegrationTestUtils::createBalancedTournamentTeam("RetryTeam");
        
        // Simulate failing first gym multiple times
        for (int attempt = 1; attempt <= 3; ++attempt) {
            TournamentManager::TournamentBattleResult failed_attempt;
            failed_attempt.challenge_name = "Test Gym";
            failed_attempt.challenge_type = "gym";
            failed_attempt.victory = (attempt == 3); // Succeed on final attempt
            failed_attempt.performance_score = 40.0 + (attempt * 10.0);
            
            tournament_manager->updatePlayerProgress(edge_player2, failed_attempt);
        }
        
        // Should have recorded all attempts
        auto progress = tournament_manager->getPlayerProgress(edge_player2);
        ASSERT_TRUE(progress.has_value());
        EXPECT_EQ(progress->total_gym_attempts, 3);
    }
    
    // Edge Case 3: Tournament settings changes mid-progression
    {
        std::string edge_player3 = "EdgeCase_SettingsChange";
        tournament_manager->initializePlayerProgress(edge_player3);
        
        // Start with one configuration
        tournament_manager->setTournamentSettings(progression_configs["nonlinear"]);
        
        // Complete some gyms
        for (int i = 0; i < 4; ++i) {
            TournamentManager::Badge badge("ChangeGym" + std::to_string(i), "normal",
                                         "ChangeLeader", "2024-01-15", 1, 80.0);
            tournament_manager->awardBadge(edge_player3, badge);
        }
        
        // Change settings mid-progression
        tournament_manager->setTournamentSettings(progression_configs["strict"]);
        
        // System should handle settings change gracefully
        auto progress = tournament_manager->getPlayerProgress(edge_player3);
        ASSERT_TRUE(progress.has_value());
        EXPECT_EQ(progress->earned_badges.size(), 4);
        
        // Should still be able to continue with new settings
        TournamentManager::Badge final_badge("FinalGym", "normal", "FinalLeader", "2024-01-15", 1, 85.0);
        bool badge_awarded = tournament_manager->awardBadge(edge_player3, final_badge);
        EXPECT_TRUE(badge_awarded);
    }
}

// ================================================================================
// Helper Method Implementations
// ================================================================================

TournamentProgressionScenariosTest::ProgressionResult 
TournamentProgressionScenariosTest::simulateNonLinearProgression(
    const std::string& player_name,
    const TournamentManager::TournamentSettings& config,
    const std::vector<int>& gym_order) {
    
    ProgressionResult result;
    result.completed = false;
    result.total_attempts = 0;
    result.average_performance = 0.0;
    
    auto start_time = std::chrono::steady_clock::now();
    
    Team player_team = IntegrationTestUtils::createBalancedTournamentTeam(player_name + "_Team");
    
    std::vector<std::string> gym_names = {"Pewter", "Cerulean", "Vermilion", "Celadon", 
                                         "Fuchsia", "Saffron", "Cinnabar", "Viridian"};
    std::vector<std::string> gym_types = {"rock", "water", "electric", "grass", 
                                         "poison", "psychic", "fire", "ground"};
    
    double total_performance = 0.0;
    
    // Complete gyms in specified order
    for (int gym_index : gym_order) {
        if (gym_index < 0 || gym_index >= 8) continue;
        
        bool gym_completed = false;
        int attempt = 1;
        
        while (!gym_completed && attempt <= config.max_attempts_per_gym) {
            // Create gym battle
            Team gym_team = test_data_generator->generateGymTeam(gym_types[gym_index], 2);
            
            Battle::AIDifficulty difficulty = (gym_index < 3) ? Battle::AIDifficulty::MEDIUM :
                                             (gym_index < 6) ? Battle::AIDifficulty::HARD :
                                                              Battle::AIDifficulty::EXPERT;
            
            auto battle_result = IntegrationTestUtils::simulateBattle(
                player_team, gym_team, difficulty, gym_names[gym_index]
            );
            
            result.total_attempts++;
            
            // Success based on difficulty and attempt number
            bool success = (rng() % 100) < static_cast<unsigned int>(70 + attempt * 10); // Increasing success rate
            
            if (success) {
                gym_completed = true;
                result.gym_completion_order.push_back(gym_index);
                
                battle_result.victory = true;
                battle_result.performance_score = 70.0 + (rng() % 25);
                total_performance += battle_result.performance_score;
                
                // Award badge
                TournamentManager::Badge badge(gym_names[gym_index] + " Gym", gym_types[gym_index],
                                             gym_names[gym_index] + "Leader", "2024-01-15", 
                                             attempt, battle_result.performance_score);
                tournament_manager->awardBadge(player_name, badge);
                
                // Update tournament progress
                TournamentManager::TournamentBattleResult tournament_result;
                tournament_result.challenge_name = gym_names[gym_index] + " Gym";
                tournament_result.challenge_type = "gym";
                tournament_result.victory = true;
                tournament_result.performance_score = battle_result.performance_score;
                tournament_manager->updatePlayerProgress(player_name, tournament_result);
            }
            
            // Track attempts per challenge
            if (result.attempts_per_challenge.size() <= static_cast<size_t>(gym_index)) {
                result.attempts_per_challenge.resize(gym_index + 1, 0);
            }
            result.attempts_per_challenge[gym_index] = attempt;
            
            attempt++;
        }
    }
    
    auto end_time = std::chrono::steady_clock::now();
    result.total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    result.completed = (result.gym_completion_order.size() == 8);
    result.average_performance = result.completed ? total_performance / 8.0 : 0.0;
    
    return result;
}

TournamentProgressionScenariosTest::ProgressionResult 
TournamentProgressionScenariosTest::simulateFailureRecoveryProgression(
    const std::string& player_name,
    const FailureScenario& scenario,
    const TournamentManager::TournamentSettings& config) {
    (void)config; // Suppress unused parameter warning
    
    ProgressionResult result;
    result.completed = false;
    result.total_attempts = 0;
    result.average_performance = 0.0;
    result.attempts_per_challenge.resize(13, 0); // 8 gyms + 4 Elite Four + 1 Champion
    
    auto start_time = std::chrono::steady_clock::now();
    
    Team player_team = IntegrationTestUtils::createBalancedTournamentTeam(player_name + "_Team");
    
    double total_performance = 0.0;
    int successful_challenges = 0;
    
    // Simulate 8 gym battles
    for (int i = 0; i < 8; ++i) {
        auto gym_result = simulateGymBattle(player_name, player_team, i, scenario);
        
        result.total_attempts += gym_result.turns_taken; // Using turns as attempt proxy
        result.attempts_per_challenge[i] = gym_result.turns_taken;
        
        if (gym_result.victory) {
            result.gym_completion_order.push_back(i);
            total_performance += gym_result.performance_score;
            successful_challenges++;
        }
    }
    
    auto end_time = std::chrono::steady_clock::now();
    result.total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    result.completed = (result.gym_completion_order.size() == 8);
    result.average_performance = successful_challenges > 0 ? total_performance / successful_challenges : 0.0;
    
    return result;
}

IntegrationTestUtils::IntegrationBattleResult 
TournamentProgressionScenariosTest::simulateGymBattle(
    const std::string& player_name,
    Team& player_team,
    int gym_index,
    const FailureScenario& scenario,
    int attempt_number) {
    
    std::vector<std::string> gym_types = {"rock", "water", "electric", "grass", 
                                         "poison", "psychic", "fire", "ground"};
    std::vector<std::string> gym_names = {"Pewter", "Cerulean", "Vermilion", "Celadon",
                                         "Fuchsia", "Saffron", "Cinnabar", "Viridian"};
    
    Team gym_team = test_data_generator->generateGymTeam(gym_types[gym_index], 2);
    
    Battle::AIDifficulty difficulty = (gym_index < 3) ? Battle::AIDifficulty::MEDIUM :
                                     (gym_index < 6) ? Battle::AIDifficulty::HARD :
                                                      Battle::AIDifficulty::EXPERT;
    
    auto battle_result = IntegrationTestUtils::simulateBattle(
        player_team, gym_team, difficulty, gym_names[gym_index] + "_Attempt" + std::to_string(attempt_number)
    );
    
    // Determine success based on failure scenario
    bool is_failure_point = std::find(scenario.failure_points.begin(), 
                                     scenario.failure_points.end(), gym_index) != scenario.failure_points.end();
    
    if (is_failure_point) {
        double failure_chance = scenario.failure_probability;
        
        // Adjust failure chance based on recovery pattern and attempt number
        if (scenario.recovery_pattern == "gradual") {
            failure_chance *= std::max(0.1, 1.0 - (attempt_number * 0.2)); // Gradual improvement
        } else if (scenario.recovery_pattern == "quick") {
            failure_chance *= (attempt_number == 1) ? 1.0 : 0.3; // Quick recovery after first failure
        } else if (scenario.recovery_pattern == "persistent") {
            failure_chance *= std::max(0.2, 1.0 - (attempt_number * 0.1)); // Slow improvement
        }
        
        if ((rng() % 100) < (failure_chance * 100)) {
            battle_result.victory = false;
            battle_result.performance_score = 30.0 + (rng() % 25); // Poor performance
        } else {
            battle_result.victory = true;
            battle_result.performance_score = 65.0 + (rng() % 25); // Recovery performance
        }
    } else {
        // Non-failure points should generally succeed
        battle_result.victory = (rng() % 100) < 85; // 85% success rate
        battle_result.performance_score = battle_result.victory ? 75.0 + (rng() % 20) : 40.0 + (rng() % 20);
    }
    
    // Update tournament progress
    TournamentManager::TournamentBattleResult tournament_result;
    tournament_result.challenge_name = gym_names[gym_index] + " Gym";
    tournament_result.challenge_type = "gym";
    tournament_result.victory = battle_result.victory;
    tournament_result.performance_score = battle_result.performance_score;
    tournament_result.turns_taken = battle_result.turns_taken;
    
    tournament_manager->updatePlayerProgress(player_name, tournament_result);
    
    if (battle_result.victory) {
        TournamentManager::Badge badge(gym_names[gym_index] + " Gym", gym_types[gym_index],
                                     gym_names[gym_index] + "Leader", "2024-01-15",
                                     attempt_number, battle_result.performance_score);
        tournament_manager->awardBadge(player_name, badge);
    }
    
    return battle_result;
}

bool TournamentProgressionScenariosTest::validateNonLinearProgressionOrder(
    const std::vector<int>& completion_order,
    const std::vector<int>& expected_order) {
    
    if (completion_order.size() != expected_order.size()) {
        return false;
    }
    
    for (size_t i = 0; i < completion_order.size(); ++i) {
        if (completion_order[i] != expected_order[i]) {
            return false;
        }
    }
    
    return true;
}

bool TournamentProgressionScenariosTest::simulateProgressionPersistence(
    const std::string& player_name,
    int checkpoint_at_challenge) {
    
    try {
        // Progress to checkpoint
        Team test_team = IntegrationTestUtils::createBalancedTournamentTeam(player_name + "_PersistenceTeam");
        
        for (int i = 0; i < checkpoint_at_challenge && i < 8; ++i) {
            TournamentManager::Badge badge("PersistenceGym" + std::to_string(i), "normal",
                                         "PersistenceLeader", "2024-01-15", 1, 80.0);
            tournament_manager->awardBadge(player_name, badge);
        }
        
        // Save state
        if (!tournament_manager->saveTournamentProgress(player_name)) {
            return false;
        }
        
        // Create new tournament manager (simulate restart)
        auto new_pokemon_data = std::make_shared<PokemonData>();
        auto new_team_builder = std::make_shared<TeamBuilder>(new_pokemon_data);
        auto new_tournament_manager = std::make_shared<TournamentManager>(new_pokemon_data, new_team_builder);
        
        // Load state
        if (!new_tournament_manager->loadTournamentProgress(player_name)) {
            return false;
        }
        
        // Verify state restoration
        auto restored_progress = new_tournament_manager->getPlayerProgress(player_name);
        if (!restored_progress.has_value()) {
            return false;
        }
        
        // Verify checkpoint progress matches
        int expected_badges = std::min(checkpoint_at_challenge, 8);
        return static_cast<int>(restored_progress->earned_badges.size()) == expected_badges;
        
    } catch (const std::exception&) {
        return false;
    }
}

std::vector<int> TournamentProgressionScenariosTest::generateRandomGymOrder() {
    std::vector<int> order = {0, 1, 2, 3, 4, 5, 6, 7};
    std::shuffle(order.begin(), order.end(), rng);
    return order;
}