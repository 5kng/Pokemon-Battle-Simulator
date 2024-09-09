#include <gtest/gtest.h>
#include "../utils/test_utils.h"
#include "move_type_mapping.h"
#include <set>
#include <algorithm>

class MoveTypeMappingTest : public TestUtils::PokemonTestFixture {
protected:
    void SetUp() override {
        TestUtils::PokemonTestFixture::SetUp();
        // MoveTypeMapping is a static class with lazy initialization
        // First call will initialize the mapping
        MoveTypeMapping::getMoveType("tackle");
    }
};

// Test basic move type retrieval
TEST_F(MoveTypeMappingTest, BasicMoveTypeRetrieval) {
    EXPECT_EQ(MoveTypeMapping::getMoveType("tackle"), "normal");
    EXPECT_EQ(MoveTypeMapping::getMoveType("flamethrower"), "fire");
    EXPECT_EQ(MoveTypeMapping::getMoveType("hydro-pump"), "water");
    EXPECT_EQ(MoveTypeMapping::getMoveType("thunderbolt"), "electric");
    EXPECT_EQ(MoveTypeMapping::getMoveType("vine-whip"), "grass");
    EXPECT_EQ(MoveTypeMapping::getMoveType("ice-beam"), "ice");
    EXPECT_EQ(MoveTypeMapping::getMoveType("karate-chop"), "fighting");
    EXPECT_EQ(MoveTypeMapping::getMoveType("poison-sting"), "poison");
    EXPECT_EQ(MoveTypeMapping::getMoveType("earthquake"), "ground");
    EXPECT_EQ(MoveTypeMapping::getMoveType("wing-attack"), "flying");
    EXPECT_EQ(MoveTypeMapping::getMoveType("confusion"), "psychic");
    EXPECT_EQ(MoveTypeMapping::getMoveType("string-shot"), "bug");
    EXPECT_EQ(MoveTypeMapping::getMoveType("rock-throw"), "rock");
    EXPECT_EQ(MoveTypeMapping::getMoveType("lick"), "ghost");
    EXPECT_EQ(MoveTypeMapping::getMoveType("dragon-rage"), "dragon");
    EXPECT_EQ(MoveTypeMapping::getMoveType("bite"), "dark");
    EXPECT_EQ(MoveTypeMapping::getMoveType("metal-claw"), "steel");
    EXPECT_EQ(MoveTypeMapping::getMoveType("sweet-kiss"), "fairy");
}

// Test default type for unknown moves
TEST_F(MoveTypeMappingTest, DefaultTypeForUnknownMoves) {
    EXPECT_EQ(MoveTypeMapping::getMoveType("nonexistent-move"), "normal");
    EXPECT_EQ(MoveTypeMapping::getMoveType("fake-move"), "normal");
    EXPECT_EQ(MoveTypeMapping::getMoveType(""), "normal");
    EXPECT_EQ(MoveTypeMapping::getMoveType("UNKNOWN"), "normal");
}

// Test case sensitivity
TEST_F(MoveTypeMappingTest, CaseSensitivity) {
    // The current implementation is case-sensitive
    EXPECT_EQ(MoveTypeMapping::getMoveType("tackle"), "normal");
    EXPECT_EQ(MoveTypeMapping::getMoveType("TACKLE"), "normal"); // Should default to normal
    EXPECT_EQ(MoveTypeMapping::getMoveType("Tackle"), "normal"); // Should default to normal
    EXPECT_EQ(MoveTypeMapping::getMoveType("TaCkLe"), "normal"); // Should default to normal
}

// Test Normal type moves
TEST_F(MoveTypeMappingTest, NormalTypeMoves) {
    std::vector<std::string> normalMoves = {
        "tackle", "scratch", "pound", "pay-day", "mega-punch", "mega-kick",
        "headbutt", "body-slam", "wrap", "take-down", "double-edge", "tail-whip",
        "leer", "growl", "roar", "sing", "supersonic", "sonic-boom", "disable",
        "hyper-beam", "strength", "growth", "quick-attack", "rage", "mimic",
        "screech", "double-team", "recover", "harden", "minimize", "smokescreen",
        "confuse-ray", "withdraw", "defense-curl", "barrier", "light-screen",
        "haze", "reflect", "focus-energy", "bide", "metronome", "mirror-move",
        "self-destruct", "egg-bomb", "swift", "skull-bash", "spike-cannon",
        "constrict", "amnesia", "soft-boiled", "dizzy-punch", "flash", "splash",
        "explosion", "fury-swipes", "bonemerang", "rest", "rock-slide",
        "hyper-fang", "sharpen", "conversion", "tri-attack", "super-fang",
        "slash", "substitute", "struggle"
    };
    
    for (const auto& move : normalMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "normal") 
            << "Move '" << move << "' should be normal type";
    }
}

// Test Fire type moves
TEST_F(MoveTypeMappingTest, FireTypeMoves) {
    std::vector<std::string> fireMoves = {
        "ember", "flamethrower", "fire-punch", "fire-blast", "fire-spin",
        "flame-wheel", "fire-fang", "blaze-kick", "heat-wave", "flare-blitz",
        "lava-plume"
    };
    
    for (const auto& move : fireMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "fire")
            << "Move '" << move << "' should be fire type";
    }
}

// Test Water type moves
TEST_F(MoveTypeMappingTest, WaterTypeMoves) {
    std::vector<std::string> waterMoves = {
        "water-gun", "hydro-pump", "surf", "bubble-beam", "water-pulse",
        "aqua-tail", "scald", "bubble", "crabhammer", "clamp",
        "aqua-jet", "brine", "aqua-ring"
    };
    
    for (const auto& move : waterMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "water")
            << "Move '" << move << "' should be water type";
    }
}

// Test Electric type moves
TEST_F(MoveTypeMappingTest, ElectricTypeMoves) {
    std::vector<std::string> electricMoves = {
        "thunder-shock", "thunderbolt", "thunder-wave", "thunder",
        "thunder-punch", "spark", "shock-wave", "discharge",
        "charge-beam", "thunder-fang"
    };
    
    for (const auto& move : electricMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "electric")
            << "Move '" << move << "' should be electric type";
    }
}

// Test Grass type moves
TEST_F(MoveTypeMappingTest, GrassTypeMoves) {
    std::vector<std::string> grassMoves = {
        "vine-whip", "razor-leaf", "solar-beam", "petal-dance", "absorb",
        "mega-drain", "leech-seed", "sleep-powder", "poison-powder", "stun-spore",
        "spore", "cotton-spore", "giga-drain", "synthesis", "magical-leaf",
        "leaf-blade", "energy-ball", "leaf-storm", "power-whip", "seed-bomb"
    };
    
    for (const auto& move : grassMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "grass")
            << "Move '" << move << "' should be grass type";
    }
}

// Test Ice type moves
TEST_F(MoveTypeMappingTest, IceTypeMoves) {
    std::vector<std::string> iceMoves = {
        "ice-beam", "blizzard", "aurora-beam", "ice-punch", "powder-snow",
        "icy-wind", "mist", "ice-shard", "avalanche", "ice-fang",
        "sheer-cold", "icicle-spear"
    };
    
    for (const auto& move : iceMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "ice")
            << "Move '" << move << "' should be ice type";
    }
}

// Test Fighting type moves
TEST_F(MoveTypeMappingTest, FightingTypeMoves) {
    std::vector<std::string> fightingMoves = {
        "karate-chop", "double-kick", "jump-kick", "rolling-kick", "horn-attack",
        "fury-attack", "horn-drill", "thrash", "low-kick", "counter",
        "seismic-toss", "submission", "high-jump-kick", "mach-punch",
        "dynamic-punch", "focus-punch", "superpower", "revenge", "brick-break",
        "arm-thrust", "sky-uppercut", "bulk-up", "cross-chop", "reversal",
        "vital-throw", "close-combat", "hammer-arm"
    };
    
    for (const auto& move : fightingMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "fighting")
            << "Move '" << move << "' should be fighting type";
    }
}

// Test Poison type moves
TEST_F(MoveTypeMappingTest, PoisonTypeMoves) {
    std::vector<std::string> poisonMoves = {
        "poison-sting", "smog", "sludge", "poison-gas", "acid",
        "sludge-bomb", "toxic", "poison-fang", "poison-jab",
        "toxic-spikes", "gunk-shot", "cross-poison"
    };
    
    for (const auto& move : poisonMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "poison")
            << "Move '" << move << "' should be poison type";
    }
}

// Test Ground type moves
TEST_F(MoveTypeMappingTest, GroundTypeMoves) {
    std::vector<std::string> groundMoves = {
        "dig", "bone-club", "bonemerang", "earthquake", "fissure",
        "sand-tomb", "mud-shot", "mud-bomb", "earth-power", "mud-slap",
        "spikes", "magnitude", "bone-rush", "mud-sport", "muddy-water"
    };
    
    for (const auto& move : groundMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "ground")
            << "Move '" << move << "' should be ground type";
    }
}

// Test Flying type moves
TEST_F(MoveTypeMappingTest, FlyingTypeMoves) {
    std::vector<std::string> flyingMoves = {
        "gust", "wing-attack", "whirlwind", "fly", "peck", "drill-peck",
        "sky-attack", "aeroblast", "air-slash", "hurricane", "brave-bird",
        "roost", "tailwind", "air-cutter", "defog", "bounce", "aerial-ace",
        "feather-dance"
    };
    
    for (const auto& move : flyingMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "flying")
            << "Move '" << move << "' should be flying type";
    }
}

// Test Psychic type moves
TEST_F(MoveTypeMappingTest, PsychicTypeMoves) {
    std::vector<std::string> psychicMoves = {
        "confusion", "psychic", "hypnosis", "meditate", "agility", "teleport",
        "night-shade", "kinesis", "dream-eater", "psybeam", "psycho-cut",
        "zen-headbutt", "psycho-boost", "future-sight", "calm-mind",
        "cosmic-power", "stored-power", "psyshock", "miracle-eye",
        "extrasensory", "synchronoise"
    };
    
    for (const auto& move : psychicMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "psychic")
            << "Move '" << move << "' should be psychic type";
    }
}

// Test Bug type moves
TEST_F(MoveTypeMappingTest, BugTypeMoves) {
    std::vector<std::string> bugMoves = {
        "leech-life", "twin-needle", "pin-missile", "spider-web",
        "fury-cutter", "megahorn", "bug-bite", "x-scissor", "bug-buzz",
        "signal-beam", "silver-wind", "u-turn", "attack-order",
        "defend-order", "heal-order"
    };
    
    for (const auto& move : bugMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "bug")
            << "Move '" << move << "' should be bug type";
    }
}

// Test Rock type moves
TEST_F(MoveTypeMappingTest, RockTypeMoves) {
    std::vector<std::string> rockMoves = {
        "rock-throw", "rock-blast", "rollout", "sandstorm", "ancient-power",
        "rock-tomb", "rock-polish", "power-gem", "stone-edge", "stealth-rock",
        "rock-wrecker", "head-smash"
    };
    
    for (const auto& move : rockMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "rock")
            << "Move '" << move << "' should be rock type";
    }
}

// Test Ghost type moves
TEST_F(MoveTypeMappingTest, GhostTypeMoves) {
    std::vector<std::string> ghostMoves = {
        "lick", "shadow-punch", "shadow-ball", "destiny-bond", "spite",
        "curse", "nightmare", "grudge", "astonish", "shadow-claw",
        "ominous-wind", "shadow-sneak", "shadow-force"
    };
    
    for (const auto& move : ghostMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "ghost")
            << "Move '" << move << "' should be ghost type";
    }
}

// Test Dragon type moves
TEST_F(MoveTypeMappingTest, DragonTypeMoves) {
    std::vector<std::string> dragonMoves = {
        "dragon-rage", "twister", "dragon-breath", "dragon-claw",
        "dragon-dance", "dragon-pulse", "dragon-rush", "draco-meteor",
        "outrage", "spacial-rend", "roar-of-time"
    };
    
    for (const auto& move : dragonMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "dragon")
            << "Move '" << move << "' should be dragon type";
    }
}

// Test Dark type moves
TEST_F(MoveTypeMappingTest, DarkTypeMoves) {
    std::vector<std::string> darkMoves = {
        "bite", "crunch", "thief", "feint-attack", "pursuit", "torment",
        "flatter", "memento", "facade", "taunt", "knock-off", "snatch",
        "fake-tears", "payback", "assurance", "embargo", "fling",
        "punishment", "sucker-punch", "dark-void", "night-daze",
        "dark-pulse", "nasty-plot", "night-slash", "switcheroo"
    };
    
    for (const auto& move : darkMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "dark")
            << "Move '" << move << "' should be dark type";
    }
}

// Test Steel type moves
TEST_F(MoveTypeMappingTest, SteelTypeMoves) {
    std::vector<std::string> steelMoves = {
        "metal-claw", "steel-wing", "meteor-mash", "iron-tail", "metal-sound",
        "iron-defense", "doom-desire", "gyro-ball", "magnet-rise", "mirror-shot",
        "flash-cannon", "bullet-punch", "iron-head", "magnet-bomb",
        "autotomize", "heavy-slam", "gear-grind"
    };
    
    for (const auto& move : steelMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "steel")
            << "Move '" << move << "' should be steel type";
    }
}

// Test Fairy type moves
TEST_F(MoveTypeMappingTest, FairyTypeMoves) {
    std::vector<std::string> fairyMoves = {
        "sweet-kiss", "charm", "moonlight", "sweet-scent", "fairy-wind",
        "draining-kiss", "crafty-shield", "flower-shield", "misty-terrain",
        "play-rough", "fairy-lock", "aromatic-mist", "eerie-impulse",
        "dazzling-gleam", "baby-doll-eyes", "disarming-voice", "moonblast",
        "geomancy", "light-of-ruin"
    };
    
    for (const auto& move : fairyMoves) {
        EXPECT_EQ(MoveTypeMapping::getMoveType(move), "fairy")
            << "Move '" << move << "' should be fairy type";
    }
}

// Test moves that appear in multiple type categories (edge cases)
TEST_F(MoveTypeMappingTest, MovesInMultipleCategories) {
    // Note: Some moves appear in multiple type mappings in the source code
    // This tests the actual implementation behavior
    
    // These moves appear in multiple categories, test the last assigned type
    EXPECT_EQ(MoveTypeMapping::getMoveType("ice-beam"), "ice"); // appears in both water and ice
    EXPECT_EQ(MoveTypeMapping::getMoveType("blizzard"), "ice"); // appears in both water and ice
    EXPECT_EQ(MoveTypeMapping::getMoveType("withdraw"), "water"); // appears in both normal and water
    EXPECT_EQ(MoveTypeMapping::getMoveType("haze"), "ice"); // appears in both normal and ice
}

// Test type coverage completeness
TEST_F(MoveTypeMappingTest, TypeCoverageCompleteness) {
    // Test that all major Pokemon types are covered
    std::set<std::string> expectedTypes = {
        "normal", "fire", "water", "electric", "grass", "ice", "fighting",
        "poison", "ground", "flying", "psychic", "bug", "rock", "ghost",
        "dragon", "dark", "steel", "fairy"
    };
    
    std::set<std::string> foundTypes;
    
    // Sample representative moves from each type
    std::vector<std::pair<std::string, std::string>> testMoves = {
        {"tackle", "normal"},
        {"flamethrower", "fire"}, 
        {"hydro-pump", "water"},
        {"thunderbolt", "electric"},
        {"vine-whip", "grass"},
        {"ice-beam", "ice"},
        {"karate-chop", "fighting"},
        {"poison-sting", "poison"},
        {"earthquake", "ground"},
        {"wing-attack", "flying"},
        {"confusion", "psychic"},
        {"string-shot", "bug"},
        {"rock-throw", "rock"},
        {"lick", "ghost"},
        {"dragon-rage", "dragon"},
        {"bite", "dark"},
        {"metal-claw", "steel"},
        {"sweet-kiss", "fairy"}
    };
    
    for (const auto& testMove : testMoves) {
        std::string actualType = MoveTypeMapping::getMoveType(testMove.first);
        foundTypes.insert(actualType);
        EXPECT_EQ(actualType, testMove.second);
    }
    
    // Verify all expected types are found
    for (const auto& expectedType : expectedTypes) {
        EXPECT_NE(foundTypes.find(expectedType), foundTypes.end())
            << "Type '" << expectedType << "' should be represented in move mappings";
    }
}

// Test initialization behavior
TEST_F(MoveTypeMappingTest, InitializationBehavior) {
    // Multiple calls should return consistent results
    std::string type1 = MoveTypeMapping::getMoveType("tackle");
    std::string type2 = MoveTypeMapping::getMoveType("tackle");
    std::string type3 = MoveTypeMapping::getMoveType("tackle");
    
    EXPECT_EQ(type1, type2);
    EXPECT_EQ(type2, type3);
    EXPECT_EQ(type1, "normal");
}

// Performance test for move type lookup
TEST_F(MoveTypeMappingTest, PerformanceTest) {
    std::vector<std::string> testMoves = {
        "tackle", "flamethrower", "hydro-pump", "thunderbolt", "vine-whip",
        "ice-beam", "karate-chop", "poison-sting", "earthquake", "wing-attack"
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Perform many lookups
    for (int i = 0; i < 1000; ++i) {
        for (const auto& move : testMoves) {
            MoveTypeMapping::getMoveType(move);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should be very fast (less than 100ms for 10,000 lookups)
    EXPECT_LT(duration.count(), 100);
}

// Test edge cases and error handling
TEST_F(MoveTypeMappingTest, EdgeCasesAndErrorHandling) {
    // Empty string
    EXPECT_EQ(MoveTypeMapping::getMoveType(""), "normal");
    
    // Whitespace
    EXPECT_EQ(MoveTypeMapping::getMoveType(" "), "normal");
    EXPECT_EQ(MoveTypeMapping::getMoveType("\t"), "normal");
    EXPECT_EQ(MoveTypeMapping::getMoveType("\n"), "normal");
    
    // Very long string
    std::string longMove(1000, 'x');
    EXPECT_EQ(MoveTypeMapping::getMoveType(longMove), "normal");
    
    // Special characters
    EXPECT_EQ(MoveTypeMapping::getMoveType("move-with-dashes"), "normal");
    EXPECT_EQ(MoveTypeMapping::getMoveType("move_with_underscores"), "normal");
    EXPECT_EQ(MoveTypeMapping::getMoveType("move with spaces"), "normal");
    EXPECT_EQ(MoveTypeMapping::getMoveType("move123"), "normal");
}

// Test specific move name patterns
TEST_F(MoveTypeMappingTest, MoveNamePatterns) {
    // Test that moves follow consistent naming patterns
    
    // Hyphenated names should work
    EXPECT_EQ(MoveTypeMapping::getMoveType("thunder-shock"), "electric");
    EXPECT_EQ(MoveTypeMapping::getMoveType("razor-leaf"), "grass");
    EXPECT_EQ(MoveTypeMapping::getMoveType("rock-throw"), "rock");
    
    // Multi-word moves
    EXPECT_EQ(MoveTypeMapping::getMoveType("body-slam"), "normal");
    EXPECT_EQ(MoveTypeMapping::getMoveType("double-team"), "normal");
    EXPECT_EQ(MoveTypeMapping::getMoveType("high-jump-kick"), "fighting");
}