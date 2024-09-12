#include <gtest/gtest.h>
#include "../utils/test_utils.h"
#include "ai_strategy.h"
#include "easy_ai.h"
#include "medium_ai.h"
#include "hard_ai.h"
#include "expert_ai.h"
#include <cmath>

// Concrete test implementation of AIStrategy for testing base class functionality
class TestAIStrategy : public AIStrategy {
public:
    TestAIStrategy(AIDifficulty difficulty) : AIStrategy(difficulty) {}
    
    MoveEvaluation chooseBestMove(const BattleState& battleState) override {
        // Simple test implementation
        MoveEvaluation eval;
        eval.moveIndex = 0;
        eval.score = 1.0;
        eval.reasoning = "Test move selection";
        return eval;
    }
    
    SwitchEvaluation chooseBestSwitch(const BattleState& battleState) override {
        // Simple test implementation
        SwitchEvaluation eval;
        eval.pokemonIndex = 0;
        eval.score = 0.5;
        eval.reasoning = "Test switch selection";
        return eval;
    }
    
    bool shouldSwitch(const BattleState& battleState) override {
        // Simple test implementation - switch if current Pokemon has low health
        return calculateHealthRatio(*battleState.aiPokemon) < 0.25;
    }
    
    // Expose protected methods for testing
    double testCalculateTypeEffectiveness(const std::string& moveType, 
                                         const std::vector<std::string>& defenderTypes) const {
        return calculateTypeEffectiveness(moveType, defenderTypes);
    }
    
    double testEstimateDamage(const Pokemon& attacker, const Pokemon& defender,
                             const Move& move, WeatherCondition weather) const {
        return estimateDamage(attacker, defender, move, weather);
    }
    
    bool testIsPokemonThreatened(const Pokemon& pokemon, const Pokemon& opponent) const {
        return isPokemonThreatened(pokemon, opponent);
    }
    
    double testCalculateHealthRatio(const Pokemon& pokemon) const {
        return calculateHealthRatio(pokemon);
    }
    
    std::vector<Move*> testGetUsableMoves(Pokemon& pokemon) const {
        return getUsableMoves(pokemon);
    }
};

class AIStrategyTest : public TestUtils::BattleTestFixture {
protected:
    void SetUp() override {
        TestUtils::BattleTestFixture::SetUp();
        
        // Create test AI strategies
        testAI = std::make_unique<TestAIStrategy>(AIDifficulty::MEDIUM);
        easyAI = std::make_unique<EasyAI>();
        
        // Create additional test Pokemon with different types
        electricPokemon = TestUtils::createTestPokemon("ElectricMon", 90, 75, 60, 95, 80, 100, {"electric"});
        waterPokemon = TestUtils::createTestPokemon("WaterMon", 110, 70, 80, 85, 95, 70, {"water"});
        flyingPokemon = TestUtils::createTestPokemon("FlyingMon", 85, 90, 65, 75, 70, 110, {"flying"});
        dualTypePokemon = TestUtils::createTestPokemon("DualMon", 95, 85, 75, 90, 85, 80, {"fire", "flying"});
        
        // Create test moves with different types and powers
        electricMove = TestUtils::createTestMove("thunderbolt", 90, 100, 15, "electric", "special");
        waterMove = TestUtils::createTestMove("hydro-pump", 110, 80, 5, "water", "special");
        groundMove = TestUtils::createTestMove("earthquake", 100, 100, 10, "ground", "physical");
        weakMove = TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical");
        statusMove = TestUtils::createTestMove("thunder-wave", 0, 90, 20, "electric", "status", StatusCondition::PARALYSIS, 100);
        
        // Set up battle state
        setupBattleState();
    }
    
    void setupBattleState() {
        battleState.aiPokemon = &testPokemon1;
        battleState.opponentPokemon = &testPokemon2;
        battleState.aiTeam = &playerTeam;
        battleState.opponentTeam = &opponentTeam;
        battleState.currentWeather = WeatherCondition::NONE;
        battleState.weatherTurnsRemaining = 0;
        battleState.turnNumber = 1;
    }
    
    BattleState battleState;
    std::unique_ptr<TestAIStrategy> testAI;
    std::unique_ptr<EasyAI> easyAI;
    
    Pokemon electricPokemon, waterPokemon, flyingPokemon, dualTypePokemon;
    Move electricMove, waterMove, groundMove, weakMove, statusMove;
};

// Test AI difficulty getter
TEST_F(AIStrategyTest, DifficultyGetter) {
    EXPECT_EQ(testAI->getDifficulty(), AIDifficulty::MEDIUM);
    EXPECT_EQ(easyAI->getDifficulty(), AIDifficulty::EASY);
    
    TestAIStrategy hardTestAI(AIDifficulty::HARD);
    EXPECT_EQ(hardTestAI.getDifficulty(), AIDifficulty::HARD);
    
    TestAIStrategy expertTestAI(AIDifficulty::EXPERT);
    EXPECT_EQ(expertTestAI.getDifficulty(), AIDifficulty::EXPERT);
}

// Test basic move evaluation
TEST_F(AIStrategyTest, BasicMoveEvaluation) {
    auto moveEval = testAI->chooseBestMove(battleState);
    
    EXPECT_GE(moveEval.moveIndex, 0);
    EXPECT_GT(moveEval.score, 0.0);
    EXPECT_FALSE(moveEval.reasoning.empty());
}

// Test basic switch evaluation
TEST_F(AIStrategyTest, BasicSwitchEvaluation) {
    auto switchEval = testAI->chooseBestSwitch(battleState);
    
    EXPECT_GE(switchEval.pokemonIndex, 0);
    EXPECT_GE(switchEval.score, 0.0);
    EXPECT_FALSE(switchEval.reasoning.empty());
}

// Test should switch logic
TEST_F(AIStrategyTest, ShouldSwitchLogic) {
    // Test with healthy Pokemon (should not switch)
    battleState.aiPokemon->current_hp = battleState.aiPokemon->stats.hp;
    EXPECT_FALSE(testAI->shouldSwitch(battleState));
    
    // Test with low health Pokemon (should switch)
    battleState.aiPokemon->current_hp = battleState.aiPokemon->stats.hp / 5; // 20% health
    EXPECT_TRUE(testAI->shouldSwitch(battleState));
    
    // Test with critical health Pokemon (should switch)
    battleState.aiPokemon->current_hp = 1;
    EXPECT_TRUE(testAI->shouldSwitch(battleState));
}

// Test type effectiveness calculation
TEST_F(AIStrategyTest, TypeEffectivenessCalculation) {
    // Super effective matchups
    double effectiveness = testAI->testCalculateTypeEffectiveness("electric", {"water"});
    EXPECT_GT(effectiveness, 1.0);
    
    effectiveness = testAI->testCalculateTypeEffectiveness("water", {"fire"});
    EXPECT_GT(effectiveness, 1.0);
    
    // Not very effective matchups
    effectiveness = testAI->testCalculateTypeEffectiveness("water", {"grass"});
    EXPECT_LT(effectiveness, 1.0);
    
    effectiveness = testAI->testCalculateTypeEffectiveness("fire", {"water"});
    EXPECT_LT(effectiveness, 1.0);
    
    // No effect matchups
    effectiveness = testAI->testCalculateTypeEffectiveness("electric", {"ground"});
    EXPECT_EQ(effectiveness, 0.0);
    
    effectiveness = testAI->testCalculateTypeEffectiveness("ground", {"flying"});
    EXPECT_EQ(effectiveness, 0.0);
    
    // Normal effectiveness
    effectiveness = testAI->testCalculateTypeEffectiveness("normal", {"normal"});
    EXPECT_EQ(effectiveness, 1.0);
}

// Test dual type effectiveness
TEST_F(AIStrategyTest, DualTypeEffectiveness) {
    // Test against dual-type Pokemon
    std::vector<std::string> fireFlying = {"fire", "flying"};
    
    // Rock should be super effective against flying but not very effective against fire
    double effectiveness = testAI->testCalculateTypeEffectiveness("rock", fireFlying);
    EXPECT_GT(effectiveness, 1.0); // Should still be super effective overall
    
    // Water should be super effective against fire but not very effective against flying
    effectiveness = testAI->testCalculateTypeEffectiveness("water", fireFlying);
    EXPECT_GT(effectiveness, 1.0);
    
    // Electric should be not very effective against fire and no effect against flying
    effectiveness = testAI->testCalculateTypeEffectiveness("electric", fireFlying);
    EXPECT_EQ(effectiveness, 0.0); // No effect due to flying type
}

// Test damage estimation
TEST_F(AIStrategyTest, DamageEstimation) {
    double damage = testAI->testEstimateDamage(electricPokemon, waterPokemon, electricMove, WeatherCondition::NONE);
    
    // Should return a positive damage value
    EXPECT_GT(damage, 0.0);
    
    // Super effective move should deal more damage
    double superEffectiveDamage = testAI->testEstimateDamage(electricPokemon, waterPokemon, electricMove, WeatherCondition::NONE);
    double normalDamage = testAI->testEstimateDamage(electricPokemon, waterPokemon, weakMove, WeatherCondition::NONE);
    
    EXPECT_GT(superEffectiveDamage, normalDamage);
}

// Test damage estimation with weather
TEST_F(AIStrategyTest, DamageEstimationWithWeather) {
    // Test fire move in sunny weather (should be boosted)
    Move fireMove = TestUtils::createTestMove("flamethrower", 90, 100, 15, "fire", "special");
    
    double sunnyDamage = testAI->testEstimateDamage(testPokemon1, testPokemon2, fireMove, WeatherCondition::SUNNY);
    double normalDamage = testAI->testEstimateDamage(testPokemon1, testPokemon2, fireMove, WeatherCondition::NONE);
    
    EXPECT_GE(sunnyDamage, normalDamage); // Should be at least equal, likely greater
    
    // Test water move in rainy weather (should be boosted)
    double rainyDamage = testAI->testEstimateDamage(testPokemon1, testPokemon2, waterMove, WeatherCondition::RAIN);
    double normalWaterDamage = testAI->testEstimateDamage(testPokemon1, testPokemon2, waterMove, WeatherCondition::NONE);
    
    EXPECT_GE(rainyDamage, normalWaterDamage);
}

// Test Pokemon threat assessment
TEST_F(AIStrategyTest, PokemonThreatAssessment) {
    // High level opponent should be threatening
    testPokemon2.level = 100;
    testPokemon2.stats.attack = 150;
    EXPECT_TRUE(testAI->testIsPokemonThreatened(testPokemon1, testPokemon2));
    
    // Low level opponent should not be threatening
    testPokemon2.level = 5;
    testPokemon2.stats.attack = 20;
    EXPECT_FALSE(testAI->testIsPokemonThreatened(testPokemon1, testPokemon2));
}

// Test health ratio calculation
TEST_F(AIStrategyTest, HealthRatioCalculation) {
    // Full health
    testPokemon1.current_hp = testPokemon1.stats.hp;
    EXPECT_DOUBLE_EQ(testAI->testCalculateHealthRatio(testPokemon1), 1.0);
    
    // Half health
    testPokemon1.current_hp = testPokemon1.stats.hp / 2;
    EXPECT_DOUBLE_EQ(testAI->testCalculateHealthRatio(testPokemon1), 0.5);
    
    // Quarter health
    testPokemon1.current_hp = testPokemon1.stats.hp / 4;
    EXPECT_DOUBLE_EQ(testAI->testCalculateHealthRatio(testPokemon1), 0.25);
    
    // Zero health
    testPokemon1.current_hp = 0;
    EXPECT_DOUBLE_EQ(testAI->testCalculateHealthRatio(testPokemon1), 0.0);
    
    // Test with edge case of 1 HP
    testPokemon1.current_hp = 1;
    double ratio = testAI->testCalculateHealthRatio(testPokemon1);
    EXPECT_GT(ratio, 0.0);
    EXPECT_LT(ratio, 0.1); // Should be very small but positive
}

// Test usable moves retrieval
TEST_F(AIStrategyTest, UsableMovesRetrieval) {
    auto usableMoves = testAI->testGetUsableMoves(testPokemon1);
    
    // Should return moves that have PP remaining
    for (const auto& move : usableMoves) {
        EXPECT_GT(move->getRemainingPP(), 0);
    }
    
    // Test with Pokemon that has no PP remaining
    for (auto& move : testPokemon1.getMoves()) {
        move.current_pp = 0;
    }
    
    auto noUsableMoves = testAI->testGetUsableMoves(testPokemon1);
    EXPECT_TRUE(noUsableMoves.empty());
}

// Test battle state with different weather conditions
TEST_F(AIStrategyTest, BattleStateWithWeatherConditions) {
    // Test with sunny weather
    battleState.currentWeather = WeatherCondition::SUNNY;
    battleState.weatherTurnsRemaining = 5;
    
    auto moveEval = testAI->chooseBestMove(battleState);
    EXPECT_GE(moveEval.moveIndex, 0);
    
    // Test with rainy weather
    battleState.currentWeather = WeatherCondition::RAIN;
    battleState.weatherTurnsRemaining = 3;
    
    moveEval = testAI->chooseBestMove(battleState);
    EXPECT_GE(moveEval.moveIndex, 0);
    
    // Test with sandstorm
    battleState.currentWeather = WeatherCondition::SANDSTORM;
    battleState.weatherTurnsRemaining = 8;
    
    moveEval = testAI->chooseBestMove(battleState);
    EXPECT_GE(moveEval.moveIndex, 0);
    
    // Test with hail
    battleState.currentWeather = WeatherCondition::HAIL;
    battleState.weatherTurnsRemaining = 4;
    
    moveEval = testAI->chooseBestMove(battleState);
    EXPECT_GE(moveEval.moveIndex, 0);
}

// Test battle state with different turn numbers
TEST_F(AIStrategyTest, BattleStateWithDifferentTurnNumbers) {
    // Early battle
    battleState.turnNumber = 1;
    auto earlyMoveEval = testAI->chooseBestMove(battleState);
    EXPECT_GE(earlyMoveEval.moveIndex, 0);
    
    // Mid battle
    battleState.turnNumber = 15;
    auto midMoveEval = testAI->chooseBestMove(battleState);
    EXPECT_GE(midMoveEval.moveIndex, 0);
    
    // Late battle
    battleState.turnNumber = 50;
    auto lateMoveEval = testAI->chooseBestMove(battleState);
    EXPECT_GE(lateMoveEval.moveIndex, 0);
}

// Test concrete AI implementations
TEST_F(AIStrategyTest, ConcreteAIImplementations) {
    // Test EasyAI
    auto easyMoveEval = easyAI->chooseBestMove(battleState);
    EXPECT_GE(easyMoveEval.moveIndex, 0);
    EXPECT_FALSE(easyMoveEval.reasoning.empty());
    
    auto easySwitchEval = easyAI->chooseBestSwitch(battleState);
    EXPECT_GE(easySwitchEval.pokemonIndex, 0);
    EXPECT_FALSE(easySwitchEval.reasoning.empty());
    
    // EasyAI should make decisions consistently
    auto easyMoveEval2 = easyAI->chooseBestMove(battleState);
    EXPECT_GE(easyMoveEval2.moveIndex, 0);
}

// Test AI strategy with status conditions
TEST_F(AIStrategyTest, AIStrategyWithStatusConditions) {
    // Test with paralyzed Pokemon
    battleState.aiPokemon->status_condition = StatusCondition::PARALYSIS;
    auto paralyzedEval = testAI->chooseBestMove(battleState);
    EXPECT_GE(paralyzedEval.moveIndex, 0);
    
    // Test with poisoned Pokemon
    battleState.aiPokemon->status_condition = StatusCondition::POISON;
    auto poisonedEval = testAI->chooseBestMove(battleState);
    EXPECT_GE(poisonedEval.moveIndex, 0);
    
    // Test with burned Pokemon
    battleState.aiPokemon->status_condition = StatusCondition::BURN;
    auto burnedEval = testAI->chooseBestMove(battleState);
    EXPECT_GE(burnedEval.moveIndex, 0);
    
    // Test with sleeping Pokemon
    battleState.aiPokemon->status_condition = StatusCondition::SLEEP;
    auto sleepEval = testAI->chooseBestMove(battleState);
    EXPECT_GE(sleepEval.moveIndex, 0);
    
    // Test with frozen Pokemon
    battleState.aiPokemon->status_condition = StatusCondition::FREEZE;
    auto frozenEval = testAI->chooseBestMove(battleState);
    EXPECT_GE(frozenEval.moveIndex, 0);
}

// Test evaluation scores are reasonable
TEST_F(AIStrategyTest, EvaluationScoresReasonable) {
    auto moveEval = testAI->chooseBestMove(battleState);
    auto switchEval = testAI->chooseBestSwitch(battleState);
    
    // Scores should be non-negative
    EXPECT_GE(moveEval.score, 0.0);
    EXPECT_GE(switchEval.score, 0.0);
    
    // Scores should be reasonable (not infinity or NaN)
    EXPECT_FALSE(std::isinf(moveEval.score));
    EXPECT_FALSE(std::isnan(moveEval.score));
    EXPECT_FALSE(std::isinf(switchEval.score));
    EXPECT_FALSE(std::isnan(switchEval.score));
}

// Test deterministic behavior with same input
TEST_F(AIStrategyTest, DeterministicBehaviorWithSameInput) {
    // Reset battle state to ensure consistency
    setupBattleState();
    
    auto eval1 = testAI->chooseBestMove(battleState);
    auto eval2 = testAI->chooseBestMove(battleState);
    
    // With same input, should get same results (for deterministic AI)
    EXPECT_EQ(eval1.moveIndex, eval2.moveIndex);
    EXPECT_EQ(eval1.score, eval2.score);
}

// Test AI with empty move list (edge case)
TEST_F(AIStrategyTest, AIWithEmptyMoveList) {
    // Remove all moves from AI Pokemon
    battleState.aiPokemon->getMoves().clear();
    
    // Should handle gracefully without crashing
    EXPECT_NO_THROW({
        auto moveEval = testAI->chooseBestMove(battleState);
        // May return invalid move index, but shouldn't crash
    });
}

// Test AI with single move
TEST_F(AIStrategyTest, AIWithSingleMove) {
    // Ensure Pokemon has only one move
    auto& moves = battleState.aiPokemon->getMoves();
    if (moves.size() > 1) {
        moves.resize(1);
    }
    
    auto moveEval = testAI->chooseBestMove(battleState);
    EXPECT_EQ(moveEval.moveIndex, 0); // Should choose the only available move
}

// Performance test for AI decision making
TEST_F(AIStrategyTest, PerformanceTest) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Make many AI decisions
    for (int i = 0; i < 100; ++i) {
        testAI->chooseBestMove(battleState);
        testAI->chooseBestSwitch(battleState);
        testAI->shouldSwitch(battleState);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete reasonably quickly (less than 5 seconds for 100 iterations)
    EXPECT_LT(duration.count(), 5000);
}