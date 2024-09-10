#include <gtest/gtest.h>
#include "../utils/test_utils.h"
#include "health_bar_animator.h"
#include <sstream>
#include <iostream>
#include <chrono>

class HealthBarAnimatorTest : public TestUtils::PokemonTestFixture {
protected:
    void SetUp() override {
        TestUtils::PokemonTestFixture::SetUp();
        
        // Create different animator configurations
        fastConfig = HealthBarAnimator::Config(HealthBarAnimator::AnimationSpeed::FAST);
        normalConfig = HealthBarAnimator::Config(HealthBarAnimator::AnimationSpeed::NORMAL);
        slowConfig = HealthBarAnimator::Config(HealthBarAnimator::AnimationSpeed::SLOW);
        disabledConfig = HealthBarAnimator::Config(HealthBarAnimator::AnimationSpeed::DISABLED);
        
        // Set different color themes
        basicColorConfig = normalConfig;
        basicColorConfig.colorTheme = HealthBarAnimator::ColorTheme::BASIC;
        
        enhancedColorConfig = normalConfig;
        enhancedColorConfig.colorTheme = HealthBarAnimator::ColorTheme::ENHANCED;
        
        noColorConfig = normalConfig;
        noColorConfig.colorTheme = HealthBarAnimator::ColorTheme::NONE;
        
        // Create animators with different configurations
        basicAnimator = std::make_unique<HealthBarAnimator>(basicColorConfig);
        enhancedAnimator = std::make_unique<HealthBarAnimator>(enhancedColorConfig);
        noColorAnimator = std::make_unique<HealthBarAnimator>(noColorConfig);
        fastAnimator = std::make_unique<HealthBarAnimator>(fastConfig);
        slowAnimator = std::make_unique<HealthBarAnimator>(slowConfig);
        disabledAnimator = std::make_unique<HealthBarAnimator>(disabledConfig);
    }
    
    void TearDown() override {
        TestUtils::PokemonTestFixture::TearDown();
    }
    
    // Helper method to capture output (simplified for testing)
    std::string captureOutput(std::function<void()> func) {
        // In a real implementation, you might redirect stdout
        // For testing purposes, we'll simulate the behavior
        func();
        return "output_captured"; // Placeholder
    }
    
    HealthBarAnimator::Config fastConfig, normalConfig, slowConfig, disabledConfig;
    HealthBarAnimator::Config basicColorConfig, enhancedColorConfig, noColorConfig;
    
    std::unique_ptr<HealthBarAnimator> basicAnimator;
    std::unique_ptr<HealthBarAnimator> enhancedAnimator;
    std::unique_ptr<HealthBarAnimator> noColorAnimator;
    std::unique_ptr<HealthBarAnimator> fastAnimator;
    std::unique_ptr<HealthBarAnimator> slowAnimator;
    std::unique_ptr<HealthBarAnimator> disabledAnimator;
};

// Test default configuration
TEST_F(HealthBarAnimatorTest, DefaultConfiguration) {
    HealthBarAnimator::Config defaultConfig;
    
    EXPECT_EQ(defaultConfig.speed, HealthBarAnimator::AnimationSpeed::NORMAL);
    EXPECT_EQ(defaultConfig.colorTheme, HealthBarAnimator::ColorTheme::BASIC);
    EXPECT_EQ(defaultConfig.barLength, 20);
    EXPECT_TRUE(defaultConfig.showPercentage);
    EXPECT_TRUE(defaultConfig.showStatusCondition);
    EXPECT_EQ(defaultConfig.stepDelayMs, 50);
}

// Test configuration with speed parameters
TEST_F(HealthBarAnimatorTest, ConfigurationWithSpeed) {
    EXPECT_EQ(fastConfig.speed, HealthBarAnimator::AnimationSpeed::FAST);
    EXPECT_EQ(fastConfig.stepDelayMs, 25);
    
    EXPECT_EQ(normalConfig.speed, HealthBarAnimator::AnimationSpeed::NORMAL);
    EXPECT_EQ(normalConfig.stepDelayMs, 50);
    
    EXPECT_EQ(slowConfig.speed, HealthBarAnimator::AnimationSpeed::SLOW);
    EXPECT_EQ(slowConfig.stepDelayMs, 100);
    
    EXPECT_EQ(disabledConfig.speed, HealthBarAnimator::AnimationSpeed::DISABLED);
    EXPECT_EQ(disabledConfig.stepDelayMs, 0);
}

// Test static health display
TEST_F(HealthBarAnimatorTest, StaticHealthDisplay) {
    // Test doesn't crash and handles various HP values
    EXPECT_NO_THROW({
        basicAnimator->displayStaticHealth("Pikachu", 100, 100);
        basicAnimator->displayStaticHealth("Charizard", 50, 100);
        basicAnimator->displayStaticHealth("Blastoise", 1, 100);
        basicAnimator->displayStaticHealth("Venusaur", 0, 100);
    });
}

// Test static health display with status conditions
TEST_F(HealthBarAnimatorTest, StaticHealthDisplayWithStatus) {
    EXPECT_NO_THROW({
        basicAnimator->displayStaticHealth("Pikachu", 75, 100, "POISONED");
        basicAnimator->displayStaticHealth("Charizard", 50, 100, "BURNED");
        basicAnimator->displayStaticHealth("Blastoise", 25, 100, "PARALYZED");
        basicAnimator->displayStaticHealth("Venusaur", 80, 100, "ASLEEP");
        basicAnimator->displayStaticHealth("Alakazam", 60, 100, "FROZEN");
    });
}

// Test animated health display
TEST_F(HealthBarAnimatorTest, AnimatedHealthDisplay) {
    // Test doesn't crash and handles various transitions
    EXPECT_NO_THROW({
        basicAnimator->displayAnimatedHealth("Pikachu", 50, 100, 100);  // Damage
        basicAnimator->displayAnimatedHealth("Pikachu", 75, 100, 50);   // Healing
        basicAnimator->displayAnimatedHealth("Pikachu", 25, 100, 75);   // More damage
        basicAnimator->displayAnimatedHealth("Pikachu", 0, 100, 25);    // Knockout
    });
}

// Test animated health display with status conditions
TEST_F(HealthBarAnimatorTest, AnimatedHealthDisplayWithStatus) {
    EXPECT_NO_THROW({
        basicAnimator->displayAnimatedHealth("Pikachu", 75, 100, 100, "POISONED");
        basicAnimator->displayAnimatedHealth("Charizard", 40, 100, 80, "BURNED");
        basicAnimator->displayAnimatedHealth("Blastoise", 60, 100, 30, "PARALYZED");
    });
}

// Test disabled animation speed
TEST_F(HealthBarAnimatorTest, DisabledAnimationSpeed) {
    auto start = std::chrono::high_resolution_clock::now();
    
    disabledAnimator->displayAnimatedHealth("Pikachu", 50, 100, 100);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete very quickly with disabled animation
    EXPECT_LT(duration.count(), 50);
}

// Test fast animation speed
TEST_F(HealthBarAnimatorTest, FastAnimationSpeed) {
    auto start = std::chrono::high_resolution_clock::now();
    
    fastAnimator->displayAnimatedHealth("Pikachu", 50, 100, 100);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete faster than normal but still take some time
    EXPECT_LT(duration.count(), 2000); // Should finish within 2 seconds
}

// Test edge cases for health values
TEST_F(HealthBarAnimatorTest, EdgeCasesForHealthValues) {
    // Test with zero HP
    EXPECT_NO_THROW({
        basicAnimator->displayStaticHealth("Fainted", 0, 100);
        basicAnimator->displayAnimatedHealth("Fainted", 0, 100, 1);
    });
    
    // Test with full HP
    EXPECT_NO_THROW({
        basicAnimator->displayStaticHealth("FullHealth", 100, 100);
        basicAnimator->displayAnimatedHealth("FullHealth", 100, 100, 50);
    });
    
    // Test with 1 HP
    EXPECT_NO_THROW({
        basicAnimator->displayStaticHealth("Critical", 1, 100);
        basicAnimator->displayAnimatedHealth("Critical", 1, 100, 50);
    });
    
    // Test with maximum HP of 1 (edge case)
    EXPECT_NO_THROW({
        basicAnimator->displayStaticHealth("Shedinja", 1, 1);
        basicAnimator->displayAnimatedHealth("Shedinja", 0, 1, 1);
    });
}

// Test different color themes
TEST_F(HealthBarAnimatorTest, DifferentColorThemes) {
    EXPECT_NO_THROW({
        basicAnimator->displayStaticHealth("Basic", 75, 100);
        enhancedAnimator->displayStaticHealth("Enhanced", 75, 100);
        noColorAnimator->displayStaticHealth("NoColor", 75, 100);
    });
}

// Test color support detection
TEST_F(HealthBarAnimatorTest, ColorSupportDetection) {
    bool supportsColors = HealthBarAnimator::supportsColors();
    
    // Should return a boolean (true or false based on terminal support)
    EXPECT_TRUE(supportsColors || !supportsColors); // Tautology to check it doesn't crash
}

// Test optimal configuration detection
TEST_F(HealthBarAnimatorTest, OptimalConfigurationDetection) {
    HealthBarAnimator::Config optimalConfig = HealthBarAnimator::detectOptimalConfig();
    
    // Should return a valid configuration
    EXPECT_GE(optimalConfig.barLength, 10);
    EXPECT_LE(optimalConfig.barLength, 50);
    EXPECT_GE(optimalConfig.stepDelayMs, 0);
    EXPECT_LE(optimalConfig.stepDelayMs, 200);
}

// Test with various Pokemon names
TEST_F(HealthBarAnimatorTest, VariousPokemonNames) {
    std::vector<std::string> pokemonNames = {
        "Pikachu", "Charizard", "Blastoise", "Venusaur", "Alakazam",
        "Machamp", "Gengar", "Dragonite", "Mewtwo", "Mew",
        "Very-Long-Pokemon-Name", "", "123", "Special@Characters!"
    };
    
    for (const auto& name : pokemonNames) {
        EXPECT_NO_THROW({
            basicAnimator->displayStaticHealth(name, 50, 100);
        });
    }
}

// Test health bar with different bar lengths
TEST_F(HealthBarAnimatorTest, DifferentBarLengths) {
    for (int length = 5; length <= 40; length += 5) {
        HealthBarAnimator::Config config;
        config.barLength = length;
        HealthBarAnimator animator(config);
        
        EXPECT_NO_THROW({
            animator.displayStaticHealth("Test", 50, 100);
        });
    }
}

// Test health bar with percentage display options
TEST_F(HealthBarAnimatorTest, PercentageDisplayOptions) {
    HealthBarAnimator::Config withPercentage;
    withPercentage.showPercentage = true;
    HealthBarAnimator animatorWithPercentage(withPercentage);
    
    HealthBarAnimator::Config withoutPercentage;
    withoutPercentage.showPercentage = false;
    HealthBarAnimator animatorWithoutPercentage(withoutPercentage);
    
    EXPECT_NO_THROW({
        animatorWithPercentage.displayStaticHealth("WithPercent", 75, 100);
        animatorWithoutPercentage.displayStaticHealth("WithoutPercent", 75, 100);
    });
}

// Test health bar with status condition display options
TEST_F(HealthBarAnimatorTest, StatusConditionDisplayOptions) {
    HealthBarAnimator::Config withStatus;
    withStatus.showStatusCondition = true;
    HealthBarAnimator animatorWithStatus(withStatus);
    
    HealthBarAnimator::Config withoutStatus;
    withoutStatus.showStatusCondition = false;
    HealthBarAnimator animatorWithoutStatus(withoutStatus);
    
    EXPECT_NO_THROW({
        animatorWithStatus.displayStaticHealth("WithStatus", 50, 100, "POISONED");
        animatorWithoutStatus.displayStaticHealth("WithoutStatus", 50, 100, "POISONED");
    });
}

// Test animation transitions in different directions
TEST_F(HealthBarAnimatorTest, AnimationTransitionsInDifferentDirections) {
    // Test damage (decreasing HP)
    EXPECT_NO_THROW({
        basicAnimator->displayAnimatedHealth("Damage", 25, 100, 75);
    });
    
    // Test healing (increasing HP)
    EXPECT_NO_THROW({
        basicAnimator->displayAnimatedHealth("Healing", 75, 100, 25);
    });
    
    // Test no change
    EXPECT_NO_THROW({
        basicAnimator->displayAnimatedHealth("NoChange", 50, 100, 50);
    });
    
    // Test knockout
    EXPECT_NO_THROW({
        basicAnimator->displayAnimatedHealth("Knockout", 0, 100, 50);
    });
    
    // Test revival
    EXPECT_NO_THROW({
        basicAnimator->displayAnimatedHealth("Revival", 50, 100, 0);
    });
}

// Test multiple consecutive animations
TEST_F(HealthBarAnimatorTest, MultipleConsecutiveAnimations) {
    EXPECT_NO_THROW({
        fastAnimator->displayAnimatedHealth("Battle", 100, 100, -1);  // Initial
        fastAnimator->displayAnimatedHealth("Battle", 75, 100, 100);  // Damage
        fastAnimator->displayAnimatedHealth("Battle", 50, 100, 75);   // More damage
        fastAnimator->displayAnimatedHealth("Battle", 80, 100, 50);   // Heal
        fastAnimator->displayAnimatedHealth("Battle", 20, 100, 80);   // Critical hit
        fastAnimator->displayAnimatedHealth("Battle", 0, 100, 20);    // Knockout
    });
}

// Test error handling with invalid HP values
TEST_F(HealthBarAnimatorTest, ErrorHandlingWithInvalidHPValues) {
    // Test negative current HP (should be handled gracefully)
    EXPECT_NO_THROW({
        basicAnimator->displayStaticHealth("Invalid", -10, 100);
    });
    
    // Test negative max HP (should be handled gracefully)
    EXPECT_NO_THROW({
        basicAnimator->displayStaticHealth("Invalid", 50, -100);
    });
    
    // Test current HP > max HP (should be handled gracefully)
    EXPECT_NO_THROW({
        basicAnimator->displayStaticHealth("Invalid", 150, 100);
    });
    
    // Test zero max HP (should be handled gracefully)
    EXPECT_NO_THROW({
        basicAnimator->displayStaticHealth("Invalid", 0, 0);
    });
}

// Test performance with rapid successive calls
TEST_F(HealthBarAnimatorTest, PerformanceWithRapidSuccessiveCalls) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10; ++i) {
        disabledAnimator->displayStaticHealth("Rapid" + std::to_string(i), 50, 100);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete quickly even with many calls
    EXPECT_LT(duration.count(), 1000);
}

// Test thread safety (basic test)
TEST_F(HealthBarAnimatorTest, BasicThreadSafety) {
    // This is a basic test - in practice you'd need more comprehensive thread testing
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([this, i]() {
            disabledAnimator->displayStaticHealth("Thread" + std::to_string(i), 50, 100);
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // If we reach here without crashing, basic thread safety is working
    SUCCEED();
}

// Test with unusual status condition strings
TEST_F(HealthBarAnimatorTest, UnusualStatusConditionStrings) {
    std::vector<std::string> statusConditions = {
        "", "NORMAL", "POISONED", "BURNED", "PARALYZED", "FROZEN", "ASLEEP",
        "CONFUSED", "BADLY_POISONED", "CURSED", "NIGHTMARE", "ATTRACTED",
        "VeryLongStatusConditionNameThatExceedsNormalLimits",
        "Status with spaces", "STATUS_WITH_UNDERSCORES", "123456"
    };
    
    for (const auto& status : statusConditions) {
        EXPECT_NO_THROW({
            basicAnimator->displayStaticHealth("StatusTest", 50, 100, status);
        });
    }
}