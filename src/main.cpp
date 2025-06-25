#include <chrono>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <vector>

#include "battle.h"

using namespace std;

int main() {
  // Get user's name
  string userName;
  cout << "Enter your name: ";
  cin >> userName;

  // Available teams
  unordered_map<string, vector<string>> selectedTeams = {
      // Player Pokemon
      {"Team 1",
       {"venusaur", "pikachu", "machamp", "arcanine", "lapras", "snorlax"}},
      {"Team 2",
       {"charizard", "starmie", "snorlax", "alakazam", "rhydon", "jolteon"}},
      {"Team 3",
       {"venusaur", "zapdos", "nidoking", "gengar", "lapras", "tauros"}},

      // Opponent Team Pokemon
      {"Opponent Team 1",
       {"aerodactyl", "kabutops", "golem", "onix", "omastar", "rhyhorn"}},
      {"Opponent Team 2",
       {"starmie", "gyarados", "lapras", "golduck", "vaporeon", "seaking"}},
      {"Opponent Team 3",
       {"raichu", "magneton", "electrode", "electabuzz", "jolteon", "pikachu"}},
      {"Opponent Team 4",
       {"victreebel", "exeggutor", "parasect", "tangela", "vileplume",
        "venusaur"}},
      {"Opponent Team 5",
       {"arbok", "tentacruel", "muk", "gengar", "weezing", "venomoth"}},
      {"Opponent Team 6",
       {"alakazam", "slowbro", "mr-mime", "jynx", "hypno", "exeggutor"}},
      {"Opponent Team 7",
       {"ninetails", "arcanine", "rapidash", "magmar", "flareon", "charizard"}},
      {"Opponent Team 8",
       {"nidoking", "nidoqueen", "dugtrio", "rhydon", "marowak", "sandslash"}},
  };

  // Select moves
  unordered_map<string, vector<pair<string, vector<string>>>> selectedMoves = {
      {"Team 1",
       {{"venusaur", {"sludge-bomb", "mega-drain", "leech-seed", "amnesia"}},
        {"pikachu", {"thunderbolt", "brick-break", "iron-tail", "reflect"}},
        {"machamp", {"superpower", "fire-blast", "earthquake", "hyper-beam"}},
        {"arcanine", {"heat-wave", "crunch", "will-o-wisp", "roar"}},
        {"lapras", {"ice-shard", "waterfall", "body-slam", "megahorn"}},
        {"snorlax", {"toxic", "protect", "rest", "body-slam"}}}},
      {"Team 2",
       {{"charizard", {"flamethrower", "slash", "earthquake", "fire-spin"}},
        {"starmie", {"hydro-pump", "psychic", "ice-beam", "recover"}},
        {"snorlax", {"body-slam", "hyper-beam", "earthquake", "rest"}},
        {"alakazam", {"psychic", "recover", "thunder-wave", "reflect"}},
        {"rhydon", {"earthquake", "rock-slide", "body-slam", "substitute"}},
        {"jolteon",
         {"thunderbolt", "thunder-wave", "pin-missile", "double-kick"}}}},
      {"Team 3",
       {{"venusaur", {"razor-leaf", "sleep-powder", "body-slam", "leech-seed"}},
        {"zapdos", {"thunderbolt", "drill-peck", "thunder-wave", "agility"}},
        {"nidoking", {"earthquake", "ice-beam", "thunderbolt", "rock-slide"}},
        {"gengar", {"psychic", "night-shade", "hypnosis", "explosion"}},
        {"lapras", {"hydro-pump", "blizzard", "psychic", "body-slam"}},
        {"tauros", {"body-slam", "hyper-beam", "blizzard", "earthquake"}}}},

      // Opponent Teams
      {"Opponent Team 1",
       {
           {"aerodactyl", {"tackle", "scratch", "protect", "amnesia"}},
           {"kabutops", {"tackle", "scratch", "protect", "amnesia"}},
           {"golem", {"tackle", "scratch", "protect", "amnesia"}},
           {"onix", {"tackle", "scratch", "protect", "amnesia"}},
           {"omastar", {"tackle", "scratch", "protect", "amnesia"}},
           {"rhyhorn", {"tackle", "scratch", "protect", "amnesia"}},
       }},
      {"Opponent Team 2",
       {
           {"starmie", {"psychic", "surf", "ice-beam", "recover"}},
           {"gyarados", {"hydro-pump", "bite", "thunder", "hyper-beam"}},
           {"lapras", {"surf", "ice-beam", "psychic", "body-slam"}},
           {"golduck", {"surf", "psychic", "ice-beam", "disable"}},
           {"vaporeon", {"surf", "ice-beam", "acid-armor", "haze"}},
           {"seaking", {"surf", "ice-beam", "double-edge", "agility"}},
       }},
      {"Opponent Team 3",
       {
           {"raichu",
            {"thunderbolt", "thunder", "double-kick", "seismic-toss"}},
           {"magneton", {"thunderbolt", "thunder-wave", "sonic-boom", "swift"}},
           {"electrode",
            {"thunderbolt", "thunder", "self-destruct", "light-screen"}},
           {"electabuzz",
            {"thunderbolt", "thunder-punch", "seismic-toss", "light-screen"}},
           {"jolteon",
            {"thunderbolt", "thunder", "double-kick", "sand-attack"}},
           {"pikachu",
            {"thunderbolt", "thunder", "seismic-toss", "double-team"}},
       }},
      {"Opponent Team 4",
       {
           {"victreebel",
            {"razor-leaf", "acid", "poison-powder", "sleep-powder"}},
           {"exeggutor", {"razor-leaf", "hypnosis", "psychic", "explosion"}},
           {"parasect", {"spore", "slash", "leech-life", "stun-spore"}},
           {"tangela",
            {"vine-whip", "poison-powder", "stun-spore", "sleep-powder"}},
           {"vileplume",
            {"petal-dance", "poison-powder", "acid", "sleep-powder"}},
           {"venusaur",
            {"razor-leaf", "leech-seed", "poison-powder", "sleep-powder"}},
       }},
      {"Opponent Team 5",
       {
           {"arbok", {"bite", "poison-sting", "acid", "glare"}},
           {"tentacruel",
            {"hydro-pump", "poison-sting", "constrict", "barrier"}},
           {"muk", {"poison-gas", "minimize", "sludge", "harden"}},
           {"gengar", {"night-shade", "hypnosis", "dream-eater", "psychic"}},
           {"weezing", {"sludge", "smokescreen", "explosion", "haze"}},
           {"venomoth",
            {"psychic", "poison-powder", "stun-spore", "sleep-powder"}},
       }},
      {"Opponent Team 6",
       {
           {"alakazam", {"psychic", "recover", "reflect", "kinesis"}},
           {"slowbro", {"psychic", "surf", "amnesia", "disable"}},
           {"mr-mime", {"psychic", "barrier", "light-screen", "meditate"}},
           {"jynx", {"psychic", "ice-beam", "lovely-kiss", "body-slam"}},
           {"hypno", {"psychic", "hypnosis", "dream-eater", "poison-gas"}},
           {"exeggutor", {"psychic", "hypnosis", "leech-seed", "explosion"}},
       }},
      {"Opponent Team 7",
       {
           {"ninetails",
            {"flamethrower", "fire-spin", "confuse-ray", "take-down"}},
           {"arcanine", {"flamethrower", "fire-blast", "take-down", "leer"}},
           {"rapidash", {"fire-blast", "stomp", "take-down", "growl"}},
           {"magmar", {"flamethrower", "fire-punch", "smokescreen", "leer"}},
           {"flareon", {"flamethrower", "fire-spin", "quick-attack", "leer"}},
           {"charizard", {"flamethrower", "fire-spin", "slash", "leer"}},
       }},
      {"Opponent Team 8",
       {
           {"nidoking", {"earthquake", "thrash", "focus-energy", "leer"}},
           {"nidoqueen",
            {"earthquake", "body-slam", "double-kick", "tail-whip"}},
           {"dugtrio", {"earthquake", "slash", "sand-attack", "growl"}},
           {"rhydon", {"earthquake", "horn-drill", "leer", "tail-whip"}},
           {"marowak", {"earthquake", "focus-energy", "leer", "growl"}},
           {"sandslash", {"earthquake", "slash", "sand-attack", "swift"}},
       }},

  };

  // Show available teams for player selection
  std::cout << "\n=== Pokemon Battle Simulator ===" << std::endl;
  std::cout << "Choose your team:" << std::endl;
  std::cout << "1. Team 1 - Balanced Team (Venusaur, Pikachu, Machamp, "
               "Arcanine, Lapras, Snorlax)"
            << std::endl;
  std::cout << "2. Team 2 - Competitive Team (Charizard, Starmie, Snorlax, "
               "Alakazam, Rhydon, Jolteon)"
            << std::endl;
  std::cout << "3. Team 3 - Mixed Team (Venusaur, Zapdos, Nidoking, Gengar, "
               "Lapras, Tauros)"
            << std::endl;

  // Prompt for team selection
  int chosenTeamNum;
  cout << "\n Enter the number of the team you want to select: ";
  cin >> chosenTeamNum;

  // Validate input
  if (chosenTeamNum < 1 || chosenTeamNum > 3) {
    cout << "Invalid selection - try again." << endl;
    return 1;
  }

  string chosenTeamKey = "Team " + to_string(chosenTeamNum);
  const vector<string> chosenTeam = selectedTeams[chosenTeamKey];

  std::cout << "" << endl;
  std::cout << "========================================================== My "
               "Team =========================================================="
            << std::endl;
  std::cout << "" << endl;

  // init Player Team and load Pokemon & Moves
  Team PlayerTeam;
  PlayerTeam.loadTeams(selectedTeams, selectedMoves, chosenTeamKey);

  // Print out Player's team with moves
  std::cout << "Your selected team includes:\n";
  for (int i = 0; i < static_cast<int>(PlayerTeam.size()); ++i) {
    const Pokemon *pokemon = PlayerTeam.getPokemon(i);
    if (pokemon) {
      std::cout << "- " << pokemon->name << "\n  Moves:\n";
      for (const auto &move : pokemon->moves) {
        std::cout << "    * " << move.name << " (Power: " << move.power
                  << ", Accuracy: " << move.accuracy
                  << ", Class: " << move.damage_class << ")\n";
      }
    }
  }
  std::cout << std::endl;

  // Show available opponent teams
  cout << "Available Opponents:" << endl;
  cout << "[1] - Brock (Rock Gym Leader)" << endl;
  cout << "[2] - Misty (Water Gym Leader)" << endl;
  cout << "[3] - Surge (Electric Gym Leader)" << endl;
  cout << "[4] - Erika (Grass Gym Leader)" << endl;
  cout << "[5] - Koga (Poison Gym Leader)" << endl;
  cout << "[6] - Sabrina (Psychic Gym Leader)" << endl;
  cout << "[7] - Blaine (Fire Gym Leader)" << endl;
  cout << "[8] - Giovanni (Ground Gym Leader)" << endl;

  // Prompt for opponent selection
  int chosenOpponentNum;
  cout << "\n Enter the number of your chosen opponent: ";
  cin >> chosenOpponentNum;

  // Validate input
  if (chosenOpponentNum < 1 || chosenOpponentNum > 8) {
    cout << "Invalid selection - try again." << endl;
    return 1;
  }

  string chosenOpponentKey = "Opponent Team " + to_string(chosenOpponentNum);
  const vector<string> chosenOpponent = selectedTeams[chosenOpponentKey];
  cout << "\nYou have selected " << chosenOpponentKey << " with the Pokémon: ";
  for (const auto &pokemon : chosenOpponent) {
    cout << pokemon << " ";
  }
  cout << "\n\n";

  std::cout << "" << endl;
  std::cout
      << "========================================================== Oppenent "
         "Team =========================================================="
      << std::endl;
  std::cout << "" << endl;

  // init Opp Team and load Pokemon & Moves
  Team OppTeam;
  OppTeam.loadTeams(selectedTeams, selectedMoves, chosenOpponentKey);

  // Print out Opponent's team with moves
  std::cout << "Opponent's selected team includes:\n";
  for (int i = 0; i < static_cast<int>(OppTeam.size()); ++i) {
    const Pokemon *pokemon = OppTeam.getPokemon(i);
    if (pokemon) {
      std::cout << "- " << pokemon->name << "\n";
    }
  }
  std::cout << std::endl;

  // BATTLE PART
  Battle battle(PlayerTeam, OppTeam);
  battle.startBattle();
}