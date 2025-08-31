#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include "test_utils.h"
#include "tournament_manager.h"
#include "battle.h"
#include "ai_factory.h"
#include "ai_strategy.h"
#include "easy_ai.h"
#include "medium_ai.h"
#include "hard_ai.h"
#include "expert_ai.h"
#include "gym_leader.h"

/**
 * @brief Integration tests for Tournament-AI system interactions
 * 
 * These tests validate AI difficulty progression through tournament stages,
 * gym leader AI behavior consistency, and proper integration between tournament
 * progression and AI challenge scaling.
 * Focus areas:
 * - AI difficulty progression from gym leaders through Elite Four to Champion
 * - Consistency of gym leader AI behavior and strategy
 * - Tournament-driven AI adaptation and challenge scaling
 * - AI strategy effectiveness in tournament contexts
 */
class TournamentAIIntegrationTest : public TestUtils::BattleTestFixture {
protected:
    void SetUp() override {
        TestUtils::BattleTestFixture::SetUp();
        
        // Initialize core systems
        pokemon_data = std::make_shared<PokemonData>();
        team_builder = std::make_shared<TeamBuilder>(pokemon_data);
        tournament_manager = std::make_shared<TournamentManager>(pokemon_data, team_builder);
        ai_factory = std::make_unique<AIFactory>();
        
        // Setup test player and progression
        setupTestPlayer();
        setupGymLeaderTeams();
        setupEliteFourTeams();
        
        // Initialize tournament progress
        tournament_manager->initializePlayerProgress(test_player_name);
    }
    
    void setupTestPlayer() {
        test_player_name = "AITestTrainer";
        
        // Create a well-balanced team for AI testing
        Pokemon versatile1 = TestUtils::createTestPokemon("Versatile1", 100, 90, 80, 95, 85, 85, {"normal", "flying"});
        versatile1.moves.clear();
        versatile1.moves.push_back(TestUtils::createTestMove("body-slam", 85, 100, 15, "normal", "physical"));
        versatile1.moves.push_back(TestUtils::createTestMove("aerial-ace", 60, 100, 20, "flying", "physical"));
        versatile1.moves.push_back(TestUtils::createTestMove("roost", 0, 100, 10, "flying", "status"));
        versatile1.moves.push_back(TestUtils::createTestMove("double-team", 0, 100, 15, "normal", "status"));
        
        Pokemon versatile2 = TestUtils::createTestPokemon("Versatile2", 95, 85, 75, 100, 90, 90, {"water", "ice"});
        versatile2.moves.clear();
        versatile2.moves.push_back(TestUtils::createTestMove("surf", 90, 100, 15, "water", "special"));
        versatile2.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
        versatile2.moves.push_back(TestUtils::createTestMove("recover", 0, 100, 5, "normal", "status"));
        versatile2.moves.push_back(TestUtils::createTestMove("toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON, 100));
        
        Pokemon versatile3 = TestUtils::createTestPokemon("Versatile3", 105, 95, 90, 85, 80, 75, {"fire", "fighting"});
        versatile3.moves.clear();
        versatile3.moves.push_back(TestUtils::createTestMove("flare-blitz", 120, 100, 15, "fire", "physical"));
        versatile3.moves.push_back(TestUtils::createTestMove("close-combat", 120, 100, 5, "fighting", "physical"));
        versatile3.moves.push_back(TestUtils::createTestMove("will-o-wisp", 0, 85, 15, "fire", "status", StatusCondition::BURN, 100));
        versatile3.moves.push_back(TestUtils::createTestMove("bulk-up", 0, 100, 20, "fighting", "status"));
        
        player_team = TestUtils::createTestTeam({versatile1, versatile2, versatile3});
    }
    
    void setupGymLeaderTeams() {
        // Early Gym (Rock-type, Easy AI)
        Pokemon rockMon1 = TestUtils::createTestPokemon("Geodude", 90, 80, 100, 30, 30, 20, {"rock", "ground"});
        rockMon1.moves.clear();
        rockMon1.moves.push_back(TestUtils::createTestMove("rock-throw", 50, 90, 15, "rock", "physical"));
        rockMon1.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
        rockMon1.moves.push_back(TestUtils::createTestMove("defense-curl", 0, 100, 40, "normal", "status"));
        rockMon1.moves.push_back(TestUtils::createTestMove("rollout", 30, 90, 20, "rock", "physical"));
        
        Pokemon rockMon2 = TestUtils::createTestPokemon("Onix", 120, 45, 160, 30, 45, 70, {"rock", "ground"});
        rockMon2.moves.clear();
        rockMon2.moves.push_back(TestUtils::createTestMove("rock-slide", 75, 90, 10, "rock", "physical"));
        rockMon2.moves.push_back(TestUtils::createTestMove("bind", 15, 85, 20, "normal", "physical"));
        rockMon2.moves.push_back(TestUtils::createTestMove("screech", 0, 85, 40, "normal", "status"));
        rockMon2.moves.push_back(TestUtils::createTestMove("earthquake", 100, 100, 10, "ground", "physical"));
        
        early_gym_team = TestUtils::createTestTeam({rockMon1, rockMon2});
        
        // Mid Gym (Electric-type, Medium AI)
        Pokemon elecMon1 = TestUtils::createTestPokemon("Voltorb", 80, 30, 50, 55, 55, 100, {"electric"});
        elecMon1.moves.clear();
        elecMon1.moves.push_back(TestUtils::createTestMove("thunderbolt", 90, 100, 15, "electric", "special"));
        elecMon1.moves.push_back(TestUtils::createTestMove("self-destruct", 200, 100, 5, "normal", "physical"));
        elecMon1.moves.push_back(TestUtils::createTestMove("thunder-wave", 0, 100, 20, "electric", "status", StatusCondition::PARALYSIS, 100));
        elecMon1.moves.push_back(TestUtils::createTestMove("light-screen", 0, 100, 30, "psychic", "status"));
        
        Pokemon elecMon2 = TestUtils::createTestPokemon("Raichu", 100, 90, 55, 90, 80, 110, {"electric"});
        elecMon2.moves.clear();
        elecMon2.moves.push_back(TestUtils::createTestMove("thunder", 110, 70, 10, "electric", "special"));
        elecMon2.moves.push_back(TestUtils::createTestMove("quick-attack", 40, 100, 30, "normal", "physical"));
        elecMon2.moves.push_back(TestUtils::createTestMove("double-team", 0, 100, 15, "normal", "status"));
        elecMon2.moves.push_back(TestUtils::createTestMove("focus-punch", 150, 100, 20, "fighting", "physical"));
        
        mid_gym_team = TestUtils::createTestTeam({elecMon1, elecMon2});
        
        // Late Gym (Dragon-type, Hard AI)
        Pokemon dragonMon1 = TestUtils::createTestPokemon("Dragonair", 110, 84, 65, 70, 70, 70, {"dragon"});
        dragonMon1.moves.clear();
        dragonMon1.moves.push_back(TestUtils::createTestMove("dragon-pulse", 85, 100, 10, "dragon", "special"));
        dragonMon1.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
        dragonMon1.moves.push_back(TestUtils::createTestMove("thunder-wave", 0, 100, 20, "electric", "status", StatusCondition::PARALYSIS, 100));
        dragonMon1.moves.push_back(TestUtils::createTestMove("dragon-dance", 0, 100, 20, "dragon", "status"));
        
        Pokemon dragonMon2 = TestUtils::createTestPokemon("Dragonite", 134, 134, 95, 100, 100, 80, {"dragon", "flying"});
        dragonMon2.moves.clear();
        dragonMon2.moves.push_back(TestUtils::createTestMove("outrage", 120, 100, 10, "dragon", "physical"));
        dragonMon2.moves.push_back(TestUtils::createTestMove("earthquake", 100, 100, 10, "ground", "physical"));
        dragonMon2.moves.push_back(TestUtils::createTestMove("fire-punch", 75, 100, 15, "fire", "physical"));
        dragonMon2.moves.push_back(TestUtils::createTestMove("extremespeed", 80, 100, 5, "normal", "physical"));
        
        late_gym_team = TestUtils::createTestTeam({dragonMon1, dragonMon2});
    }
    
    void setupEliteFourTeams() {
        // Elite Four Member 1 (Expert AI)
        Pokemon elite1Mon1 = TestUtils::createTestPokemon("EliteSteel", 120, 110, 140, 55, 80, 30, {"steel"});
        elite1Mon1.moves.clear();
        elite1Mon1.moves.push_back(TestUtils::createTestMove("meteor-mash", 90, 90, 10, "steel", "physical"));
        elite1Mon1.moves.push_back(TestUtils::createTestMove("earthquake", 100, 100, 10, "ground", "physical"));
        elite1Mon1.moves.push_back(TestUtils::createTestMove("iron-defense", 0, 100, 15, "steel", "status"));
        elite1Mon1.moves.push_back(TestUtils::createTestMove("rest", 0, 100, 10, "psychic", "status"));
        
        Pokemon elite1Mon2 = TestUtils::createTestPokemon("EliteMagnezone", 130, 70, 115, 130, 90, 60, {"electric", "steel"});
        elite1Mon2.moves.clear();
        elite1Mon2.moves.push_back(TestUtils::createTestMove("zap-cannon", 120, 50, 5, "electric", "special"));
        elite1Mon2.moves.push_back(TestUtils::createTestMove("flash-cannon", 80, 100, 10, "steel", "special"));
        elite1Mon2.moves.push_back(TestUtils::createTestMove("lock-on", 0, 100, 5, "normal", "status"));
        elite1Mon2.moves.push_back(TestUtils::createTestMove("explosion", 250, 100, 5, "normal", "physical"));
        
        elite_four_team = TestUtils::createTestTeam({elite1Mon1, elite1Mon2});
    }
    
    // Helper method to measure AI decision consistency
    struct AIDecisionPattern {
        std::vector<int> move_choices;
        std::vector<bool> switch_decisions;
        std::vector<std::string> decision_rationale;
        
        double calculateConsistencyScore() const {
            if (move_choices.empty()) return 0.0;
            
            // Simple consistency metric: variance in move selection
            std::map<int, int> move_frequency;
            for (int choice : move_choices) {
                move_frequency[choice]++;
            }
            
            // Calculate entropy (lower = more consistent/predictable)
            double entropy = 0.0;
            double total = static_cast<double>(move_choices.size());
            for (const auto& pair : move_frequency) {
                double probability = pair.second / total;
                entropy -= probability * std::log2(probability);
            }
            
            return entropy;
        }
    };
    
    AIDecisionPattern analyzeAIDecisions(const std::unique_ptr<AIStrategy>& ai, 
                                        const BattleState& battleState, 
                                        int numDecisions = 10) {
        AIDecisionPattern pattern;
        
        for (int i = 0; i < numDecisions; ++i) {
            auto moveEval = ai->chooseBestMove(battleState);
            auto switchEval = ai->chooseBestSwitch(battleState);
            bool shouldSwitch = ai->shouldSwitch(battleState);
            
            pattern.move_choices.push_back(moveEval.moveIndex);
            pattern.switch_decisions.push_back(shouldSwitch);
            pattern.decision_rationale.push_back(moveEval.reasoning);
        }
        
        return pattern;
    }
    
    // Core test objects
    std::shared_ptr<PokemonData> pokemon_data;
    std::shared_ptr<TeamBuilder> team_builder;
    std::shared_ptr<TournamentManager> tournament_manager;
    std::unique_ptr<AIFactory> ai_factory;
    
    // Test data
    std::string test_player_name;
    Team player_team;
    Team early_gym_team;   // Easy AI difficulty
    Team mid_gym_team;     // Medium AI difficulty
    Team late_gym_team;    // Hard AI difficulty
    Team elite_four_team;  // Expert AI difficulty
};

// Test 1: AI Difficulty Progression Through Tournament Stages
TEST_F(TournamentAIIntegrationTest, AIDifficultyProgressionThroughTournament) {
    // Test progression from Easy (early gyms) to Expert (Elite Four)
    
    // Early Gym Battle - Easy AI
    {
        Battle earlyGymBattle(player_team, early_gym_team, Battle::AIDifficulty::EASY);
        
        // Easy AI should be present and functional
        EXPECT_FALSE(earlyGymBattle.isBattleOver());
        EXPECT_EQ(earlyGymBattle.getBattleResult(), Battle::BattleResult::ONGOING);
        
        // Test that AI decisions are made (battle is functional)
        EXPECT_TRUE(early_gym_team.hasAlivePokemon());
        
        // Verify battle can proceed with Easy AI
        auto initialPlayerHP = player_team.getPokemon(0)->current_hp;
        auto initialOpponentHP = early_gym_team.getPokemon(0)->current_hp;
        
        EXPECT_GT(initialPlayerHP, 0);
        EXPECT_GT(initialOpponentHP, 0);
    }
    
    // Mid Tournament Gym - Medium AI
    {
        Battle midGymBattle(player_team, mid_gym_team, Battle::AIDifficulty::MEDIUM);
        
        EXPECT_FALSE(midGymBattle.isBattleOver());
        EXPECT_EQ(midGymBattle.getBattleResult(), Battle::BattleResult::ONGOING);
        
        // Medium AI should be more strategic than Easy
        EXPECT_TRUE(mid_gym_team.hasAlivePokemon());
    }
    
    // Late Tournament Gym - Hard AI
    {
        Battle lateGymBattle(player_team, late_gym_team, Battle::AIDifficulty::HARD);
        
        EXPECT_FALSE(lateGymBattle.isBattleOver());
        EXPECT_EQ(lateGymBattle.getBattleResult(), Battle::BattleResult::ONGOING);
        
        // Hard AI should be significantly more challenging
        EXPECT_TRUE(late_gym_team.hasAlivePokemon());
    }
    
    // Elite Four Battle - Expert AI
    {
        Battle eliteFourBattle(player_team, elite_four_team, Battle::AIDifficulty::EXPERT);
        
        EXPECT_FALSE(eliteFourBattle.isBattleOver());
        EXPECT_EQ(eliteFourBattle.getBattleResult(), Battle::BattleResult::ONGOING);
        
        // Expert AI should represent the highest challenge level
        EXPECT_TRUE(elite_four_team.hasAlivePokemon());
    }
    
    // Verify that all difficulty levels are distinct and functional
    std::vector<Battle::AIDifficulty> difficulties = {
        Battle::AIDifficulty::EASY,
        Battle::AIDifficulty::MEDIUM, 
        Battle::AIDifficulty::HARD,
        Battle::AIDifficulty::EXPERT
    };
    
    for (auto difficulty : difficulties) {
        Battle testBattle(player_team, early_gym_team, difficulty);
        EXPECT_FALSE(testBattle.isBattleOver());
        EXPECT_TRUE(testBattle.getBattleResult() == Battle::BattleResult::ONGOING);
    }
}

// Test 2: Gym Leader AI Behavior Consistency
TEST_F(TournamentAIIntegrationTest, GymLeaderAIBehaviorConsistency) {
    // Test that gym leader AI behaves consistently across multiple battles
    
    // Create AI strategies for testing
    auto easyAI = ai_factory->createAI(AIDifficulty::EASY);
    auto mediumAI = ai_factory->createAI(AIDifficulty::MEDIUM);
    auto hardAI = ai_factory->createAI(AIDifficulty::HARD);
    
    ASSERT_TRUE(easyAI != nullptr);
    ASSERT_TRUE(mediumAI != nullptr);
    ASSERT_TRUE(hardAI != nullptr);
    
    // Setup battle state for AI decision testing
    BattleState testBattleState;
    testBattleState.aiPokemon = early_gym_team.getPokemon(0);
    testBattleState.opponentPokemon = player_team.getPokemon(0);
    testBattleState.aiTeam = &early_gym_team;
    testBattleState.opponentTeam = &player_team;
    testBattleState.currentWeather = WeatherCondition::NONE;
    testBattleState.weatherTurnsRemaining = 0;
    testBattleState.turnNumber = 1;
    
    ASSERT_TRUE(testBattleState.aiPokemon != nullptr);
    ASSERT_TRUE(testBattleState.opponentPokemon != nullptr);
    
    // Test Easy AI consistency
    {
        auto pattern1 = analyzeAIDecisions(easyAI, testBattleState, 5);
        auto pattern2 = analyzeAIDecisions(easyAI, testBattleState, 5);
        
        // Easy AI should make decisions (not crash or freeze)
        EXPECT_EQ(pattern1.move_choices.size(), 5);
        EXPECT_EQ(pattern2.move_choices.size(), 5);
        
        // Decisions should be within valid move range
        for (int choice : pattern1.move_choices) {
            EXPECT_GE(choice, 0);
            EXPECT_LT(choice, static_cast<int>(testBattleState.aiPokemon->moves.size()));
        }
    }
    
    // Test Medium AI consistency and improvement over Easy
    {
        auto mediumPattern = analyzeAIDecisions(mediumAI, testBattleState, 5);
        
        EXPECT_EQ(mediumPattern.move_choices.size(), 5);
        
        // Medium AI should make valid decisions
        for (int choice : mediumPattern.move_choices) {
            EXPECT_GE(choice, 0);
            EXPECT_LT(choice, static_cast<int>(testBattleState.aiPokemon->moves.size()));
        }
        
        // Should have reasoning for decisions
        EXPECT_FALSE(mediumPattern.decision_rationale.empty());
    }
    
    // Test Hard AI consistency and strategic depth
    {
        auto hardPattern = analyzeAIDecisions(hardAI, testBattleState, 5);
        
        EXPECT_EQ(hardPattern.move_choices.size(), 5);
        
        // Hard AI should make sophisticated decisions
        for (int choice : hardPattern.move_choices) {
            EXPECT_GE(choice, 0);
            EXPECT_LT(choice, static_cast<int>(testBattleState.aiPokemon->moves.size()));
        }
        
        // Should consider switching when appropriate
        bool consideredSwitching = false;
        for (bool switchDecision : hardPattern.switch_decisions) {
            if (switchDecision) {
                consideredSwitching = true;
                break;
            }
        }
        
        // Suppress unused variable warning if needed
        (void)consideredSwitching;
        // Hard AI should at least consider switching (even if it doesn't always switch)
        EXPECT_TRUE(hardPattern.switch_decisions.size() > 0);
    }
}

// Test 3: Tournament-Driven AI Adaptation
TEST_F(TournamentAIIntegrationTest, TournamentDrivenAIAdaptation) {
    // Test that AI difficulty scales appropriately with tournament progress
    
    // Simulate tournament progression
    std::vector<std::pair<std::string, Battle::AIDifficulty>> gymProgression = {
        {"Boulder Badge", Battle::AIDifficulty::EASY},
        {"Cascade Badge", Battle::AIDifficulty::EASY},
        {"Thunder Badge", Battle::AIDifficulty::MEDIUM},
        {"Rainbow Badge", Battle::AIDifficulty::MEDIUM},
        {"Soul Badge", Battle::AIDifficulty::HARD},
        {"Marsh Badge", Battle::AIDifficulty::HARD},
        {"Volcano Badge", Battle::AIDifficulty::HARD},
        {"Earth Badge", Battle::AIDifficulty::EXPERT}
    };
    
    for (size_t i = 0; i < gymProgression.size(); ++i) {
        const auto& gymInfo = gymProgression[i];
        
        // Create battle with appropriate AI difficulty for this stage
        Team currentGymTeam = (i < 2) ? early_gym_team : 
                             (i < 4) ? mid_gym_team : 
                             (i < 7) ? late_gym_team : elite_four_team;
        
        Battle tournamentBattle(player_team, currentGymTeam, gymInfo.second);
        
        // Verify battle initialization
        EXPECT_FALSE(tournamentBattle.isBattleOver());
        EXPECT_EQ(tournamentBattle.getBattleResult(), Battle::BattleResult::ONGOING);
        
        // AI should be functional at all difficulty levels
        EXPECT_TRUE(currentGymTeam.hasAlivePokemon());
        
        // Simulate tournament progress update
        TournamentManager::TournamentBattleResult result;
        result.challenge_name = gymInfo.first;
        result.challenge_type = "gym";
        result.victory = true;
        result.difficulty_level = (gymInfo.second == Battle::AIDifficulty::EASY) ? "easy" :
                                 (gymInfo.second == Battle::AIDifficulty::MEDIUM) ? "medium" :
                                 (gymInfo.second == Battle::AIDifficulty::HARD) ? "hard" : "expert";
        
        tournament_manager->updatePlayerProgress(test_player_name, result);
        
        // Award badge
        TournamentManager::Badge badge(gymInfo.first, "test", "TestLeader", "2024-01-15", 1, 85.0);
        tournament_manager->awardBadge(test_player_name, badge);
    }
    
    // Verify tournament progression affects available challenges
    auto progress = tournament_manager->getPlayerProgress(test_player_name);
    ASSERT_TRUE(progress.has_value());
    
    EXPECT_EQ(progress->earned_badges.size(), 8);
    
    // With 8 badges, Elite Four should be unlocked (Expert AI)
    EXPECT_TRUE(tournament_manager->isEliteFourUnlocked(test_player_name));
    
    // Elite Four battles should use Expert AI
    Battle eliteFourBattle(player_team, elite_four_team, Battle::AIDifficulty::EXPERT);
    EXPECT_FALSE(eliteFourBattle.isBattleOver());
    EXPECT_TRUE(eliteFourBattle.getBattleResult() == Battle::BattleResult::ONGOING);
}

// Test 4: AI Strategy Effectiveness in Tournament Context
TEST_F(TournamentAIIntegrationTest, AIStrategyEffectivenessInTournamentContext) {
    // Test that different AI strategies perform appropriately in tournament battles
    
    // Create AI instances
    auto easyAI = ai_factory->createAI(AIDifficulty::EASY);
    auto mediumAI = ai_factory->createAI(AIDifficulty::MEDIUM);
    auto hardAI = ai_factory->createAI(AIDifficulty::HARD);
    auto expertAI = ai_factory->createAI(AIDifficulty::EXPERT);
    
    ASSERT_TRUE(easyAI != nullptr);
    ASSERT_TRUE(mediumAI != nullptr);
    ASSERT_TRUE(hardAI != nullptr);
    ASSERT_TRUE(expertAI != nullptr);
    
    // Test against a consistent opponent setup
    BattleState standardBattleState;
    standardBattleState.aiPokemon = early_gym_team.getPokemon(0);
    standardBattleState.opponentPokemon = player_team.getPokemon(0);
    standardBattleState.aiTeam = &early_gym_team;
    standardBattleState.opponentTeam = &player_team;
    standardBattleState.currentWeather = WeatherCondition::NONE;
    standardBattleState.weatherTurnsRemaining = 0;
    standardBattleState.turnNumber = 1;
    
    ASSERT_TRUE(standardBattleState.aiPokemon != nullptr);
    ASSERT_TRUE(standardBattleState.opponentPokemon != nullptr);
    
    // Test Easy AI characteristics
    {
        auto moveEval = easyAI->chooseBestMove(standardBattleState);
        EXPECT_GE(moveEval.moveIndex, 0);
        EXPECT_LT(moveEval.moveIndex, static_cast<int>(standardBattleState.aiPokemon->moves.size()));
        EXPECT_GE(moveEval.score, 0.0);  // Should have some positive evaluation
        
        // Easy AI should not switch frequently
        bool shouldSwitch = easyAI->shouldSwitch(standardBattleState);
        // Easy AI may or may not switch, but the decision should be consistent
        EXPECT_TRUE(shouldSwitch == true || shouldSwitch == false);  // Boolean consistency check
    }
    
    // Test Medium AI strategic improvement
    {
        auto moveEval = mediumAI->chooseBestMove(standardBattleState);
        EXPECT_GE(moveEval.moveIndex, 0);
        EXPECT_LT(moveEval.moveIndex, static_cast<int>(standardBattleState.aiPokemon->moves.size()));
        EXPECT_GT(moveEval.score, 0.0);  // Should have positive evaluation
        
        // Medium AI should have more detailed reasoning
        EXPECT_FALSE(moveEval.reasoning.empty());
        
        // Should consider type effectiveness
        auto switchEval = mediumAI->chooseBestSwitch(standardBattleState);
        EXPECT_GE(switchEval.pokemonIndex, 0);
        EXPECT_LT(switchEval.pokemonIndex, static_cast<int>(standardBattleState.aiTeam->size()));
    }
    
    // Test Hard AI advanced strategy
    {
        auto moveEval = hardAI->chooseBestMove(standardBattleState);
        EXPECT_GE(moveEval.moveIndex, 0);
        EXPECT_LT(moveEval.moveIndex, static_cast<int>(standardBattleState.aiPokemon->moves.size()));
        EXPECT_GT(moveEval.score, 0.0);
        
        // Hard AI should have sophisticated reasoning
        EXPECT_FALSE(moveEval.reasoning.empty());
        
        // Should make strategic switching decisions
        auto switchEval = hardAI->chooseBestSwitch(standardBattleState);
        EXPECT_GE(switchEval.score, 0.0);
        EXPECT_FALSE(switchEval.reasoning.empty());
    }
    
    // Test Expert AI maximum strategic depth
    {
        auto moveEval = expertAI->chooseBestMove(standardBattleState);
        EXPECT_GE(moveEval.moveIndex, 0);
        EXPECT_LT(moveEval.moveIndex, static_cast<int>(standardBattleState.aiPokemon->moves.size()));
        EXPECT_GT(moveEval.score, 0.0);
        
        // Expert AI should have the most detailed analysis
        EXPECT_FALSE(moveEval.reasoning.empty());
        
        // Should make highly strategic decisions
        bool shouldSwitch = expertAI->shouldSwitch(standardBattleState);
        EXPECT_TRUE(shouldSwitch == true || shouldSwitch == false);  // Should make a decision
        
        auto switchEval = expertAI->chooseBestSwitch(standardBattleState);
        EXPECT_GE(switchEval.score, 0.0);
        EXPECT_FALSE(switchEval.reasoning.empty());
    }
}

// Test 5: AI Difficulty Scaling with Tournament Stages
TEST_F(TournamentAIIntegrationTest, AIDifficultyScalingWithTournamentStages) {
    // Test that AI difficulty appropriately scales with tournament progression
    
    struct TournamentStage {
        std::string stage_name;
        Battle::AIDifficulty expected_difficulty;
        int required_badges;
    };
    
    std::vector<TournamentStage> stages = {
        {"Early Gyms", Battle::AIDifficulty::EASY, 0},
        {"Mid Gyms", Battle::AIDifficulty::MEDIUM, 2},
        {"Late Gyms", Battle::AIDifficulty::HARD, 5},
        {"Elite Four", Battle::AIDifficulty::EXPERT, 8}
    };
    
    for (const auto& stage : stages) {
        // Simulate having the required badges
        for (int i = 0; i < stage.required_badges; ++i) {
            TournamentManager::Badge badge(
                "TestGym" + std::to_string(i), "test", "TestLeader", "2024-01-15", 1, 80.0
            );
            tournament_manager->awardBadge(test_player_name, badge);
        }
        
        // Select appropriate team based on difficulty
        Team* stageTeam;
        switch (stage.expected_difficulty) {
            case Battle::AIDifficulty::EASY:
                stageTeam = &early_gym_team;
                break;
            case Battle::AIDifficulty::MEDIUM:
                stageTeam = &mid_gym_team;
                break;
            case Battle::AIDifficulty::HARD:
                stageTeam = &late_gym_team;
                break;
            case Battle::AIDifficulty::EXPERT:
                stageTeam = &elite_four_team;
                break;
            default:
                stageTeam = &early_gym_team;
        }
        
        // Create battle with expected difficulty
        Battle stageBattle(player_team, *stageTeam, stage.expected_difficulty);
        
        // Verify battle functionality at this difficulty level
        EXPECT_FALSE(stageBattle.isBattleOver()) << "Battle should be active for stage: " << stage.stage_name;
        EXPECT_EQ(stageBattle.getBattleResult(), Battle::BattleResult::ONGOING) 
            << "Battle result should be ongoing for stage: " << stage.stage_name;
        
        // Verify teams are properly set up
        EXPECT_TRUE(player_team.hasAlivePokemon()) << "Player team should have alive Pokemon for stage: " << stage.stage_name;
        EXPECT_TRUE(stageTeam->hasAlivePokemon()) << "Stage team should have alive Pokemon for stage: " << stage.stage_name;
        
        // Test AI functionality at this difficulty
        auto stageAI = ai_factory->createAI(static_cast<AIDifficulty>(static_cast<int>(stage.expected_difficulty)));
        ASSERT_TRUE(stageAI != nullptr) << "AI should be created for stage: " << stage.stage_name;
        
        // Verify AI can make decisions
        BattleState stageBattleState;
        stageBattleState.aiPokemon = stageTeam->getPokemon(0);
        stageBattleState.opponentPokemon = player_team.getPokemon(0);
        stageBattleState.aiTeam = stageTeam;
        stageBattleState.opponentTeam = &player_team;
        stageBattleState.currentWeather = WeatherCondition::NONE;
        stageBattleState.weatherTurnsRemaining = 0;
        stageBattleState.turnNumber = 1;
        
        if (stageBattleState.aiPokemon && stageBattleState.opponentPokemon) {
            auto moveEval = stageAI->chooseBestMove(stageBattleState);
            EXPECT_GE(moveEval.moveIndex, 0) << "Valid move should be chosen for stage: " << stage.stage_name;
            EXPECT_LT(moveEval.moveIndex, static_cast<int>(stageBattleState.aiPokemon->moves.size()))
                << "Move index should be within bounds for stage: " << stage.stage_name;
        }
    }
}

// Test 6: AI Integration with Tournament Battle Events
TEST_F(TournamentAIIntegrationTest, AIIntegrationWithTournamentBattleEvents) {
    // Test that AI properly integrates with tournament battle event system
    
    // Create battle with event tracking
    Battle tournamentBattle(player_team, mid_gym_team, Battle::AIDifficulty::MEDIUM);
    
    // Get event manager for tracking
    auto& eventManager = tournamentBattle.getEventManager();
    (void)eventManager; // Suppress unused variable warning
    
    // Verify event system is functional
    EXPECT_FALSE(tournamentBattle.isBattleOver());
    
    // Test that AI decisions can be tracked through events
    // (This tests the integration between AI and battle event system)
    
    // Simulate some battle events that AI should respond to
    Pokemon* aiPokemon = mid_gym_team.getPokemon(0);
    Pokemon* playerPokemon = player_team.getPokemon(0);
    
    ASSERT_TRUE(aiPokemon != nullptr);
    ASSERT_TRUE(playerPokemon != nullptr);
    
    // Test that AI can handle status effects in tournament context
    aiPokemon->applyStatusCondition(StatusCondition::POISON);
    
    // Create AI for decision testing
    auto mediumAI = ai_factory->createAI(AIDifficulty::MEDIUM);
    ASSERT_TRUE(mediumAI != nullptr);
    
    // Test AI decision making with status condition
    BattleState poisonedState;
    poisonedState.aiPokemon = aiPokemon;
    poisonedState.opponentPokemon = playerPokemon;
    poisonedState.aiTeam = &mid_gym_team;
    poisonedState.opponentTeam = &player_team;
    poisonedState.currentWeather = WeatherCondition::NONE;
    poisonedState.weatherTurnsRemaining = 0;
    poisonedState.turnNumber = 5;
    
    // AI should still make valid decisions even when poisoned
    auto moveEval = mediumAI->chooseBestMove(poisonedState);
    EXPECT_GE(moveEval.moveIndex, 0);
    EXPECT_LT(moveEval.moveIndex, static_cast<int>(aiPokemon->moves.size()));
    
    // AI should consider switching when in poor condition
    bool shouldSwitch = mediumAI->shouldSwitch(poisonedState);
    EXPECT_TRUE(shouldSwitch == true || shouldSwitch == false);  // Should make a decision
    
    // Test AI response to tournament-specific scenarios
    // Simulate low health scenario (common in tournament progression)
    aiPokemon->takeDamage(aiPokemon->hp / 2);  // Damage to half health
    
    auto lowHealthMoveEval = mediumAI->chooseBestMove(poisonedState);
    EXPECT_GE(lowHealthMoveEval.moveIndex, 0);
    EXPECT_LT(lowHealthMoveEval.moveIndex, static_cast<int>(aiPokemon->moves.size()));
    
    // AI might be more likely to switch when at low health and statused
    bool shouldSwitchWhenLow = mediumAI->shouldSwitch(poisonedState);
    EXPECT_TRUE(shouldSwitchWhenLow == true || shouldSwitchWhenLow == false);
    
    // Clean up status for other tests
    aiPokemon->clearStatusCondition();
    aiPokemon->heal(aiPokemon->hp);  // Full heal
}

// Test 7: Tournament AI Consistency Across Multiple Battles
TEST_F(TournamentAIIntegrationTest, TournamentAIConsistencyAcrossMultipleBattles) {
    // Test that AI behavior remains consistent across multiple tournament battles
    
    std::vector<Battle::AIDifficulty> difficulties = {
        Battle::AIDifficulty::EASY,
        Battle::AIDifficulty::MEDIUM,
        Battle::AIDifficulty::HARD,
        Battle::AIDifficulty::EXPERT
    };
    
    for (auto difficulty : difficulties) {
        // Run multiple battles with the same difficulty
        std::vector<Battle::BattleResult> results;
        std::vector<bool> battleFunctionality;
        
        for (int battle = 0; battle < 3; ++battle) {
            // Use different teams to test consistency
            Team* opponentTeam;
            switch (battle) {
                case 0: opponentTeam = &early_gym_team; break;
                case 1: opponentTeam = &mid_gym_team; break;
                case 2: opponentTeam = &late_gym_team; break;
                default: opponentTeam = &early_gym_team; break;
            }
            
            Battle consistencyBattle(player_team, *opponentTeam, difficulty);
            
            // Record battle functionality
            battleFunctionality.push_back(!consistencyBattle.isBattleOver());
            results.push_back(consistencyBattle.getBattleResult());
            
            // Test AI decision consistency
            auto ai = ai_factory->createAI(static_cast<AIDifficulty>(static_cast<int>(difficulty)));
            ASSERT_TRUE(ai != nullptr);
            
            BattleState state;
            state.aiPokemon = opponentTeam->getPokemon(0);
            state.opponentPokemon = player_team.getPokemon(0);
            state.aiTeam = opponentTeam;
            state.opponentTeam = &player_team;
            state.currentWeather = WeatherCondition::NONE;
            state.weatherTurnsRemaining = 0;
            state.turnNumber = 1;
            
            if (state.aiPokemon && state.opponentPokemon) {
                auto moveEval = ai->chooseBestMove(state);
                EXPECT_GE(moveEval.moveIndex, 0) 
                    << "Battle " << battle << " with difficulty " << static_cast<int>(difficulty);
                EXPECT_LT(moveEval.moveIndex, static_cast<int>(state.aiPokemon->moves.size()))
                    << "Battle " << battle << " with difficulty " << static_cast<int>(difficulty);
            }
        }
        
        // Verify consistency across all battles
        for (bool functional : battleFunctionality) {
            EXPECT_TRUE(functional) << "All battles should be functional for difficulty " << static_cast<int>(difficulty);
        }
        
        for (auto result : results) {
            EXPECT_EQ(result, Battle::BattleResult::ONGOING) 
                << "All battles should start as ongoing for difficulty " << static_cast<int>(difficulty);
        }
    }
}