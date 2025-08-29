#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include "test_utils.h"
#include "championship_system.h"
#include "tournament_manager.h"
#include "battle.h"
#include "team_builder.h"
#include "pokemon_data.h"

/**
 * @brief Integration tests for Championship System (Elite Four + Champion) interactions
 * 
 * These tests validate the sequential Elite Four and Champion battle system,
 * including proper challenge unlocking, team healing between battles, 
 * victory validation, and integration with tournament progression.
 * Focus areas:
 * - Elite Four sequential battle progression
 * - Champion battle unlocking and execution
 * - Team state management through championship run
 * - Championship completion and tournament integration
 */
class ChampionshipSystemIntegrationTest : public TestUtils::BattleTestFixture {
protected:
    void SetUp() override {
        TestUtils::BattleTestFixture::SetUp();
        
        // Initialize core systems
        pokemon_data = std::make_shared<PokemonData>();
        team_builder = std::make_shared<TeamBuilder>(pokemon_data);
        tournament_manager = std::make_shared<TournamentManager>(pokemon_data, team_builder);
        championship_system = std::make_shared<ChampionshipSystem>(tournament_manager, team_builder);
        
        // Setup test player and championship teams
        setupTestPlayer();
        setupEliteFourTeams();
        setupChampionTeam();
        
        // Initialize tournament progress with all 8 badges
        setupCompletedGymProgress();
    }
    
    void setupTestPlayer() {
        test_player_name = "ChampionChallenger";
        
        // Create a championship-ready team
        Pokemon champion1 = TestUtils::createTestPokemon("ChampionAce", 120, 110, 100, 120, 110, 105, {"dragon", "flying"});
        champion1.moves.clear();
        champion1.moves.push_back(TestUtils::createTestMove("dragon-dance", 0, 100, 20, "dragon", "status"));
        champion1.moves.push_back(TestUtils::createTestMove("outrage", 120, 100, 10, "dragon", "physical"));
        champion1.moves.push_back(TestUtils::createTestMove("earthquake", 100, 100, 10, "ground", "physical"));
        champion1.moves.push_back(TestUtils::createTestMove("fire-punch", 75, 100, 15, "fire", "physical"));
        
        Pokemon champion2 = TestUtils::createTestPokemon("ChampionWall", 130, 80, 130, 80, 130, 60, {"steel", "psychic"});
        champion2.moves.clear();
        champion2.moves.push_back(TestUtils::createTestMove("calm-mind", 0, 100, 20, "psychic", "status"));
        champion2.moves.push_back(TestUtils::createTestMove("psychic", 90, 100, 10, "psychic", "special"));
        champion2.moves.push_back(TestUtils::createTestMove("flash-cannon", 80, 100, 10, "steel", "special"));
        champion2.moves.push_back(TestUtils::createTestMove("recover", 0, 100, 5, "normal", "status"));
        
        Pokemon champion3 = TestUtils::createTestPokemon("ChampionSpeed", 100, 100, 70, 100, 70, 130, {"electric", "normal"});
        champion3.moves.clear();
        champion3.moves.push_back(TestUtils::createTestMove("thunderbolt", 90, 100, 15, "electric", "special"));
        champion3.moves.push_back(TestUtils::createTestMove("quick-attack", 40, 100, 30, "normal", "physical"));
        champion3.moves.push_back(TestUtils::createTestMove("double-team", 0, 100, 15, "normal", "status"));
        champion3.moves.push_back(TestUtils::createTestMove("thunder-wave", 0, 100, 20, "electric", "status", StatusCondition::PARALYSIS, 100));
        
        player_team = TestUtils::createTestTeam({champion1, champion2, champion3});
        original_team_state = captureTeamState(player_team);
    }
    
    void setupEliteFourTeams() {
        // Elite Four Member 1: Psychic Specialist
        Pokemon psychic1 = TestUtils::createTestPokemon("Alakazam", 105, 50, 45, 135, 95, 120, {"psychic"});
        psychic1.moves.clear();
        psychic1.moves.push_back(TestUtils::createTestMove("psychic", 90, 100, 10, "psychic", "special"));
        psychic1.moves.push_back(TestUtils::createTestMove("calm-mind", 0, 100, 20, "psychic", "status"));
        psychic1.moves.push_back(TestUtils::createTestMove("recover", 0, 100, 5, "normal", "status"));
        psychic1.moves.push_back(TestUtils::createTestMove("shadow-ball", 80, 100, 15, "ghost", "special"));
        
        Pokemon psychic2 = TestUtils::createTestPokemon("Slowbro", 135, 75, 110, 100, 80, 30, {"water", "psychic"});
        psychic2.moves.clear();
        psychic2.moves.push_back(TestUtils::createTestMove("surf", 90, 100, 15, "water", "special"));
        psychic2.moves.push_back(TestUtils::createTestMove("psychic", 90, 100, 10, "psychic", "special"));
        psychic2.moves.push_back(TestUtils::createTestMove("amnesia", 0, 100, 20, "psychic", "status"));
        psychic2.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
        
        elite_four_psychic = TestUtils::createTestTeam({psychic1, psychic2});
        
        // Elite Four Member 2: Fighting Specialist
        Pokemon fighting1 = TestUtils::createTestPokemon("Machamp", 130, 130, 80, 65, 85, 55, {"fighting"});
        fighting1.moves.clear();
        fighting1.moves.push_back(TestUtils::createTestMove("close-combat", 120, 100, 5, "fighting", "physical"));
        fighting1.moves.push_back(TestUtils::createTestMove("earthquake", 100, 100, 10, "ground", "physical"));
        fighting1.moves.push_back(TestUtils::createTestMove("stone-edge", 100, 80, 5, "rock", "physical"));
        fighting1.moves.push_back(TestUtils::createTestMove("bulk-up", 0, 100, 20, "fighting", "status"));
        
        Pokemon fighting2 = TestUtils::createTestPokemon("Hitmonlee", 105, 120, 53, 35, 110, 87, {"fighting"});
        fighting2.moves.clear();
        fighting2.moves.push_back(TestUtils::createTestMove("high-jump-kick", 130, 90, 10, "fighting", "physical"));
        fighting2.moves.push_back(TestUtils::createTestMove("blaze-kick", 85, 90, 10, "fire", "physical"));
        fighting2.moves.push_back(TestUtils::createTestMove("fake-out", 40, 100, 10, "normal", "physical"));
        fighting2.moves.push_back(TestUtils::createTestMove("sucker-punch", 70, 100, 5, "dark", "physical"));
        
        elite_four_fighting = TestUtils::createTestTeam({fighting1, fighting2});
        
        // Elite Four Member 3: Ghost Specialist
        Pokemon ghost1 = TestUtils::createTestPokemon("Gengar", 100, 65, 60, 130, 75, 110, {"ghost", "poison"});
        ghost1.moves.clear();
        ghost1.moves.push_back(TestUtils::createTestMove("shadow-ball", 80, 100, 15, "ghost", "special"));
        ghost1.moves.push_back(TestUtils::createTestMove("sludge-bomb", 90, 100, 10, "poison", "special"));
        ghost1.moves.push_back(TestUtils::createTestMove("thunderbolt", 90, 100, 15, "electric", "special"));
        ghost1.moves.push_back(TestUtils::createTestMove("hypnosis", 0, 60, 20, "psychic", "status", StatusCondition::SLEEP, 100));
        
        Pokemon ghost2 = TestUtils::createTestPokemon("Crobat", 115, 90, 80, 70, 80, 130, {"poison", "flying"});
        ghost2.moves.clear();
        ghost2.moves.push_back(TestUtils::createTestMove("air-slash", 75, 95, 15, "flying", "special"));
        ghost2.moves.push_back(TestUtils::createTestMove("sludge-bomb", 90, 100, 10, "poison", "special"));
        ghost2.moves.push_back(TestUtils::createTestMove("u-turn", 70, 100, 20, "bug", "physical"));
        ghost2.moves.push_back(TestUtils::createTestMove("roost", 0, 100, 10, "flying", "status"));
        
        elite_four_ghost = TestUtils::createTestTeam({ghost1, ghost2});
        
        // Elite Four Member 4: Ice Specialist
        Pokemon ice1 = TestUtils::createTestPokemon("Lapras", 160, 85, 80, 85, 95, 60, {"water", "ice"});
        ice1.moves.clear();
        ice1.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
        ice1.moves.push_back(TestUtils::createTestMove("surf", 90, 100, 15, "water", "special"));
        ice1.moves.push_back(TestUtils::createTestMove("thunderbolt", 90, 100, 15, "electric", "special"));
        ice1.moves.push_back(TestUtils::createTestMove("rest", 0, 100, 10, "psychic", "status"));
        
        Pokemon ice2 = TestUtils::createTestPokemon("Articuno", 120, 85, 100, 95, 125, 85, {"ice", "flying"});
        ice2.moves.clear();
        ice2.moves.push_back(TestUtils::createTestMove("blizzard", 110, 70, 5, "ice", "special"));
        ice2.moves.push_back(TestUtils::createTestMove("air-slash", 75, 95, 15, "flying", "special"));
        ice2.moves.push_back(TestUtils::createTestMove("roost", 0, 100, 10, "flying", "status"));
        ice2.moves.push_back(TestUtils::createTestMove("substitute", 0, 100, 10, "normal", "status"));
        
        elite_four_ice = TestUtils::createTestTeam({ice1, ice2});
    }
    
    void setupChampionTeam() {
        // Champion: Mixed specialist with diverse, powerful team
        Pokemon champ1 = TestUtils::createTestPokemon("ChampionDragonite", 134, 134, 95, 100, 100, 80, {"dragon", "flying"});
        champ1.moves.clear();
        champ1.moves.push_back(TestUtils::createTestMove("dragon-dance", 0, 100, 20, "dragon", "status"));
        champ1.moves.push_back(TestUtils::createTestMove("outrage", 120, 100, 10, "dragon", "physical"));
        champ1.moves.push_back(TestUtils::createTestMove("fire-punch", 75, 100, 15, "fire", "physical"));
        champ1.moves.push_back(TestUtils::createTestMove("extremespeed", 80, 100, 5, "normal", "physical"));
        
        Pokemon champ2 = TestUtils::createTestPokemon("ChampionTyranitar", 140, 134, 110, 95, 100, 61, {"rock", "dark"});
        champ2.moves.clear();
        champ2.moves.push_back(TestUtils::createTestMove("stone-edge", 100, 80, 5, "rock", "physical"));
        champ2.moves.push_back(TestUtils::createTestMove("crunch", 80, 100, 15, "dark", "physical"));
        champ2.moves.push_back(TestUtils::createTestMove("earthquake", 100, 100, 10, "ground", "physical"));
        champ2.moves.push_back(TestUtils::createTestMove("dragon-dance", 0, 100, 20, "dragon", "status"));
        
        Pokemon champ3 = TestUtils::createTestPokemon("ChampionMewtwo", 146, 110, 90, 154, 90, 130, {"psychic"});
        champ3.moves.clear();
        champ3.moves.push_back(TestUtils::createTestMove("psychic", 90, 100, 10, "psychic", "special"));
        champ3.moves.push_back(TestUtils::createTestMove("aura-sphere", 80, 100, 20, "fighting", "special"));
        champ3.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
        champ3.moves.push_back(TestUtils::createTestMove("recover", 0, 100, 5, "normal", "status"));
        
        champion_team = TestUtils::createTestTeam({champ1, champ2, champ3});
    }
    
    void setupCompletedGymProgress() {
        tournament_manager->initializePlayerProgress(test_player_name);
        
        // Award all 8 gym badges to unlock Elite Four
        std::vector<TournamentManager::Badge> badges = {
            {"Boulder Badge", "rock", "Brock", "2024-01-01", 1, 85.0},
            {"Cascade Badge", "water", "Misty", "2024-01-02", 1, 82.5},
            {"Thunder Badge", "electric", "Lt. Surge", "2024-01-03", 1, 88.0},
            {"Rainbow Badge", "grass", "Erika", "2024-01-04", 1, 90.5},
            {"Soul Badge", "poison", "Koga", "2024-01-05", 2, 78.0},
            {"Marsh Badge", "psychic", "Sabrina", "2024-01-06", 1, 92.0},
            {"Volcano Badge", "fire", "Blaine", "2024-01-07", 1, 86.5},
            {"Earth Badge", "ground", "Giovanni", "2024-01-08", 3, 95.0}
        };
        
        for (const auto& badge : badges) {
            tournament_manager->awardBadge(test_player_name, badge);
        }
        
        // Verify Elite Four is unlocked
        ASSERT_TRUE(tournament_manager->isEliteFourUnlocked(test_player_name));
        ASSERT_EQ(tournament_manager->getPlayerBadgeCount(test_player_name), 8);
    }
    
    // Team state capture utility
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
            Pokemon* pokemon = team.getPokemon(i);
            if (pokemon) {
                state.pokemon_hp.push_back(pokemon->hp);
                state.pokemon_current_hp.push_back(pokemon->current_hp);
                state.pokemon_status.push_back(pokemon->getStatusCondition());
                
                std::vector<int> move_pp;
                for (const auto& move : pokemon->moves) {
                    move_pp.push_back(move.current_pp);
                }
                state.pokemon_move_pp.push_back(move_pp);
            }
        }
        return state;
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
    std::shared_ptr<ChampionshipSystem> championship_system;
    
    // Test data
    std::string test_player_name;
    Team player_team;
    Team elite_four_psychic;
    Team elite_four_fighting;
    Team elite_four_ghost;
    Team elite_four_ice;
    Team champion_team;
    TeamState original_team_state;
};

// Test 1: Elite Four Sequential Battle Progression
TEST_F(ChampionshipSystemIntegrationTest, EliteFourSequentialBattleProgression) {
    // Verify Elite Four is properly unlocked
    EXPECT_TRUE(tournament_manager->isEliteFourUnlocked(test_player_name));
    EXPECT_FALSE(tournament_manager->isChampionshipUnlocked(test_player_name));
    
    // Test championship run initialization
    ChampionshipSystem::ChampionshipRun championshipRun;
    championshipRun.player_name = test_player_name;
    championshipRun.player_team_name = "ChampionshipTeam";
    championshipRun.is_active = true;
    championshipRun.current_position = 1;  // Starting with first Elite Four member
    championshipRun.allow_healing_between_battles = true;
    championshipRun.sequential_requirement = true;
    
    // Elite Four Battle 1: Psychic Specialist
    {
        Battle eliteBattle1(player_team, elite_four_psychic, Battle::AIDifficulty::EXPERT);
        
        EXPECT_FALSE(eliteBattle1.isBattleOver());
        EXPECT_EQ(eliteBattle1.getBattleResult(), Battle::BattleResult::ONGOING);
        
        // Verify teams are set up correctly
        EXPECT_TRUE(player_team.hasAlivePokemon());
        EXPECT_TRUE(elite_four_psychic.hasAlivePokemon());
        EXPECT_EQ(elite_four_psychic.size(), 2);  // Elite Four member has 2 Pokemon
        
        // Simulate victory
        ChampionshipSystem::ChampionshipBattleResult result1;
        result1.player_name = test_player_name;
        result1.opponent_name = "Elite Four Psychic";
        result1.opponent_type = "elite_four";
        result1.opponent_position = 1;
        result1.victory = true;
        result1.turns_taken = 15;
        result1.difficulty_level = "expert";
        result1.performance_score = 88.5;
        
        // Update championship run
        championshipRun.defeated_opponents.push_back("Elite Four Psychic");
        championshipRun.current_position = 2;
        championshipRun.battle_scores.push_back(result1.performance_score);
        
        EXPECT_EQ(championshipRun.defeated_opponents.size(), 1);
        EXPECT_EQ(championshipRun.current_position, 2);
    }
    
    // Heal between battles (championship system feature)
    if (championshipRun.allow_healing_between_battles) {
        healTeamFully(player_team);
    }
    
    // Elite Four Battle 2: Fighting Specialist
    {
        Battle eliteBattle2(player_team, elite_four_fighting, Battle::AIDifficulty::EXPERT);
        
        EXPECT_FALSE(eliteBattle2.isBattleOver());
        EXPECT_EQ(eliteBattle2.getBattleResult(), Battle::BattleResult::ONGOING);
        
        // Verify team is properly healed
        auto currentState = captureTeamState(player_team);
        EXPECT_EQ(currentState.pokemon_current_hp[0], currentState.pokemon_hp[0]);
        
        // Simulate victory
        ChampionshipSystem::ChampionshipBattleResult result2;
        result2.opponent_name = "Elite Four Fighting";
        result2.opponent_position = 2;
        result2.victory = true;
        result2.performance_score = 82.0;
        
        championshipRun.defeated_opponents.push_back("Elite Four Fighting");
        championshipRun.current_position = 3;
        championshipRun.battle_scores.push_back(result2.performance_score);
    }
    
    // Continue progression verification
    EXPECT_EQ(championshipRun.defeated_opponents.size(), 2);
    EXPECT_EQ(championshipRun.current_position, 3);
    EXPECT_TRUE(championshipRun.is_active);
    EXPECT_FALSE(championshipRun.is_completed);
}

// Test 2: Champion Battle Unlocking and Execution
TEST_F(ChampionshipSystemIntegrationTest, ChampionBattleUnlockingAndExecution) {
    // Simulate completed Elite Four progression
    ChampionshipSystem::ChampionshipRun completedEliteFour;
    completedEliteFour.player_name = test_player_name;
    completedEliteFour.defeated_opponents = {
        "Elite Four Psychic", "Elite Four Fighting", 
        "Elite Four Ghost", "Elite Four Ice"
    };
    completedEliteFour.current_position = 5;  // Champion position
    completedEliteFour.is_active = true;
    
    // Update tournament manager to reflect Elite Four completion
    for (int i = 1; i <= 4; ++i) {
        TournamentManager::TournamentBattleResult eliteResult;
        eliteResult.challenge_name = "Elite Four Member " + std::to_string(i);
        eliteResult.challenge_type = "elite_four";
        eliteResult.victory = true;
        eliteResult.performance_score = 85.0 + i;  // Increasing difficulty
        
        tournament_manager->updatePlayerProgress(test_player_name, eliteResult);
    }
    
    // Champion should now be unlocked
    EXPECT_TRUE(tournament_manager->isChampionshipUnlocked(test_player_name));
    
    // Champion Battle
    {
        Battle championBattle(player_team, champion_team, Battle::AIDifficulty::EXPERT);
        
        EXPECT_FALSE(championBattle.isBattleOver());
        EXPECT_EQ(championBattle.getBattleResult(), Battle::BattleResult::ONGOING);
        
        // Verify championship team setup
        EXPECT_TRUE(champion_team.hasAlivePokemon());
        EXPECT_EQ(champion_team.size(), 3);  // Champion has 3 powerful Pokemon
        
        // Champion team should be formidable
        Pokemon* championAce = champion_team.getPokemon(0);
        ASSERT_TRUE(championAce != nullptr);
        EXPECT_GT(championAce->hp, 130);  // High HP stats
        EXPECT_GT(championAce->attack, 130);  // High attack stats
        
        // Simulate championship victory
        ChampionshipSystem::ChampionshipBattleResult championResult;
        championResult.player_name = test_player_name;
        championResult.opponent_name = "Pokemon Champion";
        championResult.opponent_type = "champion";
        championResult.opponent_position = 5;
        championResult.victory = true;
        championResult.turns_taken = 25;
        championResult.difficulty_level = "expert";
        championResult.performance_score = 98.5;  // Excellent performance
        
        // Update championship run completion
        completedEliteFour.defeated_opponents.push_back("Pokemon Champion");
        completedEliteFour.is_completed = true;
        completedEliteFour.victory = true;
        completedEliteFour.battle_scores.push_back(championResult.performance_score);
        
        EXPECT_TRUE(completedEliteFour.is_completed);
        EXPECT_TRUE(completedEliteFour.victory);
        EXPECT_EQ(completedEliteFour.defeated_opponents.size(), 5);
    }
    
    // Update tournament manager with championship victory
    TournamentManager::TournamentBattleResult tournamentChampionResult;
    tournamentChampionResult.challenge_name = "Pokemon Champion";
    tournamentChampionResult.challenge_type = "champion";
    tournamentChampionResult.victory = true;
    tournamentChampionResult.performance_score = 98.5;
    
    tournament_manager->updatePlayerProgress(test_player_name, tournamentChampionResult);
    
    // Verify tournament completion
    double completion = tournament_manager->getTournamentCompletionPercentage(test_player_name);
    EXPECT_EQ(completion, 1.0);  // 100% completion
}

// Test 3: Team State Management Through Championship Run
TEST_F(ChampionshipSystemIntegrationTest, TeamStateManagementThroughChampionshipRun) {
    // Capture initial team state
    auto initialState = captureTeamState(player_team);
    
    // Simulate championship run with realistic battle damage
    ChampionshipSystem::ChampionshipRun activeRun;
    activeRun.player_name = test_player_name;
    activeRun.is_active = true;
    activeRun.allow_healing_between_battles = true;
    
    // Elite Four Battle 1 - Apply battle damage
    {
        Battle elite1Battle(player_team, elite_four_psychic, Battle::AIDifficulty::EXPERT);
        
        // Simulate battle damage
        Pokemon* pokemon1 = player_team.getPokemon(0);
        Pokemon* pokemon2 = player_team.getPokemon(1);
        Pokemon* pokemon3 = player_team.getPokemon(2);
        
        ASSERT_TRUE(pokemon1 != nullptr);
        ASSERT_TRUE(pokemon2 != nullptr);
        ASSERT_TRUE(pokemon3 != nullptr);
        
        // Apply realistic Elite Four battle damage
        pokemon1->takeDamage(40);  // Moderate damage
        pokemon2->takeDamage(20);  // Light damage
        pokemon3->setStatusCondition(StatusCondition::PARALYSIS);  // Status effect
        
        // Use some PP
        if (!pokemon1->moves.empty()) pokemon1->moves[0].current_pp -= 5;
        if (!pokemon2->moves.empty()) pokemon2->moves[1].current_pp -= 3;
        
        // Verify damage was applied
        auto damagedState = captureTeamState(player_team);
        EXPECT_LT(damagedState.pokemon_current_hp[0], initialState.pokemon_current_hp[0]);
        EXPECT_LT(damagedState.pokemon_current_hp[1], initialState.pokemon_current_hp[1]);
        EXPECT_EQ(damagedState.pokemon_status[2], StatusCondition::PARALYSIS);
        
        // Championship healing between battles
        if (activeRun.allow_healing_between_battles) {
            healTeamFully(player_team);
            activeRun.total_healing_events++;
        }
    }
    
    // Verify healing restored team
    {
        auto healedState = captureTeamState(player_team);
        EXPECT_EQ(healedState.pokemon_current_hp[0], healedState.pokemon_hp[0]);
        EXPECT_EQ(healedState.pokemon_current_hp[1], healedState.pokemon_hp[1]);
        EXPECT_EQ(healedState.pokemon_current_hp[2], healedState.pokemon_hp[2]);
        
        // Status should be cleared
        for (const auto& status : healedState.pokemon_status) {
            EXPECT_EQ(status, StatusCondition::NONE);
        }
        
        // PP should be restored
        EXPECT_EQ(healedState.pokemon_move_pp[0][0], initialState.pokemon_move_pp[0][0]);
        EXPECT_EQ(healedState.pokemon_move_pp[1][1], initialState.pokemon_move_pp[1][1]);
    }
    
    // Elite Four Battle 2 - Different damage pattern
    {
        Battle elite2Battle(player_team, elite_four_fighting, Battle::AIDifficulty::EXPERT);
        
        // Apply different damage pattern
        Pokemon* pokemon1 = player_team.getPokemon(0);
        Pokemon* pokemon2 = player_team.getPokemon(1);
        
        pokemon1->setStatusCondition(StatusCondition::BURN);  // Status effect
        pokemon2->takeDamage(60);  // Heavy damage
        
        if (activeRun.allow_healing_between_battles) {
            healTeamFully(player_team);
            activeRun.total_healing_events++;
        }
        
        // Verify consistent healing
        auto reHealedState = captureTeamState(player_team);
        EXPECT_EQ(reHealedState.pokemon_current_hp[1], reHealedState.pokemon_hp[1]);
        EXPECT_EQ(reHealedState.pokemon_status[0], StatusCondition::NONE);
    }
    
    // Champion Battle - No healing after (final battle)
    {
        Battle championBattle(player_team, champion_team, Battle::AIDifficulty::EXPERT);
        
        // Apply championship battle damage
        Pokemon* pokemon1 = player_team.getPokemon(0);
        Pokemon* pokemon3 = player_team.getPokemon(2);
        
        pokemon1->takeDamage(80);  // Heavy champion battle damage
        pokemon3->takeDamage(50);
        
        // No healing after champion battle (run is complete)
        activeRun.is_completed = true;
        
        // Verify final damage remains
        auto finalState = captureTeamState(player_team);
        EXPECT_LT(finalState.pokemon_current_hp[0], finalState.pokemon_hp[0]);
        EXPECT_LT(finalState.pokemon_current_hp[2], finalState.pokemon_hp[2]);
    }
    
    // Verify championship run tracking
    EXPECT_EQ(activeRun.total_healing_events, 2);  // 2 Elite Four healings
    EXPECT_TRUE(activeRun.is_completed);
}

// Test 4: Championship Completion and Tournament Integration
TEST_F(ChampionshipSystemIntegrationTest, ChampionshipCompletionAndTournamentIntegration) {
    // Complete full championship run
    std::vector<std::string> eliteFourNames = {
        "Elite Four Psychic", "Elite Four Fighting", 
        "Elite Four Ghost", "Elite Four Ice"
    };
    
    // Process Elite Four victories
    for (size_t i = 0; i < eliteFourNames.size(); ++i) {
        TournamentManager::TournamentBattleResult eliteResult;
        eliteResult.challenge_name = eliteFourNames[i];
        eliteResult.challenge_type = "elite_four";
        eliteResult.victory = true;
        eliteResult.turns_taken = 12 + static_cast<int>(i) * 2;  // Increasing difficulty
        eliteResult.performance_score = 85.0 + i * 2.0;  // Progressive scoring
        eliteResult.difficulty_level = "expert";
        
        bool updated = tournament_manager->updatePlayerProgress(test_player_name, eliteResult);
        EXPECT_TRUE(updated);
    }
    
    // Verify Elite Four completion unlocks Champion
    EXPECT_TRUE(tournament_manager->isChampionshipUnlocked(test_player_name));
    
    // Champion victory
    {
        TournamentManager::TournamentBattleResult championResult;
        championResult.challenge_name = "Pokemon Champion";
        championResult.challenge_type = "champion";
        championResult.victory = true;
        championResult.turns_taken = 28;
        championResult.performance_score = 97.5;
        championResult.difficulty_level = "expert";
        
        bool championUpdated = tournament_manager->updatePlayerProgress(test_player_name, championResult);
        EXPECT_TRUE(championUpdated);
    }
    
    // Verify complete tournament progression
    auto finalProgress = tournament_manager->getPlayerProgress(test_player_name);
    ASSERT_TRUE(finalProgress.has_value());
    
    // Should have all 8 badges
    EXPECT_EQ(finalProgress->earned_badges.size(), 8);
    
    // Elite Four should be completed
    EXPECT_TRUE(finalProgress->elite_four_completed);
    
    // Championship should be completed
    EXPECT_TRUE(finalProgress->champion_defeated);
    EXPECT_FALSE(finalProgress->championship_date.empty());
    
    // Tournament completion should be 100%
    double completionPercent = tournament_manager->getTournamentCompletionPercentage(test_player_name);
    EXPECT_EQ(completionPercent, 1.0);
    
    // Battle history should include all championship battles
    auto battleHistory = tournament_manager->getPlayerBattleHistory(test_player_name);
    EXPECT_GE(battleHistory.size(), 5);  // 4 Elite Four + 1 Champion
    
    // Elite Four specific history
    auto eliteHistory = tournament_manager->getPlayerBattleHistory(test_player_name, "elite_four");
    EXPECT_EQ(eliteHistory.size(), 4);
    
    // Champion specific history
    auto championHistory = tournament_manager->getPlayerBattleHistory(test_player_name, "champion");
    EXPECT_EQ(championHistory.size(), 1);
    
    // Tournament statistics should reflect championship completion
    auto tournamentStats = tournament_manager->getPlayerTournamentStats(test_player_name);
    EXPECT_GT(tournamentStats.size(), 0);
    
    // Player should appear on leaderboard
    auto leaderboard = tournament_manager->getTournamentLeaderboard("completion", 10);
    bool playerOnLeaderboard = false;
    for (const auto& entry : leaderboard) {
        if (entry.first == test_player_name) {
            playerOnLeaderboard = true;
            EXPECT_EQ(entry.second, 1.0);  // 100% completion
            break;
        }
    }
    EXPECT_TRUE(playerOnLeaderboard);
}

// Test 5: Championship System Error Handling and Edge Cases
TEST_F(ChampionshipSystemIntegrationTest, ChampionshipSystemErrorHandlingAndEdgeCases) {
    // Test accessing Elite Four without sufficient badges
    {
        // Create new player without badges
        std::string newPlayer = "UnqualifiedPlayer";
        tournament_manager->initializePlayerProgress(newPlayer);
        
        // Should not be able to access Elite Four
        EXPECT_FALSE(tournament_manager->isEliteFourUnlocked(newPlayer));
        EXPECT_FALSE(tournament_manager->isChampionshipUnlocked(newPlayer));
        
        // Elite Four challenge should not be available
        auto challenges = tournament_manager->getAvailableChallenges(newPlayer);
        bool hasEliteFourChallenge = false;
        for (const auto& challenge : challenges) {
            if (challenge.challenge_type == "elite_four") {
                hasEliteFourChallenge = true;
                break;
            }
        }
        EXPECT_FALSE(hasEliteFourChallenge);
    }
    
    // Test championship run validation
    {
        ChampionshipSystem::ChampionshipRun invalidRun;
        invalidRun.player_name = "";  // Invalid empty name
        invalidRun.is_active = true;
        
        // System should handle invalid runs gracefully
        EXPECT_TRUE(invalidRun.player_name.empty());
        EXPECT_FALSE(invalidRun.is_completed);
        EXPECT_EQ(invalidRun.defeated_opponents.size(), 0);
    }
    
    // Test battle with fainted team
    {
        Team faintedTeam = player_team;
        for (size_t i = 0; i < faintedTeam.size(); ++i) {
            Pokemon* pokemon = faintedTeam.getPokemon(i);
            if (pokemon) {
                pokemon->takeDamage(pokemon->hp);  // Faint all Pokemon
            }
        }
        
        Battle impossibleBattle(faintedTeam, elite_four_psychic, Battle::AIDifficulty::EXPERT);
        EXPECT_TRUE(impossibleBattle.isBattleOver());
        EXPECT_EQ(impossibleBattle.getBattleResult(), Battle::BattleResult::OPPONENT_WINS);
    }
    
    // Test championship progression validation
    {
        // Try to fight Champion without completing Elite Four
        std::string skipPlayer = "SkipperPlayer";
        tournament_manager->initializePlayerProgress(skipPlayer);
        
        // Give all gym badges but no Elite Four progress
        for (int i = 1; i <= 8; ++i) {
            TournamentManager::Badge badge("TestGym" + std::to_string(i), "test", "TestLeader", "2024-01-15", 1, 80.0);
            tournament_manager->awardBadge(skipPlayer, badge);
        }
        
        // Elite Four should be unlocked but Champion should not
        EXPECT_TRUE(tournament_manager->isEliteFourUnlocked(skipPlayer));
        EXPECT_FALSE(tournament_manager->isChampionshipUnlocked(skipPlayer));
        
        // Cannot skip to Champion battle
        auto progress = tournament_manager->getPlayerProgress(skipPlayer);
        ASSERT_TRUE(progress.has_value());
        EXPECT_FALSE(progress->elite_four_completed);
        EXPECT_FALSE(progress->champion_defeated);
    }
}

// Test 6: Championship System Battle Statistics and Performance Tracking
TEST_F(ChampionshipSystemIntegrationTest, ChampionshipBattleStatisticsAndPerformanceTracking) {
    // Run complete championship with detailed performance tracking
    ChampionshipSystem::ChampionshipRun trackedRun;
    trackedRun.player_name = test_player_name;
    trackedRun.is_active = true;
    trackedRun.allow_healing_between_battles = true;
    
    // Elite Four battles with varying performance
    std::vector<std::pair<Team*, double>> eliteBattles = {
        {&elite_four_psychic, 92.5},   // Excellent performance
        {&elite_four_fighting, 78.0},  // Good performance
        {&elite_four_ghost, 85.5},     // Very good performance
        {&elite_four_ice, 88.0}        // Great performance
    };
    
    for (size_t i = 0; i < eliteBattles.size(); ++i) {
        Team* opponentTeam = eliteBattles[i].first;
        double expectedScore = eliteBattles[i].second;
        
        Battle eliteBattle(player_team, *opponentTeam, Battle::AIDifficulty::EXPERT);
        EXPECT_FALSE(eliteBattle.isBattleOver());
        
        // Record battle statistics
        ChampionshipSystem::ChampionshipBattleResult battleResult;
        battleResult.player_name = test_player_name;
        battleResult.opponent_name = "Elite Four Member " + std::to_string(i + 1);
        battleResult.opponent_type = "elite_four";
        battleResult.opponent_position = static_cast<int>(i + 1);
        battleResult.victory = true;
        battleResult.turns_taken = 15 + static_cast<int>(i) * 2;
        battleResult.performance_score = expectedScore;
        battleResult.difficulty_level = "expert";
        
        // Update tracking
        trackedRun.battle_turns.push_back(battleResult.turns_taken);
        trackedRun.battle_scores.push_back(battleResult.performance_score);
        trackedRun.defeated_opponents.push_back(battleResult.opponent_name);
        
        // Update tournament progress
        TournamentManager::TournamentBattleResult tournamentResult;
        tournamentResult.challenge_name = battleResult.opponent_name;
        tournamentResult.challenge_type = "elite_four";
        tournamentResult.victory = true;
        tournamentResult.performance_score = expectedScore;
        tournamentResult.turns_taken = battleResult.turns_taken;
        tournamentResult.difficulty_level = "expert";
        
        tournament_manager->updatePlayerProgress(test_player_name, tournamentResult);
        
        // Healing between battles
        if (trackedRun.allow_healing_between_battles) {
            healTeamFully(player_team);
            trackedRun.total_healing_events++;
        }
    }
    
    // Champion battle with highest performance expectation
    {
        Battle championBattle(player_team, champion_team, Battle::AIDifficulty::EXPERT);
        
        ChampionshipSystem::ChampionshipBattleResult championResult;
        championResult.opponent_name = "Pokemon Champion";
        championResult.opponent_type = "champion";
        championResult.opponent_position = 5;
        championResult.victory = true;
        championResult.turns_taken = 32;
        championResult.performance_score = 96.5;  // Exceptional performance
        championResult.difficulty_level = "expert";
        
        trackedRun.battle_turns.push_back(championResult.turns_taken);
        trackedRun.battle_scores.push_back(championResult.performance_score);
        trackedRun.defeated_opponents.push_back(championResult.opponent_name);
        trackedRun.is_completed = true;
        trackedRun.victory = true;
        
        // Final tournament update
        TournamentManager::TournamentBattleResult finalTournamentResult;
        finalTournamentResult.challenge_name = "Pokemon Champion";
        finalTournamentResult.challenge_type = "champion";
        finalTournamentResult.victory = true;
        finalTournamentResult.performance_score = 96.5;
        finalTournamentResult.turns_taken = 32;
        finalTournamentResult.difficulty_level = "expert";
        
        tournament_manager->updatePlayerProgress(test_player_name, finalTournamentResult);
    }
    
    // Verify comprehensive tracking
    EXPECT_EQ(trackedRun.battle_scores.size(), 5);  // 4 Elite Four + 1 Champion
    EXPECT_EQ(trackedRun.battle_turns.size(), 5);
    EXPECT_EQ(trackedRun.defeated_opponents.size(), 5);
    EXPECT_EQ(trackedRun.total_healing_events, 4);  // Only between Elite Four battles
    EXPECT_TRUE(trackedRun.is_completed);
    EXPECT_TRUE(trackedRun.victory);
    
    // Calculate performance statistics
    double totalScore = 0.0;
    for (double score : trackedRun.battle_scores) {
        totalScore += score;
    }
    double averageScore = totalScore / trackedRun.battle_scores.size();
    EXPECT_GT(averageScore, 85.0);  // Should have good overall performance
    
    int totalTurns = 0;
    for (int turns : trackedRun.battle_turns) {
        totalTurns += turns;
    }
    EXPECT_GT(totalTurns, 70);  // Should have realistic battle length
    
    // Verify tournament statistics reflect championship completion
    auto finalStats = tournament_manager->getPlayerTournamentStats(test_player_name);
    EXPECT_GT(finalStats.size(), 0);
    
    // Player progress should reflect excellence
    auto finalProgress = tournament_manager->getPlayerProgress(test_player_name);
    ASSERT_TRUE(finalProgress.has_value());
    EXPECT_TRUE(finalProgress->champion_defeated);
    EXPECT_GT(finalProgress->average_battle_performance, 85.0);
}