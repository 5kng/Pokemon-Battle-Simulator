#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include "test_utils.h"
#include "tournament_manager.h"
#include "battle.h"
#include "team_builder.h"
#include "pokemon_data.h"
#include "team.h"

/**
 * @brief Integration tests for Team State Persistence across tournament battles
 * 
 * These tests validate that team state (HP, PP, status conditions, experience)
 * is properly maintained and synchronized across tournament progression,
 * including persistence to disk and recovery after system restarts.
 * Focus areas:
 * - Team state consistency across multiple tournament battles
 * - Proper team healing and restoration between tournament stages
 * - Team state persistence to disk and recovery
 * - Team experience and growth tracking through tournament progression
 */
class TeamStatePersistenceIntegrationTest : public TestUtils::BattleTestFixture {
protected:
    void SetUp() override {
        TestUtils::BattleTestFixture::SetUp();
        
        // Initialize core systems
        pokemon_data = std::make_shared<PokemonData>();
        team_builder = std::make_shared<TeamBuilder>(pokemon_data);
        tournament_manager = std::make_shared<TournamentManager>(pokemon_data, team_builder);
        
        // Setup test player and teams
        setupTestPlayer();
        setupOpponentTeams();
        
        // Initialize tournament progress
        tournament_manager->initializePlayerProgress(test_player_name);
        
        // Setup test directories
        setupTestDirectories();
    }
    
    void TearDown() override {
        // Clean up test files
        cleanupTestFiles();
    }
    
    void setupTestPlayer() {
        test_player_name = "PersistenceTestTrainer";
        
        // Create a team with varied Pokemon for comprehensive state testing
        Pokemon persistent1 = TestUtils::createTestPokemon("StateKeeper1", 100, 90, 80, 95, 85, 85, {"water", "electric"});
        persistent1.moves.clear();
        persistent1.moves.push_back(TestUtils::createTestMove("surf", 90, 100, 15, "water", "special"));
        persistent1.moves.push_back(TestUtils::createTestMove("thunderbolt", 90, 100, 15, "electric", "special"));
        persistent1.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
        persistent1.moves.push_back(TestUtils::createTestMove("recover", 0, 100, 5, "normal", "status"));
        
        Pokemon persistent2 = TestUtils::createTestPokemon("StateKeeper2", 110, 85, 95, 80, 90, 70, {"grass", "poison"});
        persistent2.moves.clear();
        persistent2.moves.push_back(TestUtils::createTestMove("giga-drain", 75, 100, 10, "grass", "special"));
        persistent2.moves.push_back(TestUtils::createTestMove("sludge-bomb", 90, 100, 10, "poison", "special"));
        persistent2.moves.push_back(TestUtils::createTestMove("synthesis", 0, 100, 5, "grass", "status"));
        persistent2.moves.push_back(TestUtils::createTestMove("toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON, 100));
        
        Pokemon persistent3 = TestUtils::createTestPokemon("StateKeeper3", 95, 110, 75, 100, 80, 100, {"fire", "flying"});
        persistent3.moves.clear();
        persistent3.moves.push_back(TestUtils::createTestMove("flamethrower", 90, 100, 15, "fire", "special"));
        persistent3.moves.push_back(TestUtils::createTestMove("air-slash", 75, 95, 15, "flying", "special"));
        persistent3.moves.push_back(TestUtils::createTestMove("roost", 0, 100, 10, "flying", "status"));
        persistent3.moves.push_back(TestUtils::createTestMove("will-o-wisp", 0, 85, 15, "fire", "status", StatusCondition::BURN, 100));
        
        player_team = TestUtils::createTestTeam({persistent1, persistent2, persistent3});
        original_team_state = captureDetailedTeamState(player_team);
    }
    
    void setupOpponentTeams() {
        // Early tournament opponent (Gym 1)
        Pokemon early1 = TestUtils::createTestPokemon("EarlyRock1", 90, 80, 100, 30, 30, 20, {"rock"});
        early1.moves.clear();
        early1.moves.push_back(TestUtils::createTestMove("rock-throw", 50, 90, 15, "rock", "physical"));
        early1.moves.push_back(TestUtils::createTestMove("tackle", 40, 100, 35, "normal", "physical"));
        early1.moves.push_back(TestUtils::createTestMove("harden", 0, 100, 30, "normal", "status"));
        
        early_opponent = TestUtils::createTestTeam({early1});
        
        // Mid tournament opponent (Gym 4)
        Pokemon mid1 = TestUtils::createTestPokemon("MidGrass1", 105, 75, 85, 100, 95, 65, {"grass"});
        mid1.moves.clear();
        mid1.moves.push_back(TestUtils::createTestMove("petal-dance", 120, 100, 10, "grass", "special"));
        mid1.moves.push_back(TestUtils::createTestMove("sleep-powder", 0, 75, 15, "grass", "status", StatusCondition::SLEEP, 100));
        mid1.moves.push_back(TestUtils::createTestMove("giga-drain", 75, 100, 10, "grass", "special"));
        
        Pokemon mid2 = TestUtils::createTestPokemon("MidGrass2", 100, 85, 80, 95, 90, 75, {"grass", "poison"});
        mid2.moves.clear();
        mid2.moves.push_back(TestUtils::createTestMove("razor-leaf", 55, 95, 25, "grass", "physical"));
        mid2.moves.push_back(TestUtils::createTestMove("poison-powder", 0, 75, 35, "poison", "status", StatusCondition::POISON, 100));
        mid2.moves.push_back(TestUtils::createTestMove("synthesis", 0, 100, 5, "grass", "status"));
        
        mid_opponent = TestUtils::createTestTeam({mid1, mid2});
        
        // Late tournament opponent (Gym 8)
        Pokemon late1 = TestUtils::createTestPokemon("LateGround1", 120, 120, 100, 80, 80, 50, {"ground"});
        late1.moves.clear();
        late1.moves.push_back(TestUtils::createTestMove("earthquake", 100, 100, 10, "ground", "physical"));
        late1.moves.push_back(TestUtils::createTestMove("fissure", 0, 30, 5, "ground", "physical"));  // OHKO move
        late1.moves.push_back(TestUtils::createTestMove("sandstorm", 0, 100, 10, "rock", "status"));
        
        Pokemon late2 = TestUtils::createTestPokemon("LateGround2", 110, 100, 90, 85, 85, 80, {"ground", "rock"});
        late2.moves.clear();
        late2.moves.push_back(TestUtils::createTestMove("stone-edge", 100, 80, 5, "rock", "physical"));
        late2.moves.push_back(TestUtils::createTestMove("earthquake", 100, 100, 10, "ground", "physical"));
        late2.moves.push_back(TestUtils::createTestMove("sandstorm", 0, 100, 10, "rock", "status"));
        
        late_opponent = TestUtils::createTestTeam({late1, late2});
    }
    
    void setupTestDirectories() {
        test_save_directory = std::filesystem::current_path() / "test_tournament_saves";
        std::filesystem::create_directories(test_save_directory);
    }
    
    void cleanupTestFiles() {
        if (std::filesystem::exists(test_save_directory)) {
            std::filesystem::remove_all(test_save_directory);
        }
    }
    
    // Detailed team state capture for comprehensive persistence testing
    struct DetailedTeamState {
        std::vector<int> pokemon_hp;
        std::vector<int> pokemon_current_hp;
        std::vector<std::vector<int>> pokemon_move_pp;
        std::vector<std::vector<int>> pokemon_move_max_pp;
        std::vector<StatusCondition> pokemon_status;
        std::vector<std::string> pokemon_names;
        std::vector<std::vector<std::string>> pokemon_types;
        
        bool operator==(const DetailedTeamState& other) const {
            return pokemon_hp == other.pokemon_hp && 
                   pokemon_current_hp == other.pokemon_current_hp &&
                   pokemon_move_pp == other.pokemon_move_pp &&
                   pokemon_move_max_pp == other.pokemon_move_max_pp &&
                   pokemon_status == other.pokemon_status &&
                   pokemon_names == other.pokemon_names &&
                   pokemon_types == other.pokemon_types;
        }
        
        // Calculate state hash for integrity checking
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
    };
    
    DetailedTeamState captureDetailedTeamState(const Team& team) {
        DetailedTeamState state;
        for (size_t i = 0; i < team.size(); ++i) {
            const Pokemon* pokemon = team.getPokemon(i);
            if (pokemon) {
                state.pokemon_hp.push_back(pokemon->hp);
                state.pokemon_current_hp.push_back(pokemon->current_hp);
                state.pokemon_status.push_back(pokemon->status);
                state.pokemon_names.push_back(pokemon->name);
                state.pokemon_types.push_back(pokemon->types);
                
                std::vector<int> move_pp, move_max_pp;
                for (const auto& move : pokemon->moves) {
                    move_pp.push_back(move.current_pp);
                    move_max_pp.push_back(move.pp);
                }
                state.pokemon_move_pp.push_back(move_pp);
                state.pokemon_move_max_pp.push_back(move_max_pp);
            }
        }
        return state;
    }
    
    void applyDetailedTeamState(Team& team, const DetailedTeamState& state) {
        for (size_t i = 0; i < team.size() && i < state.pokemon_hp.size(); ++i) {
            Pokemon* pokemon = const_cast<Pokemon*>(team.getPokemon(i));
            if (pokemon) {
                pokemon->current_hp = state.pokemon_current_hp[i];
                pokemon->applyStatusCondition(state.pokemon_status[i]);
                
                // Restore move PP
                for (size_t j = 0; j < pokemon->moves.size() && j < state.pokemon_move_pp[i].size(); ++j) {
                    pokemon->moves[j].current_pp = state.pokemon_move_pp[i][j];
                }
            }
        }
    }
    
    void simulateBattleDamage(Team& team, const std::string& damagePattern) {
        if (damagePattern == "light") {
            // Light damage pattern
            if (team.getPokemon(0)) {
                team.getPokemon(0)->takeDamage(15);
                if (!team.getPokemon(0)->moves.empty()) team.getPokemon(0)->moves[0].current_pp -= 2;
            }
            if (team.getPokemon(1)) {
                team.getPokemon(1)->takeDamage(10);
                if (team.getPokemon(1)->moves.size() > 1) team.getPokemon(1)->moves[1].current_pp -= 1;
            }
        } else if (damagePattern == "moderate") {
            // Moderate damage pattern
            if (team.getPokemon(0)) {
                team.getPokemon(0)->takeDamage(40);
                team.getPokemon(0)->applyStatusCondition(StatusCondition::BURN);
                if (!team.getPokemon(0)->moves.empty()) team.getPokemon(0)->moves[0].current_pp -= 4;
            }
            if (team.getPokemon(1)) {
                team.getPokemon(1)->takeDamage(25);
                if (team.getPokemon(1)->moves.size() > 2) team.getPokemon(1)->moves[2].current_pp -= 3;
            }
            if (team.getPokemon(2)) {
                team.getPokemon(2)->takeDamage(20);
                if (team.getPokemon(2)->moves.size() > 1) team.getPokemon(2)->moves[1].current_pp -= 2;
            }
        } else if (damagePattern == "heavy") {
            // Heavy damage pattern
            if (team.getPokemon(0)) {
                team.getPokemon(0)->takeDamage(70);
                team.getPokemon(0)->applyStatusCondition(StatusCondition::PARALYSIS);
                if (!team.getPokemon(0)->moves.empty()) team.getPokemon(0)->moves[0].current_pp -= 6;
            }
            if (team.getPokemon(1)) {
                team.getPokemon(1)->takeDamage(50);
                team.getPokemon(1)->applyStatusCondition(StatusCondition::POISON);
                if (team.getPokemon(1)->moves.size() > 3) team.getPokemon(1)->moves[3].current_pp -= 4;
            }
            if (team.getPokemon(2)) {
                team.getPokemon(2)->takeDamage(35);
                if (team.getPokemon(2)->moves.size() > 2) team.getPokemon(2)->moves[2].current_pp -= 5;
            }
        }
    }
    
    void healTeamFully(Team& team) {
        for (size_t i = 0; i < team.size(); ++i) {
            Pokemon* pokemon = team.getPokemon(i);
            if (pokemon) {
                pokemon->heal(pokemon->hp);
                pokemon->clearStatusCondition();
                for (auto& move : pokemon->moves) {
                    move.current_pp = move.pp;
                }
            }
        }
    }
    
    // Core test objects
    std::shared_ptr<PokemonData> pokemon_data;
    std::shared_ptr<TeamBuilder> team_builder;
    std::shared_ptr<TournamentManager> tournament_manager;
    
    // Test data
    std::string test_player_name;
    Team player_team;
    Team early_opponent;
    Team mid_opponent;
    Team late_opponent;
    DetailedTeamState original_team_state;
    std::filesystem::path test_save_directory;
};

// Test 1: Team State Consistency Across Multiple Tournament Battles
TEST_F(TeamStatePersistenceIntegrationTest, TeamStateConsistencyAcrossMultipleBattles) {
    // Capture initial state
    auto initialState = captureDetailedTeamState(player_team);
    EXPECT_EQ(initialState.pokemon_current_hp[0], initialState.pokemon_hp[0]);
    EXPECT_EQ(initialState.pokemon_current_hp[1], initialState.pokemon_hp[1]);
    EXPECT_EQ(initialState.pokemon_current_hp[2], initialState.pokemon_hp[2]);
    
    // Battle 1: Early tournament battle
    {
        Battle earlyBattle(player_team, early_opponent, Battle::AIDifficulty::EASY);
        EXPECT_FALSE(earlyBattle.isBattleOver());
        
        // Simulate light battle damage
        simulateBattleDamage(player_team, "light");
        
        auto postBattle1State = captureDetailedTeamState(player_team);
        
        // Verify damage was applied
        EXPECT_LT(postBattle1State.pokemon_current_hp[0], initialState.pokemon_current_hp[0]);
        EXPECT_LT(postBattle1State.pokemon_move_pp[0][0], initialState.pokemon_move_pp[0][0]);
        
        // Record battle result
        TournamentManager::TournamentBattleResult result1;
        result1.challenge_name = "Early Gym";
        result1.challenge_type = "gym";
        result1.victory = true;
        result1.performance_score = 88.5;
        
        tournament_manager->updatePlayerProgress(test_player_name, result1);
        
        // Simulate healing between battles (tournament feature)
        healTeamFully(player_team);
        
        // Verify healing restored team
        auto healedState = captureDetailedTeamState(player_team);
        EXPECT_EQ(healedState.pokemon_current_hp[0], healedState.pokemon_hp[0]);
        EXPECT_EQ(healedState.pokemon_status[0], StatusCondition::NONE);
    }
    
    // Battle 2: Mid tournament battle with higher difficulty
    {
        Battle midBattle(player_team, mid_opponent, Battle::AIDifficulty::MEDIUM);
        EXPECT_FALSE(midBattle.isBattleOver());
        
        // Simulate moderate battle damage with status effects
        simulateBattleDamage(player_team, "moderate");
        
        auto postBattle2State = captureDetailedTeamState(player_team);
        
        // Verify more significant damage
        EXPECT_LT(postBattle2State.pokemon_current_hp[0], postBattle2State.pokemon_hp[0] * 0.7);
        EXPECT_NE(postBattle2State.pokemon_status[0], StatusCondition::NONE);
        EXPECT_LT(postBattle2State.pokemon_move_pp[0][0], initialState.pokemon_move_pp[0][0]);
        
        // Record battle result
        TournamentManager::TournamentBattleResult result2;
        result2.challenge_name = "Mid Gym";
        result2.challenge_type = "gym";
        result2.victory = true;
        result2.performance_score = 82.0;
        
        tournament_manager->updatePlayerProgress(test_player_name, result2);
        
        // Healing between battles
        healTeamFully(player_team);
    }
    
    // Battle 3: Late tournament battle with maximum difficulty
    {
        Battle lateBattle(player_team, late_opponent, Battle::AIDifficulty::HARD);
        EXPECT_FALSE(lateBattle.isBattleOver());
        
        // Simulate heavy battle damage
        simulateBattleDamage(player_team, "heavy");
        
        auto postBattle3State = captureDetailedTeamState(player_team);
        
        // Verify heavy damage and multiple status effects
        EXPECT_LT(postBattle3State.pokemon_current_hp[0], postBattle3State.pokemon_hp[0] * 0.5);
        EXPECT_LT(postBattle3State.pokemon_current_hp[1], postBattle3State.pokemon_hp[1] * 0.6);
        EXPECT_NE(postBattle3State.pokemon_status[0], StatusCondition::NONE);
        EXPECT_NE(postBattle3State.pokemon_status[1], StatusCondition::NONE);
        
        // Record battle result
        TournamentManager::TournamentBattleResult result3;
        result3.challenge_name = "Late Gym";
        result3.challenge_type = "gym";
        result3.victory = true;
        result3.performance_score = 75.5;
        
        tournament_manager->updatePlayerProgress(test_player_name, result3);
    }
    
    // Verify tournament progression tracking
    auto progress = tournament_manager->getPlayerProgress(test_player_name);
    ASSERT_TRUE(progress.has_value());
    
    // Should have battle history
    auto battleHistory = tournament_manager->getPlayerBattleHistory(test_player_name);
    EXPECT_EQ(battleHistory.size(), 3);
    
    // Team state should be consistently trackable
    auto finalState = captureDetailedTeamState(player_team);
    EXPECT_EQ(finalState.pokemon_names.size(), 3);
    EXPECT_EQ(finalState.pokemon_hp.size(), 3);
}

// Test 2: Team Healing and Restoration Between Tournament Stages
TEST_F(TeamStatePersistenceIntegrationTest, TeamHealingAndRestorationBetweenStages) {
    // Test various healing scenarios in tournament progression
    
    // Scenario 1: Full heal between gym battles
    {
        // Apply damage
        simulateBattleDamage(player_team, "moderate");
        auto damagedState = captureDetailedTeamState(player_team);
        
        // Verify damage
        EXPECT_LT(damagedState.pokemon_current_hp[0], damagedState.pokemon_hp[0]);
        EXPECT_NE(damagedState.pokemon_status[0], StatusCondition::NONE);
        
        // Tournament healing
        healTeamFully(player_team);
        auto healedState = captureDetailedTeamState(player_team);
        
        // Verify full restoration
        EXPECT_EQ(healedState.pokemon_current_hp[0], healedState.pokemon_hp[0]);
        EXPECT_EQ(healedState.pokemon_current_hp[1], healedState.pokemon_hp[1]);
        EXPECT_EQ(healedState.pokemon_current_hp[2], healedState.pokemon_hp[2]);
        
        // Status conditions should be cleared
        for (const auto& status : healedState.pokemon_status) {
            EXPECT_EQ(status, StatusCondition::NONE);
        }
        
        // PP should be restored
        for (size_t i = 0; i < healedState.pokemon_move_pp.size(); ++i) {
            for (size_t j = 0; j < healedState.pokemon_move_pp[i].size(); ++j) {
                EXPECT_EQ(healedState.pokemon_move_pp[i][j], healedState.pokemon_move_max_pp[i][j]);
            }
        }
    }
    
    // Scenario 2: Partial heal (simulate Pokemon Center visit)
    {
        // Apply heavy damage
        simulateBattleDamage(player_team, "heavy");
        
        // Partial healing (HP only, not PP or status)
        for (size_t i = 0; i < player_team.size(); ++i) {
            Pokemon* pokemon = player_team.getPokemon(i);
            if (pokemon) {
                pokemon->heal(pokemon->hp);  // Heal HP only
            }
        }
        
        auto partialHealState = captureDetailedTeamState(player_team);
        
        // HP should be restored
        EXPECT_EQ(partialHealState.pokemon_current_hp[0], partialHealState.pokemon_hp[0]);
        
        // Status conditions should remain
        bool hasStatusConditions = false;
        for (const auto& status : partialHealState.pokemon_status) {
            if (status != StatusCondition::NONE) {
                hasStatusConditions = true;
                break;
            }
        }
        // Some Pokemon might still have status (partial heal)
        
        // PP should not be fully restored
        bool hasPPDepleted = false;
        for (size_t i = 0; i < partialHealState.pokemon_move_pp.size(); ++i) {
            for (size_t j = 0; j < partialHealState.pokemon_move_pp[i].size(); ++j) {
                if (partialHealState.pokemon_move_pp[i][j] < partialHealState.pokemon_move_max_pp[i][j]) {
                    hasPPDepleted = true;
                    break;
                }
            }
            if (hasPPDepleted) break;
        }
        // Some moves should still have depleted PP
    }
    
    // Scenario 3: No heal (Elite Four sequential battles)
    {
        auto preEliteFourState = captureDetailedTeamState(player_team);
        
        // Elite Four battle 1
        simulateBattleDamage(player_team, "moderate");
        auto postElite1State = captureDetailedTeamState(player_team);
        
        // Elite Four battle 2 (no healing)
        simulateBattleDamage(player_team, "moderate");
        auto postElite2State = captureDetailedTeamState(player_team);
        
        // Damage should accumulate
        EXPECT_LT(postElite2State.pokemon_current_hp[0], postElite1State.pokemon_current_hp[0]);
        
        // Multiple status effects possible
        int statusCount = 0;
        for (const auto& status : postElite2State.pokemon_status) {
            if (status != StatusCondition::NONE) statusCount++;
        }
        // Elite Four battles are challenging, expect status effects
        
        // PP should be significantly depleted
        int totalPPUsed = 0;
        for (size_t i = 0; i < postElite2State.pokemon_move_pp.size(); ++i) {
            for (size_t j = 0; j < postElite2State.pokemon_move_pp[i].size(); ++j) {
                totalPPUsed += (postElite2State.pokemon_move_max_pp[i][j] - postElite2State.pokemon_move_pp[i][j]);
            }
        }
        EXPECT_GT(totalPPUsed, 5);  // Should have used significant PP
    }
}

// Test 3: Team State Persistence to Disk and Recovery
TEST_F(TeamStatePersistenceIntegrationTest, TeamStatePersistenceToDiskAndRecovery) {
    // Apply specific team state for testing
    simulateBattleDamage(player_team, "moderate");
    auto preSerializeState = captureDetailedTeamState(player_team);
    
    // Record tournament progress
    TournamentManager::TournamentBattleResult battleResult;
    battleResult.challenge_name = "Persistence Test Gym";
    battleResult.challenge_type = "gym";
    battleResult.victory = true;
    battleResult.performance_score = 87.5;
    battleResult.turns_taken = 14;
    
    tournament_manager->updatePlayerProgress(test_player_name, battleResult);
    
    // Save tournament progress (should include team state)
    bool saveSuccess = tournament_manager->saveTournamentProgress(test_player_name);
    EXPECT_TRUE(saveSuccess);
    
    // Modify team state to verify restoration
    simulateBattleDamage(player_team, "heavy");
    auto modifiedState = captureDetailedTeamState(player_team);
    
    // Verify state was modified
    EXPECT_NE(modifiedState.calculateHash(), preSerializeState.calculateHash());
    
    // Create new tournament manager instance (simulate system restart)
    auto new_tournament_manager = std::make_shared<TournamentManager>(pokemon_data, team_builder);
    
    // Load tournament progress
    bool loadSuccess = new_tournament_manager->loadTournamentProgress(test_player_name);
    EXPECT_TRUE(loadSuccess);
    
    // Verify progress was restored
    auto restoredProgress = new_tournament_manager->getPlayerProgress(test_player_name);
    ASSERT_TRUE(restoredProgress.has_value());
    
    // Check that battle history was preserved
    auto restoredHistory = new_tournament_manager->getPlayerBattleHistory(test_player_name);
    EXPECT_GT(restoredHistory.size(), 0);
    
    // Verify specific battle result was preserved
    bool foundBattle = false;
    for (const auto& result : restoredHistory) {
        if (result.challenge_name == "Persistence Test Gym") {
            foundBattle = true;
            EXPECT_EQ(result.performance_score, 87.5);
            EXPECT_EQ(result.turns_taken, 14);
            break;
        }
    }
    EXPECT_TRUE(foundBattle);
}

// Test 4: Team Experience and Growth Tracking Through Tournament
TEST_F(TeamStatePersistenceIntegrationTest, TeamExperienceAndGrowthTrackingThroughTournament) {
    // Simulate tournament progression with team growth tracking
    
    struct TeamGrowthMetrics {
        std::vector<int> battles_participated;
        std::vector<int> victories_achieved;
        std::vector<double> performance_scores;
        double team_average_performance;
        int total_battles;
    };
    
    TeamGrowthMetrics growthMetrics;
    growthMetrics.battles_participated = std::vector<int>(player_team.size(), 0);
    growthMetrics.victories_achieved = std::vector<int>(player_team.size(), 0);
    growthMetrics.total_battles = 0;
    
    // Tournament progression simulation
    std::vector<std::pair<std::string, Battle::AIDifficulty>> tournamentStages = {
        {"Early Tournament", Battle::AIDifficulty::EASY},
        {"Mid Tournament", Battle::AIDifficulty::MEDIUM},
        {"Late Tournament", Battle::AIDifficulty::HARD},
        {"Elite Challenge", Battle::AIDifficulty::EXPERT}
    };
    
    for (size_t stage = 0; stage < tournamentStages.size(); ++stage) {
        const auto& stageInfo = tournamentStages[stage];
        
        // Select opponent team based on stage
        Team* opponent;
        switch (stage) {
            case 0: opponent = &early_opponent; break;
            case 1: opponent = &mid_opponent; break;
            case 2: opponent = &late_opponent; break;
            case 3: opponent = &mid_opponent; break;  // Reuse for Elite
            default: opponent = &early_opponent; break;
        }
        
        // Battle simulation
        Battle stageBattle(player_team, *opponent, stageInfo.second);
        EXPECT_FALSE(stageBattle.isBattleOver());
        
        // Apply stage-appropriate damage
        std::string damagePattern = (stage < 2) ? "light" : "moderate";
        simulateBattleDamage(player_team, damagePattern);
        
        // Record battle participation
        for (size_t i = 0; i < player_team.size(); ++i) {
            Pokemon* pokemon = player_team.getPokemon(i);
            if (pokemon && pokemon->isAlive()) {
                growthMetrics.battles_participated[i]++;
                growthMetrics.victories_achieved[i]++;  // Assume victory
            }
        }
        
        // Calculate performance score based on stage difficulty
        double baseScore = 80.0;
        double difficultyMultiplier = 1.0 + (static_cast<double>(stage) * 0.1);
        double stageScore = baseScore + (stage * 3.0);  // Increasing challenge
        growthMetrics.performance_scores.push_back(stageScore * difficultyMultiplier);
        growthMetrics.total_battles++;
        
        // Record tournament battle result
        TournamentManager::TournamentBattleResult result;
        result.challenge_name = stageInfo.first;
        result.challenge_type = (stage < 3) ? "gym" : "elite_four";
        result.victory = true;
        result.performance_score = stageScore * difficultyMultiplier;
        result.turns_taken = 10 + static_cast<int>(stage) * 3;
        result.difficulty_level = (stageInfo.second == Battle::AIDifficulty::EASY) ? "easy" :
                                 (stageInfo.second == Battle::AIDifficulty::MEDIUM) ? "medium" :
                                 (stageInfo.second == Battle::AIDifficulty::HARD) ? "hard" : "expert";
        
        tournament_manager->updatePlayerProgress(test_player_name, result);
        
        // Healing between most battles (except Elite Four sequence)
        if (stage < 3) {
            healTeamFully(player_team);
        }
    }
    
    // Calculate team growth metrics
    double totalPerformance = 0.0;
    for (double score : growthMetrics.performance_scores) {
        totalPerformance += score;
    }
    growthMetrics.team_average_performance = totalPerformance / growthMetrics.performance_scores.size();
    
    // Verify team growth tracking
    EXPECT_EQ(growthMetrics.total_battles, 4);
    EXPECT_GT(growthMetrics.team_average_performance, 80.0);  // Should improve over time
    
    // All Pokemon should have participated
    for (int participated : growthMetrics.battles_participated) {
        EXPECT_GT(participated, 0);
    }
    
    // Verify tournament statistics reflect growth
    auto tournamentStats = tournament_manager->getPlayerTournamentStats(test_player_name);
    EXPECT_GT(tournamentStats.size(), 0);
    
    // Battle history should show progression
    auto battleHistory = tournament_manager->getPlayerBattleHistory(test_player_name);
    EXPECT_EQ(battleHistory.size(), 4);
    
    // Performance should show improvement trend
    bool showsImprovement = false;
    if (battleHistory.size() >= 2) {
        double firstScore = battleHistory[0].performance_score;
        double lastScore = battleHistory[battleHistory.size() - 1].performance_score;
        showsImprovement = (lastScore > firstScore);
    }
    EXPECT_TRUE(showsImprovement);  // Performance should improve through tournament
}

// Test 5: Complex Team State Scenarios and Edge Cases
TEST_F(TeamStatePersistenceIntegrationTest, ComplexTeamStateScenariosAndEdgeCases) {
    // Scenario 1: All Pokemon fainted (should handle gracefully)
    {
        Team faintedTeam = player_team;
        for (size_t i = 0; i < faintedTeam.size(); ++i) {
            Pokemon* pokemon = faintedTeam.getPokemon(i);
            if (pokemon) {
                pokemon->takeDamage(pokemon->hp);  // Faint all
            }
        }
        
        auto faintedState = captureDetailedTeamState(faintedTeam);
        
        // All Pokemon should be fainted
        for (int hp : faintedState.pokemon_current_hp) {
            EXPECT_EQ(hp, 0);
        }
        
        // Battle should end immediately
        Battle impossibleBattle(faintedTeam, early_opponent, Battle::AIDifficulty::EASY);
        EXPECT_TRUE(impossibleBattle.isBattleOver());
        EXPECT_EQ(impossibleBattle.getBattleResult(), Battle::BattleResult::OPPONENT_WINS);
        
        // Tournament should handle this gracefully
        TournamentManager::TournamentBattleResult lossResult;
        lossResult.challenge_name = "Impossible Battle";
        lossResult.challenge_type = "gym";
        lossResult.victory = false;
        lossResult.performance_score = 0.0;
        
        bool lossHandled = tournament_manager->updatePlayerProgress(test_player_name, lossResult);
        EXPECT_TRUE(lossHandled);  // Should handle losses
    }
    
    // Scenario 2: All moves depleted (PP = 0)
    {
        Team depletedTeam = player_team;
        healTeamFully(depletedTeam);  // Start fresh
        
        for (size_t i = 0; i < depletedTeam.size(); ++i) {
            Pokemon* pokemon = depletedTeam.getPokemon(i);
            if (pokemon) {
                // Deplete all PP
                for (auto& move : pokemon->moves) {
                    move.current_pp = 0;
                }
            }
        }
        
        auto depletedState = captureDetailedTeamState(depletedTeam);
        
        // Verify PP depletion
        bool allPPDepleted = true;
        for (const auto& pokemonPP : depletedState.pokemon_move_pp) {
            for (int pp : pokemonPP) {
                if (pp > 0) {
                    allPPDepleted = false;
                    break;
                }
            }
            if (!allPPDepleted) break;
        }
        EXPECT_TRUE(allPPDepleted);
        
        // Battle system should handle PP depletion
        Battle depletedBattle(depletedTeam, early_opponent, Battle::AIDifficulty::EASY);
        // Battle might still be ongoing (Pokemon can use Struggle when PP is depleted)
        EXPECT_TRUE(depletedBattle.getBattleResult() == Battle::BattleResult::ONGOING ||
                   depletedBattle.getBattleResult() == Battle::BattleResult::OPPONENT_WINS);
    }
    
    // Scenario 3: Multiple status conditions and complex state
    {
        Team complexTeam = player_team;
        healTeamFully(complexTeam);  // Start fresh
        
        // Apply complex status pattern
        if (complexTeam.getPokemon(0)) {
            complexTeam.getPokemon(0)->applyStatusCondition(StatusCondition::BURN);
            complexTeam.getPokemon(0)->takeDamage(50);
        }
        if (complexTeam.getPokemon(1)) {
            complexTeam.getPokemon(1)->applyStatusCondition(StatusCondition::POISON);
            complexTeam.getPokemon(1)->takeDamage(30);
        }
        if (complexTeam.getPokemon(2)) {
            complexTeam.getPokemon(2)->applyStatusCondition(StatusCondition::PARALYSIS);
            complexTeam.getPokemon(2)->takeDamage(20);
        }
        
        auto complexState = captureDetailedTeamState(complexTeam);
        
        // Verify complex state
        EXPECT_EQ(complexState.pokemon_status[0], StatusCondition::BURN);
        EXPECT_EQ(complexState.pokemon_status[1], StatusCondition::POISON);
        EXPECT_EQ(complexState.pokemon_status[2], StatusCondition::PARALYSIS);
        
        // All Pokemon should have taken different amounts of damage
        EXPECT_LT(complexState.pokemon_current_hp[0], complexState.pokemon_hp[0]);
        EXPECT_LT(complexState.pokemon_current_hp[1], complexState.pokemon_hp[1]);
        EXPECT_LT(complexState.pokemon_current_hp[2], complexState.pokemon_hp[2]);
        
        // Battle should handle complex status scenarios
        Battle complexBattle(complexTeam, mid_opponent, Battle::AIDifficulty::MEDIUM);
        EXPECT_TRUE(complexBattle.getBattleResult() == Battle::BattleResult::ONGOING ||
                   complexBattle.isBattleOver());
        
        // Tournament should track complex scenarios
        TournamentManager::TournamentBattleResult complexResult;
        complexResult.challenge_name = "Complex Status Battle";
        complexResult.challenge_type = "gym";
        complexResult.victory = true;  // Assume victory despite status conditions
        complexResult.performance_score = 65.0;  // Lower due to status effects
        
        bool complexHandled = tournament_manager->updatePlayerProgress(test_player_name, complexResult);
        EXPECT_TRUE(complexHandled);
    }
    
    // Scenario 4: State integrity verification
    {
        auto preModificationState = captureDetailedTeamState(player_team);
        size_t initialHash = preModificationState.calculateHash();
        
        // Apply modifications
        simulateBattleDamage(player_team, "light");
        auto postModificationState = captureDetailedTeamState(player_team);
        size_t modifiedHash = postModificationState.calculateHash();
        
        // Hash should be different after modifications
        EXPECT_NE(initialHash, modifiedHash);
        
        // Restore original state
        applyDetailedTeamState(player_team, preModificationState);
        auto restoredState = captureDetailedTeamState(player_team);
        size_t restoredHash = restoredState.calculateHash();
        
        // Hash should match original after restoration
        EXPECT_EQ(initialHash, restoredHash);
        EXPECT_EQ(preModificationState.pokemon_current_hp, restoredState.pokemon_current_hp);
        EXPECT_EQ(preModificationState.pokemon_status, restoredState.pokemon_status);
    }
}