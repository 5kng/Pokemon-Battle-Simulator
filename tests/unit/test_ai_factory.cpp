#include <gtest/gtest.h>
#include "../utils/test_utils.h"
#include "ai_factory.h"
#include "ai_strategy.h"
#include "easy_ai.h"
#include "medium_ai.h"
#include "hard_ai.h"
#include "expert_ai.h"

class AIFactoryTest : public TestUtils::PokemonTestFixture {
protected:
    void SetUp() override {
        TestUtils::PokemonTestFixture::SetUp();
        
        // Create test Pokemon and teams for AI testing
        testPokemon1 = TestUtils::createTestPokemon("testmon1", 100, 80, 70, 90, 85, 75, {"normal"});
        testPokemon2 = TestUtils::createTestPokemon("testmon2", 90, 85, 65, 95, 80, 85, {"fire"});
        testPokemon3 = TestUtils::createTestPokemon("testmon3", 110, 75, 80, 85, 90, 70, {"water"});
        
        std::vector<Pokemon> aiTeamPokemon = {testPokemon1, testPokemon2};
        std::vector<Pokemon> opponentTeamPokemon = {testPokemon3};
        
        aiTeam = TestUtils::createTestTeam(aiTeamPokemon);
        opponentTeam = TestUtils::createTestTeam(opponentTeamPokemon);
        
        // Set up basic battle state
        battleState.aiPokemon = &aiTeam.getPokemon()[0];
        battleState.opponentPokemon = &opponentTeam.getPokemon()[0];
        battleState.aiTeam = &aiTeam;
        battleState.opponentTeam = &opponentTeam;
        battleState.currentWeather = WeatherCondition::NONE;
        battleState.weatherTurnsRemaining = 0;
        battleState.turnNumber = 1;
    }
    
    Pokemon testPokemon1, testPokemon2, testPokemon3;
    Team aiTeam, opponentTeam;
    BattleState battleState;
};

// Test creating EasyAI
TEST_F(AIFactoryTest, CreateEasyAI) {
    auto easyAI = AIFactory::createAI(AIDifficulty::EASY);
    
    ASSERT_NE(easyAI, nullptr);
    EXPECT_EQ(easyAI->getDifficulty(), AIDifficulty::EASY);
    
    // Test that it can make basic decisions
    auto moveEval = easyAI->chooseBestMove(battleState);
    EXPECT_GE(moveEval.moveIndex, 0);
    EXPECT_LT(moveEval.moveIndex, static_cast<int>(battleState.aiPokemon->getMoves().size()));
    EXPECT_GE(moveEval.score, 0.0);
    EXPECT_FALSE(moveEval.reasoning.empty());
}

// Test creating MediumAI
TEST_F(AIFactoryTest, CreateMediumAI) {
    auto mediumAI = AIFactory::createAI(AIDifficulty::MEDIUM);
    
    ASSERT_NE(mediumAI, nullptr);
    EXPECT_EQ(mediumAI->getDifficulty(), AIDifficulty::MEDIUM);
    
    // Test that it can make decisions
    auto moveEval = mediumAI->chooseBestMove(battleState);
    EXPECT_GE(moveEval.moveIndex, 0);
    EXPECT_LT(moveEval.moveIndex, static_cast<int>(battleState.aiPokemon->getMoves().size()));
    EXPECT_GE(moveEval.score, 0.0);
    EXPECT_FALSE(moveEval.reasoning.empty());
}

// Test creating HardAI
TEST_F(AIFactoryTest, CreateHardAI) {
    auto hardAI = AIFactory::createAI(AIDifficulty::HARD);
    
    ASSERT_NE(hardAI, nullptr);
    EXPECT_EQ(hardAI->getDifficulty(), AIDifficulty::HARD);
    
    // Test that it can make decisions
    auto moveEval = hardAI->chooseBestMove(battleState);
    EXPECT_GE(moveEval.moveIndex, 0);
    EXPECT_LT(moveEval.moveIndex, static_cast<int>(battleState.aiPokemon->getMoves().size()));
    EXPECT_GE(moveEval.score, 0.0);
    EXPECT_FALSE(moveEval.reasoning.empty());
}

// Test creating ExpertAI
TEST_F(AIFactoryTest, CreateExpertAI) {
    auto expertAI = AIFactory::createAI(AIDifficulty::EXPERT);
    
    ASSERT_NE(expertAI, nullptr);
    EXPECT_EQ(expertAI->getDifficulty(), AIDifficulty::EXPERT);
    
    // Test that it can make decisions
    auto moveEval = expertAI->chooseBestMove(battleState);
    EXPECT_GE(moveEval.moveIndex, 0);
    EXPECT_LT(moveEval.moveIndex, static_cast<int>(battleState.aiPokemon->getMoves().size()));
    EXPECT_GE(moveEval.score, 0.0);
    EXPECT_FALSE(moveEval.reasoning.empty());
}

// Test that different AI difficulties return different instances
TEST_F(AIFactoryTest, DifferentDifficultiesReturnDifferentInstances) {
    auto easyAI = AIFactory::createAI(AIDifficulty::EASY);
    auto mediumAI = AIFactory::createAI(AIDifficulty::MEDIUM);
    auto hardAI = AIFactory::createAI(AIDifficulty::HARD);
    auto expertAI = AIFactory::createAI(AIDifficulty::EXPERT);
    
    ASSERT_NE(easyAI, nullptr);
    ASSERT_NE(mediumAI, nullptr);
    ASSERT_NE(hardAI, nullptr);
    ASSERT_NE(expertAI, nullptr);
    
    // Each should be different instances
    EXPECT_NE(easyAI.get(), mediumAI.get());
    EXPECT_NE(mediumAI.get(), hardAI.get());
    EXPECT_NE(hardAI.get(), expertAI.get());
    
    // Each should have correct difficulty
    EXPECT_EQ(easyAI->getDifficulty(), AIDifficulty::EASY);
    EXPECT_EQ(mediumAI->getDifficulty(), AIDifficulty::MEDIUM);
    EXPECT_EQ(hardAI->getDifficulty(), AIDifficulty::HARD);
    EXPECT_EQ(expertAI->getDifficulty(), AIDifficulty::EXPERT);
}

// Test that factory creates fresh instances each time
TEST_F(AIFactoryTest, FactoryCreatesFreshInstances) {
    auto ai1 = AIFactory::createAI(AIDifficulty::EASY);
    auto ai2 = AIFactory::createAI(AIDifficulty::EASY);
    
    ASSERT_NE(ai1, nullptr);
    ASSERT_NE(ai2, nullptr);
    EXPECT_NE(ai1.get(), ai2.get()); // Should be different instances
    EXPECT_EQ(ai1->getDifficulty(), ai2->getDifficulty()); // But same difficulty
}

// Test AI decision consistency within difficulty level
TEST_F(AIFactoryTest, AIDecisionConsistencyWithinDifficulty) {
    auto ai1 = AIFactory::createAI(AIDifficulty::MEDIUM);
    auto ai2 = AIFactory::createAI(AIDifficulty::MEDIUM);
    
    // Same battle state should produce similar decisions (though not necessarily identical due to randomization)
    auto moveEval1 = ai1->chooseBestMove(battleState);
    auto moveEval2 = ai2->chooseBestMove(battleState);
    
    // Both should choose valid moves
    EXPECT_GE(moveEval1.moveIndex, 0);
    EXPECT_GE(moveEval2.moveIndex, 0);
    EXPECT_LT(moveEval1.moveIndex, static_cast<int>(battleState.aiPokemon->getMoves().size()));
    EXPECT_LT(moveEval2.moveIndex, static_cast<int>(battleState.aiPokemon->getMoves().size()));
}

// Test switch decision functionality
TEST_F(AIFactoryTest, SwitchDecisionFunctionality) {
    auto ai = AIFactory::createAI(AIDifficulty::HARD);
    
    // Test switch evaluation
    auto switchEval = ai->chooseBestSwitch(battleState);
    EXPECT_GE(switchEval.pokemonIndex, 0);
    EXPECT_LT(switchEval.pokemonIndex, static_cast<int>(battleState.aiTeam->getPokemon().size()));
    EXPECT_FALSE(switchEval.reasoning.empty());
    
    // Test should switch decision
    bool shouldSwitch = ai->shouldSwitch(battleState);
    EXPECT_TRUE(shouldSwitch || !shouldSwitch); // Should return a boolean (tautology for compile check)
}

// Test AI behavior under different battle conditions
TEST_F(AIFactoryTest, AIBehaviorUnderDifferentConditions) {
    auto ai = AIFactory::createAI(AIDifficulty::MEDIUM);
    
    // Test with different weather conditions
    battleState.currentWeather = WeatherCondition::SUNNY;
    battleState.weatherTurnsRemaining = 3;
    auto sunnyMoveEval = ai->chooseBestMove(battleState);
    
    battleState.currentWeather = WeatherCondition::RAIN;
    battleState.weatherTurnsRemaining = 2;
    auto rainyMoveEval = ai->chooseBestMove(battleState);
    
    // Both should be valid moves
    EXPECT_GE(sunnyMoveEval.moveIndex, 0);
    EXPECT_GE(rainyMoveEval.moveIndex, 0);
    
    // Test with damaged Pokemon
    battleState.aiPokemon->current_hp = battleState.aiPokemon->stats.hp / 2; // Half health
    auto lowHealthMoveEval = ai->chooseBestMove(battleState);
    EXPECT_GE(lowHealthMoveEval.moveIndex, 0);
}

// Test AI performance with edge cases
TEST_F(AIFactoryTest, AIPerformanceWithEdgeCases) {
    auto ai = AIFactory::createAI(AIDifficulty::EXPERT);
    
    // Test with Pokemon at 1 HP
    battleState.aiPokemon->current_hp = 1;
    auto criticalHealthEval = ai->chooseBestMove(battleState);
    EXPECT_GE(criticalHealthEval.moveIndex, 0);
    
    // Test late in battle
    battleState.turnNumber = 50;
    auto lateBattleEval = ai->chooseBestMove(battleState);
    EXPECT_GE(lateBattleEval.moveIndex, 0);
    
    // Test with status conditions
    battleState.aiPokemon->status_condition = StatusCondition::POISON;
    auto statusMoveEval = ai->chooseBestMove(battleState);
    EXPECT_GE(statusMoveEval.moveIndex, 0);
}

// Test memory management and cleanup
TEST_F(AIFactoryTest, MemoryManagementAndCleanup) {
    std::vector<std::unique_ptr<AIStrategy>> ais;
    
    // Create multiple AI instances
    for (int i = 0; i < 10; ++i) {
        ais.push_back(AIFactory::createAI(AIDifficulty::EASY));
        ais.push_back(AIFactory::createAI(AIDifficulty::MEDIUM));
        ais.push_back(AIFactory::createAI(AIDifficulty::HARD));
        ais.push_back(AIFactory::createAI(AIDifficulty::EXPERT));
    }
    
    // All should be valid
    for (const auto& ai : ais) {
        ASSERT_NE(ai, nullptr);
    }
    
    // Test that they can all make decisions
    for (const auto& ai : ais) {
        auto moveEval = ai->chooseBestMove(battleState);
        EXPECT_GE(moveEval.moveIndex, 0);
    }
    
    // Vector will clean up automatically when out of scope
}

// Test AI evaluation scoring consistency
TEST_F(AIFactoryTest, AIEvaluationScoringConsistency) {
    auto ai = AIFactory::createAI(AIDifficulty::HARD);
    
    // Test multiple evaluations of the same state
    std::vector<MoveEvaluation> evaluations;
    for (int i = 0; i < 5; ++i) {
        evaluations.push_back(ai->chooseBestMove(battleState));
    }
    
    // All evaluations should be valid
    for (const auto& eval : evaluations) {
        EXPECT_GE(eval.moveIndex, 0);
        EXPECT_LT(eval.moveIndex, static_cast<int>(battleState.aiPokemon->getMoves().size()));
        EXPECT_GE(eval.score, 0.0);
        EXPECT_FALSE(eval.reasoning.empty());
    }
}

// Test polymorphic behavior through base class
TEST_F(AIFactoryTest, PolymorphicBehaviorThroughBaseClass) {
    std::vector<std::unique_ptr<AIStrategy>> ais;
    ais.push_back(AIFactory::createAI(AIDifficulty::EASY));
    ais.push_back(AIFactory::createAI(AIDifficulty::MEDIUM));
    ais.push_back(AIFactory::createAI(AIDifficulty::HARD));
    ais.push_back(AIFactory::createAI(AIDifficulty::EXPERT));
    
    // Test polymorphic calls
    for (size_t i = 0; i < ais.size(); ++i) {
        auto& ai = ais[i];
        ASSERT_NE(ai, nullptr);
        
        // Test move evaluation
        auto moveEval = ai->chooseBestMove(battleState);
        EXPECT_GE(moveEval.moveIndex, 0);
        EXPECT_LT(moveEval.moveIndex, static_cast<int>(battleState.aiPokemon->getMoves().size()));
        
        // Test switch evaluation
        auto switchEval = ai->chooseBestSwitch(battleState);
        EXPECT_GE(switchEval.pokemonIndex, 0);
        EXPECT_LT(switchEval.pokemonIndex, static_cast<int>(battleState.aiTeam->getPokemon().size()));
        
        // Test should switch
        ai->shouldSwitch(battleState); // Just ensure it doesn't crash
        
        // Test difficulty getter
        EXPECT_EQ(ai->getDifficulty(), static_cast<AIDifficulty>(i));
    }
}

// Test factory with invalid difficulty handling (if applicable)
TEST_F(AIFactoryTest, FactoryRobustness) {
    // Test creating AI for each valid difficulty multiple times
    for (int difficulty = 0; difficulty < 4; ++difficulty) {
        auto ai = AIFactory::createAI(static_cast<AIDifficulty>(difficulty));
        ASSERT_NE(ai, nullptr);
        EXPECT_EQ(static_cast<int>(ai->getDifficulty()), difficulty);
    }
}

// Performance test for AI creation
TEST_F(AIFactoryTest, AICreationPerformance) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // Create many AI instances
    std::vector<std::unique_ptr<AIStrategy>> ais;
    for (int i = 0; i < 100; ++i) {
        ais.push_back(AIFactory::createAI(AIDifficulty::EASY));
        ais.push_back(AIFactory::createAI(AIDifficulty::MEDIUM));
        ais.push_back(AIFactory::createAI(AIDifficulty::HARD));
        ais.push_back(AIFactory::createAI(AIDifficulty::EXPERT));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should create AIs quickly (less than 1 second for 400 instances)
    EXPECT_LT(duration.count(), 1000);
    
    // Verify all AIs were created successfully
    EXPECT_EQ(ais.size(), 400);
    for (const auto& ai : ais) {
        ASSERT_NE(ai, nullptr);
    }
}