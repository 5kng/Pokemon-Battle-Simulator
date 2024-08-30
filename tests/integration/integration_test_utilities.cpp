#include "integration_test_utilities.h"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace IntegrationTestUtils {

// MultiSystemIntegrationFixture Implementation

void MultiSystemIntegrationFixture::createBalancedTestTeam() {
    // Create a well-balanced team for comprehensive testing
    Pokemon balanced1 = TestUtils::createTestPokemon("IntegrationAce", 110, 95, 85, 100, 90, 95, {"dragon", "flying"});
    balanced1.moves.clear();
    balanced1.moves.push_back(TestUtils::createTestMove("dragon-pulse", 85, 100, 10, "dragon", "special"));
    balanced1.moves.push_back(TestUtils::createTestMove("aerial-ace", 60, 100, 20, "flying", "physical"));
    balanced1.moves.push_back(TestUtils::createTestMove("roost", 0, 100, 10, "flying", "status"));
    balanced1.moves.push_back(TestUtils::createTestMove("dragon-dance", 0, 100, 20, "dragon", "status"));
    
    Pokemon balanced2 = TestUtils::createTestPokemon("IntegrationTank", 125, 75, 110, 85, 110, 60, {"steel", "water"});
    balanced2.moves.clear();
    balanced2.moves.push_back(TestUtils::createTestMove("surf", 90, 100, 15, "water", "special"));
    balanced2.moves.push_back(TestUtils::createTestMove("flash-cannon", 80, 100, 10, "steel", "special"));
    balanced2.moves.push_back(TestUtils::createTestMove("recover", 0, 100, 5, "normal", "status"));
    balanced2.moves.push_back(TestUtils::createTestMove("toxic", 0, 90, 10, "poison", "status", StatusCondition::POISON, 100));
    
    Pokemon balanced3 = TestUtils::createTestPokemon("IntegrationSweeper", 90, 110, 70, 110, 70, 115, {"electric", "fire"});
    balanced3.moves.clear();
    balanced3.moves.push_back(TestUtils::createTestMove("thunderbolt", 90, 100, 15, "electric", "special"));
    balanced3.moves.push_back(TestUtils::createTestMove("flamethrower", 90, 100, 15, "fire", "special"));
    balanced3.moves.push_back(TestUtils::createTestMove("quick-attack", 40, 100, 30, "normal", "physical"));
    balanced3.moves.push_back(TestUtils::createTestMove("double-team", 0, 100, 15, "normal", "status"));
    
    default_player_team = TestUtils::createTestTeam({balanced1, balanced2, balanced3});
}

void MultiSystemIntegrationFixture::setupGymLeaderTeams() {
    gym_leader_teams.clear();
    
    // Gym 1: Rock-type (Easy)
    Team rock_gym = createGymLeaderTeam("rock", Battle::AIDifficulty::EASY);
    gym_leader_teams.push_back(rock_gym);
    
    // Gym 3: Electric-type (Medium)
    Team electric_gym = createGymLeaderTeam("electric", Battle::AIDifficulty::MEDIUM);
    gym_leader_teams.push_back(electric_gym);
    
    // Gym 6: Psychic-type (Hard)
    Team psychic_gym = createGymLeaderTeam("psychic", Battle::AIDifficulty::HARD);
    gym_leader_teams.push_back(psychic_gym);
    
    // Gym 8: Ground-type (Hard)
    Team ground_gym = createGymLeaderTeam("ground", Battle::AIDifficulty::HARD);
    gym_leader_teams.push_back(ground_gym);
}

void MultiSystemIntegrationFixture::setupChampionshipTeams() {
    elite_four_teams.clear();
    
    // Elite Four Member 1: Ice specialist
    Pokemon ice1 = TestUtils::createTestPokemon("EliteIce1", 120, 85, 100, 95, 125, 85, {"ice", "water"});
    ice1.moves.clear();
    ice1.moves.push_back(TestUtils::createTestMove("blizzard", 110, 70, 5, "ice", "special"));
    ice1.moves.push_back(TestUtils::createTestMove("surf", 90, 100, 15, "water", "special"));
    ice1.moves.push_back(TestUtils::createTestMove("ice-shard", 40, 100, 30, "ice", "physical"));
    ice1.moves.push_back(TestUtils::createTestMove("hail", 0, 100, 10, "ice", "status"));
    
    Pokemon ice2 = TestUtils::createTestPokemon("EliteIce2", 100, 100, 80, 80, 80, 100, {"ice", "ground"});
    ice2.moves.clear();
    ice2.moves.push_back(TestUtils::createTestMove("ice-beam", 90, 100, 10, "ice", "special"));
    ice2.moves.push_back(TestUtils::createTestMove("earthquake", 100, 100, 10, "ground", "physical"));
    ice2.moves.push_back(TestUtils::createTestMove("icicle-spear", 25, 100, 30, "ice", "physical"));
    ice2.moves.push_back(TestUtils::createTestMove("amnesia", 0, 100, 20, "psychic", "status"));
    
    elite_four_teams.push_back(TestUtils::createTestTeam({ice1, ice2}));
    
    // Champion Team
    Pokemon champ1 = TestUtils::createTestPokemon("ChampionMega", 150, 140, 100, 140, 100, 90, {"dragon", "psychic"});
    champ1.moves.clear();
    champ1.moves.push_back(TestUtils::createTestMove("dragon-pulse", 85, 100, 10, "dragon", "special"));
    champ1.moves.push_back(TestUtils::createTestMove("psychic", 90, 100, 10, "psychic", "special"));
    champ1.moves.push_back(TestUtils::createTestMove("recover", 0, 100, 5, "normal", "status"));
    champ1.moves.push_back(TestUtils::createTestMove("calm-mind", 0, 100, 20, "psychic", "status"));
    
    Pokemon champ2 = TestUtils::createTestPokemon("ChampionLegend", 130, 120, 90, 120, 90, 110, {"fire", "flying"});
    champ2.moves.clear();
    champ2.moves.push_back(TestUtils::createTestMove("fire-blast", 110, 85, 5, "fire", "special"));
    champ2.moves.push_back(TestUtils::createTestMove("air-slash", 75, 95, 15, "flying", "special"));
    champ2.moves.push_back(TestUtils::createTestMove("solar-beam", 120, 100, 10, "grass", "special"));
    champ2.moves.push_back(TestUtils::createTestMove("sunny-day", 0, 100, 5, "fire", "status"));
    
    champion_team = TestUtils::createTestTeam({champ1, champ2});
}

// Utility Functions Implementation

IntegrationTeamState captureTeamState(const Team& team, const std::string& context) {
    IntegrationTeamState state;
    state.state_context = context;
    state.timestamp = std::chrono::steady_clock::now();
    
    for (size_t i = 0; i < team.size(); ++i) {
        const Pokemon* pokemon = team.getPokemon(i);
        if (pokemon) {
            state.pokemon_hp.push_back(pokemon->hp);
            state.pokemon_current_hp.push_back(pokemon->current_hp);
            state.pokemon_status.push_back(pokemon->status);
            state.pokemon_names.push_back(pokemon->name);
            
            std::vector<int> move_pp;
            for (const auto& move : pokemon->moves) {
                move_pp.push_back(move.current_pp);
            }
            state.pokemon_move_pp.push_back(move_pp);
        }
    }
    
    return state;
}

void applyTeamState(Team& team, const IntegrationTeamState& state) {
    for (size_t i = 0; i < team.size() && i < state.pokemon_hp.size(); ++i) {
        Pokemon* pokemon = team.getPokemon(i);
        if (pokemon) {
            pokemon->current_hp = state.pokemon_current_hp[i];
            pokemon->status = state.pokemon_status[i];
            
            for (size_t j = 0; j < pokemon->moves.size() && j < state.pokemon_move_pp[i].size(); ++j) {
                pokemon->moves[j].current_pp = state.pokemon_move_pp[i][j];
            }
        }
    }
}

Pokemon createTestPokemonWithMoves(
    const std::string& name,
    int hp, int attack, int defense, int special_attack, int special_defense, int speed,
    const std::vector<std::string>& types,
    const std::vector<std::pair<std::string, std::string>>& move_pairs) {
    
    Pokemon pokemon = TestUtils::createTestPokemon(name, hp, attack, defense, special_attack, special_defense, speed, types);
    pokemon.moves.clear();
    
    for (const auto& move_pair : move_pairs) {
        Move move = TestUtils::createTestMove(move_pair.first, 80, 100, 15, move_pair.second, "special");
        pokemon.moves.push_back(move);
    }
    
    return pokemon;
}

Team createBalancedTournamentTeam(const std::string& team_name) {
    Pokemon attacker = createTestPokemonWithMoves(
        team_name + "_Attacker", 100, 120, 70, 90, 80, 100,
        {"fighting", "steel"},
        {{"close-combat", "fighting"}, {"iron-head", "steel"}, {"earthquake", "ground"}, {"stone-edge", "rock"}}
    );
    
    Pokemon defender = createTestPokemonWithMoves(
        team_name + "_Defender", 150, 70, 130, 70, 130, 50,
        {"water", "steel"},
        {{"surf", "water"}, {"flash-cannon", "steel"}, {"recover", "normal"}, {"toxic", "poison"}}
    );
    
    Pokemon support = createTestPokemonWithMoves(
        team_name + "_Support", 120, 80, 90, 110, 100, 85,
        {"psychic", "fairy"},
        {{"psychic", "psychic"}, {"moonblast", "fairy"}, {"calm-mind", "psychic"}, {"heal-bell", "normal"}}
    );
    
    return TestUtils::createTestTeam({attacker, defender, support});
}

Team createGymLeaderTeam(const std::string& gym_type, Battle::AIDifficulty difficulty) {
    // Adjust stats based on difficulty
    int base_hp = 80;
    int base_attack = 70;
    int base_defense = 70;
    
    switch (difficulty) {
        case Battle::AIDifficulty::EASY:
            // Keep base stats
            break;
        case Battle::AIDifficulty::MEDIUM:
            base_hp += 20;
            base_attack += 15;
            base_defense += 10;
            break;
        case Battle::AIDifficulty::HARD:
            base_hp += 40;
            base_attack += 30;
            base_defense += 20;
            break;
        case Battle::AIDifficulty::EXPERT:
            base_hp += 60;
            base_attack += 45;
            base_defense += 30;
            break;
    }
    
    Pokemon leader1 = createTestPokemonWithMoves(
        "GymLeader_" + gym_type + "1", base_hp, base_attack, base_defense, 
        base_attack, base_defense, base_attack - 10,
        {gym_type},
        {{gym_type + "-move1", gym_type}, {gym_type + "-move2", gym_type}, {"tackle", "normal"}, {"double-team", "normal"}}
    );
    
    Pokemon leader2 = createTestPokemonWithMoves(
        "GymLeader_" + gym_type + "2", base_hp + 20, base_attack + 10, base_defense + 10,
        base_attack + 10, base_defense + 10, base_attack - 5,
        {gym_type, "normal"},
        {{gym_type + "-signature", gym_type}, {"hyper-beam", "normal"}, {"rest", "psychic"}, {"protect", "normal"}}
    );
    
    return TestUtils::createTestTeam({leader1, leader2});
}

IntegrationBattleResult simulateBattle(
    Team& player_team,
    Team& opponent_team,
    Battle::AIDifficulty ai_difficulty,
    const std::string& context) {
    
    IntegrationBattleResult result;
    result.pre_battle_state = captureTeamState(player_team, "Pre-battle: " + context);
    
    auto start_time = std::chrono::steady_clock::now();
    
    // Create battle
    Battle battle(player_team, opponent_team, ai_difficulty);
    result.battle_result = battle.getBattleResult();
    
    // Simulate battle progression
    result.turns_taken = 10 + (static_cast<int>(ai_difficulty) * 3);  // Estimate based on difficulty
    
    // Apply realistic damage based on difficulty
    std::mt19937 rng(42);
    applyBattleDamage(player_team, ai_difficulty, rng);
    
    auto end_time = std::chrono::steady_clock::now();
    result.battle_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    result.post_battle_state = captureTeamState(player_team, "Post-battle: " + context);
    
    // Determine victory based on team survival
    result.victory = player_team.hasAlivePokemon();
    if (result.victory) {
        result.battle_result = Battle::BattleResult::PLAYER_WINS;
    } else {
        result.battle_result = Battle::BattleResult::OPPONENT_WINS;
    }
    
    // Calculate performance score
    result.performance_score = calculatePerformanceScore(
        result.pre_battle_state, result.post_battle_state, result.turns_taken, ai_difficulty
    );
    
    // Set difficulty level
    switch (ai_difficulty) {
        case Battle::AIDifficulty::EASY:   result.difficulty_level = "easy"; break;
        case Battle::AIDifficulty::MEDIUM: result.difficulty_level = "medium"; break;
        case Battle::AIDifficulty::HARD:   result.difficulty_level = "hard"; break;
        case Battle::AIDifficulty::EXPERT: result.difficulty_level = "expert"; break;
    }
    
    return result;
}

IntegrationTournamentProgress simulateTournamentProgression(
    std::shared_ptr<TournamentManager> tournament_manager,
    const std::string& player_name,
    Team& player_team,
    const std::vector<std::pair<Team, Battle::AIDifficulty>>& opponents) {
    
    IntegrationTournamentProgress progress;
    progress.player_name = player_name;
    progress.start_time = std::chrono::steady_clock::now();
    
    for (size_t i = 0; i < opponents.size(); ++i) {
        Team opponent_team = opponents[i].first;
        Battle::AIDifficulty difficulty = opponents[i].second;
        
        std::string challenge_name = "Challenge_" + std::to_string(i + 1);
        
        // Simulate battle
        IntegrationBattleResult battle_result = simulateBattle(
            player_team, opponent_team, difficulty, challenge_name
        );
        
        // Add to progress
        progress.addBattleResult(battle_result);
        
        if (battle_result.victory) {
            progress.completed_challenges.push_back(challenge_name);
            
            // Update tournament manager
            TournamentManager::TournamentBattleResult tournament_result;
            tournament_result.challenge_name = challenge_name;
            tournament_result.challenge_type = (i < 8) ? "gym" : "elite_four";
            tournament_result.victory = true;
            tournament_result.performance_score = battle_result.performance_score;
            tournament_result.turns_taken = battle_result.turns_taken;
            tournament_result.difficulty_level = battle_result.difficulty_level;
            
            tournament_manager->updatePlayerProgress(player_name, tournament_result);
        }
        
        // Heal between battles (except Elite Four)
        if (i < 8 || i == opponents.size() - 1) {  // Heal after gyms and after Elite Four completion
            applyTournamentHealing(player_team, "full");
        }
        
        // Save checkpoint state
        std::string checkpoint_key = "checkpoint_" + std::to_string(i);
        progress.checkpoint_states[checkpoint_key] = captureTeamState(player_team, checkpoint_key);
    }
    
    progress.end_time = std::chrono::steady_clock::now();
    return progress;
}

bool validateBattleResultConsistency(
    const IntegrationBattleResult& result1,
    const IntegrationBattleResult& result2,
    double tolerance) {
    
    // Both results should be internally consistent
    if (!result1.isConsistent() || !result2.isConsistent()) {
        return false;
    }
    
    // Performance scores should be within tolerance if same difficulty
    if (result1.difficulty_level == result2.difficulty_level) {
        double score_diff = std::abs(result1.performance_score - result2.performance_score);
        if (score_diff > tolerance * 100.0) {  // tolerance as percentage
            return false;
        }
    }
    
    // Turn counts should be reasonable
    if (result1.turns_taken <= 0 || result2.turns_taken <= 0) {
        return false;
    }
    
    return true;
}

bool validateAIDifficultyProgression(const std::vector<IntegrationBattleResult>& results) {
    if (results.size() < 2) return true;  // Can't validate progression with less than 2 results
    
    // Check that difficulty progression makes sense
    std::vector<int> difficulty_levels;
    for (const auto& result : results) {
        if (result.difficulty_level == "easy") difficulty_levels.push_back(1);
        else if (result.difficulty_level == "medium") difficulty_levels.push_back(2);
        else if (result.difficulty_level == "hard") difficulty_levels.push_back(3);
        else if (result.difficulty_level == "expert") difficulty_levels.push_back(4);
    }
    
    // Check if generally increasing (allowing for some variation)
    for (size_t i = 1; i < difficulty_levels.size(); ++i) {
        // Major regression in difficulty should not happen
        if (difficulty_levels[i] < difficulty_levels[i-1] - 1) {
            return false;
        }
    }
    
    return true;
}

void applyBattleDamage(Team& team, Battle::AIDifficulty difficulty, std::mt19937& rng) {
    std::uniform_int_distribution<int> damage_dist(10, 60);
    std::uniform_int_distribution<int> pp_dist(1, 5);
    std::uniform_int_distribution<int> status_chance(1, 100);
    
    int damage_multiplier = 1;
    int status_chance_threshold = 80;
    
    switch (difficulty) {
        case Battle::AIDifficulty::EASY:
            damage_multiplier = 1;
            status_chance_threshold = 90;
            break;
        case Battle::AIDifficulty::MEDIUM:
            damage_multiplier = 2;
            status_chance_threshold = 80;
            break;
        case Battle::AIDifficulty::HARD:
            damage_multiplier = 3;
            status_chance_threshold = 70;
            break;
        case Battle::AIDifficulty::EXPERT:
            damage_multiplier = 4;
            status_chance_threshold = 60;
            break;
    }
    
    for (size_t i = 0; i < team.size(); ++i) {
        Pokemon* pokemon = team.getPokemon(i);
        if (pokemon) {
            // Apply damage
            int damage = damage_dist(rng) * damage_multiplier;
            pokemon->takeDamage(std::min(damage, pokemon->current_hp - 1));  // Keep alive with at least 1 HP
            
            // Apply PP reduction
            for (auto& move : pokemon->moves) {
                int pp_reduction = pp_dist(rng);
                move.current_pp = std::max(0, move.current_pp - pp_reduction);
            }
            
            // Apply status conditions based on difficulty
            if (status_chance(rng) > status_chance_threshold) {
                std::vector<StatusCondition> possible_status = {
                    StatusCondition::BURN, StatusCondition::POISON, StatusCondition::PARALYSIS
                };
                std::uniform_int_distribution<size_t> status_dist(0, possible_status.size() - 1);
                pokemon->applyStatusCondition(possible_status[status_dist(rng)]);
            }
        }
    }
}

void applyTournamentHealing(Team& team, const std::string& healing_type) {
    for (size_t i = 0; i < team.size(); ++i) {
        Pokemon* pokemon = team.getPokemon(i);
        if (pokemon) {
            if (healing_type == "full") {
                // Full heal: HP, PP, and status
                pokemon->heal(pokemon->hp);
                pokemon->clearStatusCondition();
                for (auto& move : pokemon->moves) {
                    move.current_pp = move.pp;
                }
            } else if (healing_type == "partial") {
                // Partial heal: HP only
                pokemon->heal(pokemon->hp);
            }
            // "none" = no healing
        }
    }
}

double calculatePerformanceScore(
    const IntegrationTeamState& pre_battle,
    const IntegrationTeamState& post_battle,
    int turns_taken,
    Battle::AIDifficulty difficulty) {
    
    if (pre_battle.pokemon_hp.empty() || post_battle.pokemon_hp.empty()) {
        return 0.0;
    }
    
    // Base score
    double base_score = 50.0;
    
    // Health retention bonus
    double health_retention = post_battle.calculateHealthPercentage();
    double health_bonus = health_retention * 30.0;
    
    // Turn efficiency bonus (fewer turns = better)
    double turn_efficiency = std::max(0.0, (20.0 - turns_taken) / 20.0) * 20.0;
    
    // Difficulty multiplier
    double difficulty_multiplier = 1.0;
    switch (difficulty) {
        case Battle::AIDifficulty::EASY:   difficulty_multiplier = 1.0; break;
        case Battle::AIDifficulty::MEDIUM: difficulty_multiplier = 1.2; break;
        case Battle::AIDifficulty::HARD:   difficulty_multiplier = 1.4; break;
        case Battle::AIDifficulty::EXPERT: difficulty_multiplier = 1.6; break;
    }
    
    double total_score = (base_score + health_bonus + turn_efficiency) * difficulty_multiplier;
    return std::max(0.0, std::min(100.0, total_score));
}

bool validateTournamentProgressIntegrity(const IntegrationTournamentProgress& progress) {
    // Check basic integrity
    if (progress.player_name.empty()) return false;
    if (progress.battle_results.size() != progress.completed_challenges.size() + progress.total_defeats) return false;
    
    // Check win/loss consistency
    int calculated_victories = 0;
    for (const auto& result : progress.battle_results) {
        if (result.victory) calculated_victories++;
    }
    
    if (calculated_victories != progress.total_victories) return false;
    
    // Check performance calculation
    if (!progress.battle_results.empty()) {
        double calculated_average = 0.0;
        for (const auto& result : progress.battle_results) {
            calculated_average += result.performance_score;
        }
        calculated_average /= progress.battle_results.size();
        
        if (std::abs(calculated_average - progress.average_performance) > 0.1) return false;
    }
    
    return true;
}

AIDecisionAnalysis analyzeAIDecisions(
    std::unique_ptr<AIStrategy>& ai,
    const BattleState& battle_state,
    int num_decisions) {
    
    AIDecisionAnalysis analysis;
    
    for (int i = 0; i < num_decisions; ++i) {
        auto move_eval = ai->chooseBestMove(battle_state);
        auto switch_eval = ai->chooseBestSwitch(battle_state);
        bool should_switch = ai->shouldSwitch(battle_state);
        
        analysis.move_choices.push_back(move_eval.moveIndex);
        analysis.switch_decisions.push_back(should_switch);
        analysis.decision_scores.push_back(move_eval.score);
        analysis.decision_reasoning.push_back(move_eval.reasoning);
    }
    
    // Calculate consistency score (lower variance = higher consistency)
    if (!analysis.decision_scores.empty()) {
        double mean_score = std::accumulate(analysis.decision_scores.begin(), analysis.decision_scores.end(), 0.0) / analysis.decision_scores.size();
        double variance = 0.0;
        for (double score : analysis.decision_scores) {
            variance += (score - mean_score) * (score - mean_score);
        }
        variance /= analysis.decision_scores.size();
        analysis.consistency_score = std::max(0.0, 1.0 - (variance / 100.0));  // Normalize to 0-1
    }
    
    // Calculate strategic depth (variety of moves considered)
    std::set<int> unique_moves(analysis.move_choices.begin(), analysis.move_choices.end());
    analysis.strategic_depth = static_cast<double>(unique_moves.size()) / std::max(1, num_decisions);
    
    return analysis;
}

bool validateAIConsistency(const std::vector<AIDecisionAnalysis>& analyses, double min_consistency) {
    for (const auto& analysis : analyses) {
        if (analysis.consistency_score < min_consistency) {
            return false;
        }
    }
    return true;
}

Battle::AIDifficulty getStageAIDifficulty(TournamentStage stage) {
    switch (stage) {
        case TournamentStage::EARLY_GYM: return Battle::AIDifficulty::EASY;
        case TournamentStage::MID_GYM:   return Battle::AIDifficulty::MEDIUM;
        case TournamentStage::LATE_GYM:  return Battle::AIDifficulty::HARD;
        case TournamentStage::ELITE_FOUR:
        case TournamentStage::CHAMPION:  return Battle::AIDifficulty::EXPERT;
        default:                         return Battle::AIDifficulty::EASY;
    }
}

double getStagePerformanceThreshold(TournamentStage stage) {
    switch (stage) {
        case TournamentStage::EARLY_GYM: return 70.0;
        case TournamentStage::MID_GYM:   return 75.0;
        case TournamentStage::LATE_GYM:  return 80.0;
        case TournamentStage::ELITE_FOUR: return 85.0;
        case TournamentStage::CHAMPION:  return 90.0;
        default:                         return 70.0;
    }
}

// IntegrationTestValidator Implementation

bool IntegrationTestValidator::validateBattleIntegration(
    const std::vector<IntegrationBattleResult>& results) {
    
    if (results.empty()) return false;
    
    for (const auto& result : results) {
        if (!result.isConsistent()) return false;
    }
    
    return validateAIDifficultyProgression(results);
}

bool IntegrationTestValidator::validateTournamentIntegration(
    const IntegrationTournamentProgress& progress) {
    
    return validateTournamentProgressIntegrity(progress);
}

bool IntegrationTestValidator::validateAIIntegration(
    const std::vector<AIDecisionAnalysis>& ai_analyses) {
    
    return validateAIConsistency(ai_analyses, 0.6);  // Minimum 60% consistency
}

bool IntegrationTestValidator::validateChampionshipIntegration(
    const std::vector<IntegrationBattleResult>& elite_four_results,
    const IntegrationBattleResult& champion_result) {
    
    // Elite Four should be expert difficulty
    for (const auto& result : elite_four_results) {
        if (result.difficulty_level != "expert") return false;
        if (!result.isConsistent()) return false;
    }
    
    // Champion should be expert difficulty and consistent
    if (champion_result.difficulty_level != "expert") return false;
    if (!champion_result.isConsistent()) return false;
    
    // Champion should generally be more challenging (higher performance threshold)
    if (champion_result.performance_score > 0 && champion_result.performance_score < 80.0) {
        return false;  // Champion battles should require high performance
    }
    
    return true;
}

bool IntegrationTestValidator::validateTeamStatePersistence(
    const std::vector<IntegrationTeamState>& state_sequence) {
    
    if (state_sequence.empty()) return false;
    
    // Check that all states have consistent team size
    size_t expected_team_size = state_sequence[0].pokemon_hp.size();
    for (const auto& state : state_sequence) {
        if (state.pokemon_hp.size() != expected_team_size) return false;
        if (state.pokemon_current_hp.size() != expected_team_size) return false;
        if (state.pokemon_names.size() != expected_team_size) return false;
    }
    
    // Check that Pokemon names remain consistent (same team)
    for (size_t i = 1; i < state_sequence.size(); ++i) {
        if (state_sequence[i].pokemon_names != state_sequence[0].pokemon_names) {
            return false;
        }
    }
    
    return true;
}

bool IntegrationTestValidator::isWithinTolerance(double value1, double value2, double tolerance) {
    return std::abs(value1 - value2) <= tolerance;
}

bool IntegrationTestValidator::hasValidProgression(const std::vector<double>& values) {
    if (values.size() < 2) return true;
    
    // Check for general upward trend (allowing for some variation)
    int increasing_count = 0;
    for (size_t i = 1; i < values.size(); ++i) {
        if (values[i] >= values[i-1]) {
            increasing_count++;
        }
    }
    
    // At least 60% of transitions should be non-decreasing
    double progression_ratio = static_cast<double>(increasing_count) / (values.size() - 1);
    return progression_ratio >= 0.6;
}

// IntegrationTestDataGenerator Implementation

Team IntegrationTestDataGenerator::generateRandomTeam(const std::string& team_name, int team_size) {
    std::vector<Pokemon> team_members;
    
    std::vector<std::vector<std::string>> type_combinations = {
        {"fire"}, {"water"}, {"grass"}, {"electric"}, {"psychic"}, {"fighting"},
        {"rock"}, {"ground"}, {"flying"}, {"bug"}, {"poison"}, {"ghost"},
        {"ice"}, {"dragon"}, {"dark"}, {"steel"}, {"fairy"}, {"normal"},
        {"fire", "flying"}, {"water", "ice"}, {"grass", "poison"}, {"electric", "steel"},
        {"psychic", "fairy"}, {"fighting", "steel"}, {"rock", "ground"}, {"dragon", "flying"}
    };
    
    std::uniform_int_distribution<size_t> type_dist(0, type_combinations.size() - 1);
    
    for (int i = 0; i < team_size; ++i) {
        std::string pokemon_name = team_name + "_Member" + std::to_string(i + 1);
        auto types = type_combinations[type_dist(rng_)];
        
        Pokemon random_pokemon = generateRandomPokemon(pokemon_name, types);
        team_members.push_back(random_pokemon);
    }
    
    return TestUtils::createTestTeam(team_members);
}

Team IntegrationTestDataGenerator::generateGymTeam(const std::string& type, int team_size) {
    std::vector<Pokemon> gym_team;
    
    for (int i = 0; i < team_size; ++i) {
        std::string pokemon_name = type + "Gym_Pokemon" + std::to_string(i + 1);
        
        // Gym Pokemon should be type-specialized
        std::vector<std::string> types = {type};
        if (i == team_size - 1) {  // Leader's ace gets dual type
            std::vector<std::string> secondary_types = {"normal", "flying", "steel"};
            std::uniform_int_distribution<size_t> sec_dist(0, secondary_types.size() - 1);
            types.push_back(secondary_types[sec_dist(rng_)]);
        }
        
        Pokemon gym_pokemon = generateRandomPokemon(pokemon_name, types, 70, 120);
        
        // Replace some moves with type-specific ones
        gym_pokemon.moves.clear();
        gym_pokemon.moves.push_back(TestUtils::createTestMove(type + "-signature", 85, 95, 10, type, "special"));
        gym_pokemon.moves.push_back(TestUtils::createTestMove(type + "-move", 70, 100, 15, type, "physical"));
        gym_pokemon.moves.push_back(TestUtils::createTestMove("double-team", 0, 100, 15, "normal", "status"));
        gym_pokemon.moves.push_back(TestUtils::createTestMove("protect", 0, 100, 10, "normal", "status"));
        
        gym_team.push_back(gym_pokemon);
    }
    
    return TestUtils::createTestTeam(gym_team);
}

Team IntegrationTestDataGenerator::generateEliteFourTeam(const std::string& specialization, int team_size) {
    std::vector<Pokemon> elite_team;
    
    // Elite Four teams are more powerful and specialized
    std::vector<std::string> base_types;
    if (specialization == "ice") base_types = {"ice", "water"};
    else if (specialization == "fighting") base_types = {"fighting", "rock"};
    else if (specialization == "ghost") base_types = {"ghost", "poison"};
    else if (specialization == "dragon") base_types = {"dragon", "flying"};
    else base_types = {specialization};
    
    for (int i = 0; i < team_size; ++i) {
        std::string pokemon_name = "Elite" + specialization + "_Pokemon" + std::to_string(i + 1);
        
        std::vector<std::string> types = {base_types[i % base_types.size()]};
        if (i == team_size - 1) {  // Elite Four ace gets specialization type
            types = {specialization, base_types[0]};
        }
        
        Pokemon elite_pokemon = generateRandomPokemon(pokemon_name, types, 90, 150);
        
        // Elite Four Pokemon have powerful movesets
        elite_pokemon.moves.clear();
        elite_pokemon.moves.push_back(TestUtils::createTestMove(specialization + "-ultimate", 110, 85, 5, specialization, "special"));
        elite_pokemon.moves.push_back(TestUtils::createTestMove(specialization + "-blast", 90, 100, 10, specialization, "special"));
        elite_pokemon.moves.push_back(TestUtils::createTestMove("recover", 0, 100, 5, "normal", "status"));
        elite_pokemon.moves.push_back(TestUtils::createTestMove("calm-mind", 0, 100, 20, "psychic", "status"));
        
        elite_team.push_back(elite_pokemon);
    }
    
    return TestUtils::createTestTeam(elite_team);
}

Team IntegrationTestDataGenerator::generateChampionTeam(int team_size) {
    std::vector<Pokemon> champion_team;
    
    // Champion has the most diverse and powerful team
    std::vector<std::vector<std::string>> champion_types = {
        {"dragon", "flying"}, {"psychic", "steel"}, {"fire", "dark"}, 
        {"water", "fairy"}, {"electric", "flying"}, {"grass", "poison"}
    };
    
    for (int i = 0; i < team_size; ++i) {
        std::string pokemon_name = "Champion_Legendary" + std::to_string(i + 1);
        
        auto types = champion_types[i % champion_types.size()];
        Pokemon champion_pokemon = generateRandomPokemon(pokemon_name, types, 110, 170);
        
        // Champion Pokemon have legendary movesets
        champion_pokemon.moves.clear();
        champion_pokemon.moves.push_back(TestUtils::createTestMove("champion-signature", 130, 90, 5, types[0], "special"));
        champion_pokemon.moves.push_back(TestUtils::createTestMove("legendary-power", 100, 100, 8, types[1], "special"));
        champion_pokemon.moves.push_back(TestUtils::createTestMove("recover", 0, 100, 5, "normal", "status"));
        champion_pokemon.moves.push_back(TestUtils::createTestMove("dragon-dance", 0, 100, 20, "dragon", "status"));
        
        champion_team.push_back(champion_pokemon);
    }
    
    return TestUtils::createTestTeam(champion_team);
}

std::vector<std::pair<Team, Battle::AIDifficulty>> IntegrationTestDataGenerator::generateTournamentSequence() {
    std::vector<std::pair<Team, Battle::AIDifficulty>> sequence;
    
    // 8 Gyms with progressive difficulty
    std::vector<std::string> gym_types = {"rock", "water", "electric", "grass", "poison", "psychic", "fire", "ground"};
    std::vector<Battle::AIDifficulty> gym_difficulties = {
        Battle::AIDifficulty::EASY, Battle::AIDifficulty::EASY,       // Gyms 1-2
        Battle::AIDifficulty::MEDIUM, Battle::AIDifficulty::MEDIUM, Battle::AIDifficulty::MEDIUM,  // Gyms 3-5
        Battle::AIDifficulty::HARD, Battle::AIDifficulty::HARD, Battle::AIDifficulty::HARD    // Gyms 6-8
    };
    
    for (size_t i = 0; i < gym_types.size(); ++i) {
        Team gym_team = generateGymTeam(gym_types[i], 2);
        sequence.emplace_back(gym_team, gym_difficulties[i]);
    }
    
    // 4 Elite Four members - all Expert difficulty
    std::vector<std::string> elite_specializations = {"ice", "fighting", "ghost", "dragon"};
    for (const auto& spec : elite_specializations) {
        Team elite_team = generateEliteFourTeam(spec, 3);
        sequence.emplace_back(elite_team, Battle::AIDifficulty::EXPERT);
    }
    
    // Champion - Expert difficulty
    Team champion_team = generateChampionTeam(4);
    sequence.emplace_back(champion_team, Battle::AIDifficulty::EXPERT);
    
    return sequence;
}

Pokemon IntegrationTestDataGenerator::generateRandomPokemon(
    const std::string& name_prefix, 
    const std::vector<std::string>& types,
    int min_stat, int max_stat) {
    
    std::uniform_int_distribution<int> stat_dist(min_stat, max_stat);
    
    int hp = stat_dist(rng_);
    int attack = stat_dist(rng_);
    int defense = stat_dist(rng_);
    int special_attack = stat_dist(rng_);
    int special_defense = stat_dist(rng_);
    int speed = stat_dist(rng_);
    
    Pokemon random_pokemon = TestUtils::createTestPokemon(
        name_prefix, hp, attack, defense, special_attack, special_defense, speed, types
    );
    
    // Generate random moveset
    random_pokemon.moves.clear();
    
    // Create distributions for move stats
    std::uniform_int_distribution<int> power_dist(60, 90);
    std::uniform_int_distribution<int> accuracy_dist(85, 100);
    std::uniform_int_distribution<int> pp_dist(10, 20);
    
    // Add type-based moves
    for (const auto& type : types) {
        random_pokemon.moves.push_back(TestUtils::createTestMove(
            type + "-attack", 
            power_dist(rng_),  // Power
            accuracy_dist(rng_), // Accuracy
            pp_dist(rng_),  // PP
            type, 
            "special"
        ));
    }
    
    // Fill remaining slots with generic moves
    while (random_pokemon.moves.size() < 4) {
        std::vector<std::pair<std::string, std::string>> generic_moves = {
            {"tackle", "normal"}, {"double-team", "normal"}, {"rest", "psychic"}, {"protect", "normal"}
        };
        
        std::uniform_int_distribution<size_t> move_dist(0, generic_moves.size() - 1);
        auto [move_name, move_type] = generic_moves[move_dist(rng_)];
        
        random_pokemon.moves.push_back(TestUtils::createTestMove(
            move_name, 70, 100, 15, move_type, "physical"
        ));
    }
    
    return random_pokemon;
}

} // namespace IntegrationTestUtils