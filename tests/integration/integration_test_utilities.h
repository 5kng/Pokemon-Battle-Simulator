#pragma once

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <unordered_map>
#include <functional>

#include "test_utils.h"
#include "tournament_manager.h"
#include "championship_system.h"
#include "battle.h"
#include "team_builder.h"
#include "pokemon_data.h"
#include "ai_factory.h"
#include "ai_strategy.h"

namespace IntegrationTestUtils {

/**
 * @brief Comprehensive integration test fixture for multi-system testing
 * 
 * Provides common setup for tournament, battle, and AI system integration tests.
 * Includes utilities for team creation, battle simulation, and state verification.
 */
class MultiSystemIntegrationFixture : public TestUtils::BattleTestFixture {
protected:
    void SetUp() override {
        TestUtils::BattleTestFixture::SetUp();
        
        // Initialize core systems
        pokemon_data = std::make_shared<PokemonData>();
        team_builder = std::make_shared<TeamBuilder>(pokemon_data);
        tournament_manager = std::make_shared<TournamentManager>(pokemon_data, team_builder);
        championship_system = std::make_shared<ChampionshipSystem>(pokemon_data, team_builder, tournament_manager);
        ai_factory = std::make_unique<AIFactory>();
        
        // Setup random number generator for consistent testing
        rng.seed(42);  // Fixed seed for reproducible tests
        
        // Initialize test data
        setupDefaultTestData();
    }
    
    void setupDefaultTestData() {
        test_player_name = "IntegrationTestTrainer";
        tournament_manager->initializePlayerProgress(test_player_name);
        
        // Create default balanced team
        createBalancedTestTeam();
        
        // Setup gym leader teams for different difficulty levels
        setupGymLeaderTeams();
        
        // Setup Elite Four and Champion teams
        setupChampionshipTeams();
    }
    
    // Core system instances
    std::shared_ptr<PokemonData> pokemon_data;
    std::shared_ptr<TeamBuilder> team_builder;
    std::shared_ptr<TournamentManager> tournament_manager;
    std::shared_ptr<ChampionshipSystem> championship_system;
    std::unique_ptr<AIFactory> ai_factory;
    
    // Test data
    std::string test_player_name;
    Team default_player_team;
    std::vector<Team> gym_leader_teams;
    std::vector<Team> elite_four_teams;
    Team champion_team;
    
    // Utility members
    std::mt19937 rng;
    
private:
    void createBalancedTestTeam();
    void setupGymLeaderTeams();
    void setupChampionshipTeams();
};

/**
 * @brief Enhanced team state tracking for integration testing
 */
struct IntegrationTeamState {
    std::vector<int> pokemon_hp;
    std::vector<int> pokemon_current_hp;
    std::vector<std::vector<int>> pokemon_move_pp;
    std::vector<StatusCondition> pokemon_status;
    std::vector<std::string> pokemon_names;
    std::chrono::steady_clock::time_point timestamp;
    std::string state_context;  // Description of when this state was captured
    
    IntegrationTeamState() : timestamp(std::chrono::steady_clock::now()) {}
    
    bool operator==(const IntegrationTeamState& other) const {
        return pokemon_hp == other.pokemon_hp && 
               pokemon_current_hp == other.pokemon_current_hp &&
               pokemon_move_pp == other.pokemon_move_pp &&
               pokemon_status == other.pokemon_status &&
               pokemon_names == other.pokemon_names;
    }
    
    // Calculate integrity hash
    size_t calculateHash() const {
        size_t hash = 0;
        for (size_t i = 0; i < pokemon_hp.size(); ++i) {
            hash ^= std::hash<int>{}(pokemon_hp[i]);
            hash ^= std::hash<int>{}(pokemon_current_hp[i]);
            if (i < pokemon_names.size()) {
                hash ^= std::hash<std::string>{}(pokemon_names[i]);
            }
        }
        return hash;
    }
    
    // Check if team is in valid battle state
    bool isValidForBattle() const {
        for (int hp : pokemon_current_hp) {
            if (hp > 0) return true;  // At least one Pokemon alive
        }
        return false;
    }
    
    // Calculate team health percentage
    double calculateHealthPercentage() const {
        if (pokemon_hp.empty()) return 0.0;
        
        int totalMaxHP = 0;
        int totalCurrentHP = 0;
        for (size_t i = 0; i < pokemon_hp.size(); ++i) {
            totalMaxHP += pokemon_hp[i];
            totalCurrentHP += pokemon_current_hp[i];
        }
        
        return totalMaxHP > 0 ? (static_cast<double>(totalCurrentHP) / totalMaxHP) : 0.0;
    }
};

/**
 * @brief Battle simulation result for integration testing
 */
struct IntegrationBattleResult {
    bool victory;
    int turns_taken;
    double performance_score;
    std::string difficulty_level;
    Battle::BattleResult battle_result;
    
    IntegrationTeamState pre_battle_state;
    IntegrationTeamState post_battle_state;
    
    std::vector<std::string> battle_events;
    std::string mvp_pokemon;
    std::chrono::milliseconds battle_duration;
    
    IntegrationBattleResult() 
        : victory(false), turns_taken(0), performance_score(0.0), 
          battle_result(Battle::BattleResult::ONGOING),
          battle_duration(std::chrono::milliseconds(0)) {}
    
    // Calculate battle efficiency (performance per turn)
    double calculateEfficiency() const {
        return turns_taken > 0 ? performance_score / turns_taken : 0.0;
    }
    
    // Check if battle result is consistent
    bool isConsistent() const {
        // Victory should match battle result
        if (victory && battle_result != Battle::BattleResult::PLAYER_WINS) return false;
        if (!victory && battle_result == Battle::BattleResult::PLAYER_WINS) return false;
        
        // Performance should be reasonable
        if (performance_score < 0.0 || performance_score > 100.0) return false;
        
        // Turns should be positive if battle completed
        if (battle_result != Battle::BattleResult::ONGOING && turns_taken <= 0) return false;
        
        return true;
    }
};

/**
 * @brief Tournament progression tracker for integration testing
 */
struct IntegrationTournamentProgress {
    std::string player_name;
    std::vector<std::string> completed_challenges;
    std::vector<IntegrationBattleResult> battle_results;
    std::unordered_map<std::string, IntegrationTeamState> checkpoint_states;
    
    int total_victories;
    int total_defeats;
    double average_performance;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    
    IntegrationTournamentProgress() 
        : total_victories(0), total_defeats(0), average_performance(0.0),
          start_time(std::chrono::steady_clock::now()) {}
    
    // Add battle result and update statistics
    void addBattleResult(const IntegrationBattleResult& result) {
        battle_results.push_back(result);
        
        if (result.victory) {
            total_victories++;
        } else {
            total_defeats++;
        }
        
        // Recalculate average performance
        double total_performance = 0.0;
        for (const auto& battle : battle_results) {
            total_performance += battle.performance_score;
        }
        average_performance = battle_results.empty() ? 0.0 : 
                             total_performance / battle_results.size();
    }
    
    // Check tournament completion
    bool isCompleted() const {
        return completed_challenges.size() >= 13;  // 8 gyms + 4 Elite Four + 1 Champion
    }
    
    // Calculate completion percentage
    double getCompletionPercentage() const {
        return static_cast<double>(completed_challenges.size()) / 13.0;
    }
    
    // Get win rate
    double getWinRate() const {
        int total = total_victories + total_defeats;
        return total > 0 ? static_cast<double>(total_victories) / total : 0.0;
    }
};

// Utility Functions

/**
 * @brief Capture detailed team state for integration testing
 */
IntegrationTeamState captureTeamState(const Team& team, const std::string& context = "");

/**
 * @brief Apply team state for testing restoration
 */
void applyTeamState(Team& team, const IntegrationTeamState& state);

/**
 * @brief Create a Pokemon with specific stats for testing
 */
Pokemon createTestPokemonWithMoves(
    const std::string& name,
    int hp, int attack, int defense, int special_attack, int special_defense, int speed,
    const std::vector<std::string>& types,
    const std::vector<std::pair<std::string, std::string>>& move_pairs  // (name, type) pairs
);

/**
 * @brief Create a balanced team for tournament testing
 */
Team createBalancedTournamentTeam(const std::string& team_name);

/**
 * @brief Create a gym leader team with type specialization
 */
Team createGymLeaderTeam(const std::string& gym_type, Battle::AIDifficulty difficulty);

/**
 * @brief Simulate a battle with detailed tracking
 */
IntegrationBattleResult simulateBattle(
    Team& player_team,
    Team& opponent_team,
    Battle::AIDifficulty ai_difficulty,
    const std::string& context = ""
);

/**
 * @brief Simulate tournament progression through multiple battles
 */
IntegrationTournamentProgress simulateTournamentProgression(
    std::shared_ptr<TournamentManager> tournament_manager,
    const std::string& player_name,
    Team& player_team,
    const std::vector<std::pair<Team, Battle::AIDifficulty>>& opponents
);

/**
 * @brief Validate battle result consistency
 */
bool validateBattleResultConsistency(
    const IntegrationBattleResult& result1,
    const IntegrationBattleResult& result2,
    double tolerance = 0.1
);

/**
 * @brief Validate AI difficulty progression
 */
bool validateAIDifficultyProgression(
    const std::vector<IntegrationBattleResult>& results
);

/**
 * @brief Apply realistic battle damage based on difficulty
 */
void applyBattleDamage(
    Team& team,
    Battle::AIDifficulty difficulty,
    std::mt19937& rng
);

/**
 * @brief Heal team according to tournament rules
 */
void applyTournamentHealing(
    Team& team,
    const std::string& healing_type = "full"  // "full", "partial", "none"
);

/**
 * @brief Generate performance score based on battle parameters
 */
double calculatePerformanceScore(
    const IntegrationTeamState& pre_battle,
    const IntegrationTeamState& post_battle,
    int turns_taken,
    Battle::AIDifficulty difficulty
);

/**
 * @brief Validate tournament progress integrity
 */
bool validateTournamentProgressIntegrity(
    const IntegrationTournamentProgress& progress
);

/**
 * @brief Create AI decision analysis data
 */
struct AIDecisionAnalysis {
    std::vector<int> move_choices;
    std::vector<bool> switch_decisions;
    std::vector<double> decision_scores;
    std::vector<std::string> decision_reasoning;
    double consistency_score;
    double strategic_depth;
    
    AIDecisionAnalysis() : consistency_score(0.0), strategic_depth(0.0) {}
};

/**
 * @brief Analyze AI decision patterns for consistency testing
 */
AIDecisionAnalysis analyzeAIDecisions(
    std::unique_ptr<AIStrategy>& ai,
    const BattleState& battle_state,
    int num_decisions = 10
);

/**
 * @brief Validate AI decision consistency across multiple runs
 */
bool validateAIConsistency(
    const std::vector<AIDecisionAnalysis>& analyses,
    double min_consistency = 0.7
);

/**
 * @brief Tournament stage definitions for testing
 */
enum class TournamentStage {
    EARLY_GYM,      // Gyms 1-2 (Easy AI)
    MID_GYM,        // Gyms 3-5 (Medium AI)
    LATE_GYM,       // Gyms 6-8 (Hard AI)
    ELITE_FOUR,     // Elite Four (Expert AI)
    CHAMPION        // Champion (Expert AI)
};

/**
 * @brief Get expected AI difficulty for tournament stage
 */
Battle::AIDifficulty getStageAIDifficulty(TournamentStage stage);

/**
 * @brief Get expected performance threshold for tournament stage
 */
double getStagePerformanceThreshold(TournamentStage stage);

/**
 * @brief Comprehensive integration test data generator
 */
class IntegrationTestDataGenerator {
public:
    IntegrationTestDataGenerator(std::mt19937& rng) : rng_(rng) {}
    
    // Generate varied Pokemon teams
    Team generateRandomTeam(const std::string& team_name, int team_size = 3);
    
    // Generate type-specialized gym team
    Team generateGymTeam(const std::string& type, int team_size = 2);
    
    // Generate Elite Four team
    Team generateEliteFourTeam(const std::string& specialization, int team_size = 2);
    
    // Generate Champion team
    Team generateChampionTeam(int team_size = 3);
    
    // Generate tournament battle sequence
    std::vector<std::pair<Team, Battle::AIDifficulty>> generateTournamentSequence();
    
private:
    std::mt19937& rng_;
    
    Pokemon generateRandomPokemon(const std::string& name_prefix, 
                                 const std::vector<std::string>& types,
                                 int min_stat = 50, int max_stat = 150);
};

/**
 * @brief Integration test result validator
 */
class IntegrationTestValidator {
public:
    // Validate battle system integration
    static bool validateBattleIntegration(
        const std::vector<IntegrationBattleResult>& results
    );
    
    // Validate tournament system integration
    static bool validateTournamentIntegration(
        const IntegrationTournamentProgress& progress
    );
    
    // Validate AI system integration
    static bool validateAIIntegration(
        const std::vector<AIDecisionAnalysis>& ai_analyses
    );
    
    // Validate championship system integration
    static bool validateChampionshipIntegration(
        const std::vector<IntegrationBattleResult>& elite_four_results,
        const IntegrationBattleResult& champion_result
    );
    
    // Validate team state persistence
    static bool validateTeamStatePersistence(
        const std::vector<IntegrationTeamState>& state_sequence
    );
    
private:
    static bool isWithinTolerance(double value1, double value2, double tolerance);
    static bool hasValidProgression(const std::vector<double>& values);
};

} // namespace IntegrationTestUtils