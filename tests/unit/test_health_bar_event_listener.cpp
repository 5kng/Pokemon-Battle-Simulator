#include <gtest/gtest.h>
#include "../utils/test_utils.h"
#include "health_bar_event_listener.h"
#include "health_bar_animator.h"
#include "battle_events.h"
#include <memory>

// Mock health bar animator for testing
class MockHealthBarAnimator : public HealthBarAnimator {
public:
    MockHealthBarAnimator() : HealthBarAnimator() {
        displayAnimatedHealthCalls = 0;
        displayStaticHealthCalls = 0;
    }
    
    void displayAnimatedHealth(const std::string& pokemonName,
                             int currentHP, int maxHP,
                             int previousHP = -1,
                             const std::string& statusCondition = "") const override {
        lastPokemonName = pokemonName;
        lastCurrentHP = currentHP;
        lastMaxHP = maxHP;
        lastPreviousHP = previousHP;
        lastStatusCondition = statusCondition;
        displayAnimatedHealthCalls++;
    }
    
    void displayStaticHealth(const std::string& pokemonName,
                           int currentHP, int maxHP,
                           const std::string& statusCondition = "") const override {
        lastPokemonName = pokemonName;
        lastCurrentHP = currentHP;
        lastMaxHP = maxHP;
        lastStatusCondition = statusCondition;
        displayStaticHealthCalls++;
    }
    
    // Test accessors
    mutable int displayAnimatedHealthCalls;
    mutable int displayStaticHealthCalls;
    mutable std::string lastPokemonName;
    mutable int lastCurrentHP;
    mutable int lastMaxHP;
    mutable int lastPreviousHP;
    mutable std::string lastStatusCondition;
};

class HealthBarEventListenerTest : public TestUtils::BattleTestFixture {
protected:
    void SetUp() override {
        TestUtils::BattleTestFixture::SetUp();
        
        // Create mock animator
        mockAnimator = std::make_shared<MockHealthBarAnimator>();
        
        // Create event listener with mock animator
        eventListener = std::make_unique<HealthBarEventListener>(mockAnimator);
        
        // Create additional test Pokemon
        testPokemon3 = TestUtils::createTestPokemon("TestMon3", 80, 90, 85, 95, 80, 100, {"electric"});
        testPokemon4 = TestUtils::createTestPokemon("TestMon4", 120, 70, 110, 80, 95, 60, {"water"});
        
        // Register test Pokemon with the listener
        eventListener->registerPokemon(&testPokemon1, "Player Pokemon");
        eventListener->registerPokemon(&testPokemon2, "AI Pokemon");
    }
    
    void TearDown() override {
        TestUtils::BattleTestFixture::TearDown();
    }
    
    std::shared_ptr<MockHealthBarAnimator> mockAnimator;
    std::unique_ptr<HealthBarEventListener> eventListener;
    Pokemon testPokemon3, testPokemon4;
};

// Test constructor and basic initialization
TEST_F(HealthBarEventListenerTest, ConstructorAndInitialization) {
    EXPECT_EQ(eventListener->getAnimator(), mockAnimator);
}

// Test Pokemon registration
TEST_F(HealthBarEventListenerTest, PokemonRegistration) {
    // Already registered Pokemon should be registered
    EXPECT_TRUE(eventListener->isPokemonRegistered(&testPokemon1));
    EXPECT_TRUE(eventListener->isPokemonRegistered(&testPokemon2));
    
    // Not registered Pokemon should not be registered
    EXPECT_FALSE(eventListener->isPokemonRegistered(&testPokemon3));
    
    // Register new Pokemon
    eventListener->registerPokemon(&testPokemon3, "Electric Type");
    EXPECT_TRUE(eventListener->isPokemonRegistered(&testPokemon3));
    
    // Register Pokemon without custom display name
    eventListener->registerPokemon(&testPokemon4);
    EXPECT_TRUE(eventListener->isPokemonRegistered(&testPokemon4));
}

// Test Pokemon unregistration
TEST_F(HealthBarEventListenerTest, PokemonUnregistration) {
    EXPECT_TRUE(eventListener->isPokemonRegistered(&testPokemon1));
    
    eventListener->unregisterPokemon(&testPokemon1);
    EXPECT_FALSE(eventListener->isPokemonRegistered(&testPokemon1));
    
    // Unregistering non-registered Pokemon should not crash
    eventListener->unregisterPokemon(&testPokemon3);
    EXPECT_FALSE(eventListener->isPokemonRegistered(&testPokemon3));
}

// Test health change event handling
TEST_F(HealthBarEventListenerTest, HealthChangeEventHandling) {
    BattleEvents::HealthChangeEvent event;
    event.pokemon = &testPokemon1;
    event.oldHealth = 100;
    event.newHealth = 75;
    event.damage = 25;
    event.source = "move";
    
    // Reset call counters
    mockAnimator->displayAnimatedHealthCalls = 0;
    
    eventListener->onHealthChanged(event);
    
    // Should call animated health display
    EXPECT_EQ(mockAnimator->displayAnimatedHealthCalls, 1);
    EXPECT_EQ(mockAnimator->lastCurrentHP, 75);
    EXPECT_EQ(mockAnimator->lastPreviousHP, 100);
    EXPECT_EQ(mockAnimator->lastMaxHP, testPokemon1.stats.hp);
}

// Test health change event with unregistered Pokemon
TEST_F(HealthBarEventListenerTest, HealthChangeEventUnregisteredPokemon) {
    BattleEvents::HealthChangeEvent event;
    event.pokemon = &testPokemon3; // Not registered
    event.oldHealth = 80;
    event.newHealth = 60;
    event.damage = 20;
    event.source = "move";
    
    mockAnimator->displayAnimatedHealthCalls = 0;
    
    eventListener->onHealthChanged(event);
    
    // Should not call animator for unregistered Pokemon
    EXPECT_EQ(mockAnimator->displayAnimatedHealthCalls, 0);
}

// Test battle start event handling
TEST_F(HealthBarEventListenerTest, BattleStartEventHandling) {
    BattleEvents::BattleStartEvent event;
    event.playerStartPokemon = &testPokemon1;
    event.aiStartPokemon = &testPokemon2;
    
    mockAnimator->displayStaticHealthCalls = 0;
    
    eventListener->onBattleStart(event);
    
    // Should display static health for both Pokemon
    EXPECT_GE(mockAnimator->displayStaticHealthCalls, 1);
}

// Test Pokemon switch event handling
TEST_F(HealthBarEventListenerTest, PokemonSwitchEventHandling) {
    // First register the new Pokemon
    eventListener->registerPokemon(&testPokemon3, "New Pokemon");
    
    BattleEvents::PokemonSwitchEvent event;
    event.oldPokemon = &testPokemon1;
    event.newPokemon = &testPokemon3;
    event.isPlayerSwitch = true;
    
    mockAnimator->displayStaticHealthCalls = 0;
    
    eventListener->onPokemonSwitch(event);
    
    // Should display health for the new Pokemon
    EXPECT_GE(mockAnimator->displayStaticHealthCalls, 1);
}

// Test animation speed configuration
TEST_F(HealthBarEventListenerTest, AnimationSpeedConfiguration) {
    // Test different animation speeds
    EXPECT_NO_THROW({
        eventListener->setAnimationSpeed(HealthBarAnimator::AnimationSpeed::FAST);
        eventListener->setAnimationSpeed(HealthBarAnimator::AnimationSpeed::NORMAL);
        eventListener->setAnimationSpeed(HealthBarAnimator::AnimationSpeed::SLOW);
        eventListener->setAnimationSpeed(HealthBarAnimator::AnimationSpeed::DISABLED);
    });
}

// Test color theme configuration
TEST_F(HealthBarEventListenerTest, ColorThemeConfiguration) {
    // Test different color themes
    EXPECT_NO_THROW({
        eventListener->setColorTheme(HealthBarAnimator::ColorTheme::NONE);
        eventListener->setColorTheme(HealthBarAnimator::ColorTheme::BASIC);
        eventListener->setColorTheme(HealthBarAnimator::ColorTheme::ENHANCED);
    });
}

// Test factory function
TEST_F(HealthBarEventListenerTest, FactoryFunction) {
    auto listener1 = createHealthBarListener();
    EXPECT_NE(listener1, nullptr);
    EXPECT_NE(listener1->getAnimator(), nullptr);
    
    auto listener2 = createHealthBarListener(HealthBarAnimator::AnimationSpeed::FAST, HealthBarAnimator::ColorTheme::ENHANCED);
    EXPECT_NE(listener2, nullptr);
    EXPECT_NE(listener2->getAnimator(), nullptr);
    
    // Should create different instances
    EXPECT_NE(listener1, listener2);
}

// Test healing events
TEST_F(HealthBarEventListenerTest, HealingEvents) {
    BattleEvents::HealthChangeEvent healEvent;
    healEvent.pokemon = &testPokemon1;
    healEvent.oldHealth = 50;
    healEvent.newHealth = 75;
    healEvent.damage = -25; // Negative damage indicates healing
    healEvent.source = "item";
    
    mockAnimator->displayAnimatedHealthCalls = 0;
    
    eventListener->onHealthChanged(healEvent);
    
    EXPECT_EQ(mockAnimator->displayAnimatedHealthCalls, 1);
    EXPECT_EQ(mockAnimator->lastCurrentHP, 75);
    EXPECT_EQ(mockAnimator->lastPreviousHP, 50);
}

// Test multiple consecutive health changes
TEST_F(HealthBarEventListenerTest, MultipleConsecutiveHealthChanges) {
    // First damage event
    BattleEvents::HealthChangeEvent event1;
    event1.pokemon = &testPokemon1;
    event1.oldHealth = 100;
    event1.newHealth = 80;
    event1.damage = 20;
    event1.source = "move";
    
    // Second damage event
    BattleEvents::HealthChangeEvent event2;
    event2.pokemon = &testPokemon1;
    event2.oldHealth = 80;
    event2.newHealth = 50;
    event2.damage = 30;
    event2.source = "move";
    
    mockAnimator->displayAnimatedHealthCalls = 0;
    
    eventListener->onHealthChanged(event1);
    eventListener->onHealthChanged(event2);
    
    EXPECT_EQ(mockAnimator->displayAnimatedHealthCalls, 2);
}

// Test status condition integration
TEST_F(HealthBarEventListenerTest, StatusConditionIntegration) {
    // Set status condition on Pokemon
    testPokemon1.status_condition = StatusCondition::POISON;
    
    BattleEvents::HealthChangeEvent event;
    event.pokemon = &testPokemon1;
    event.oldHealth = 100;
    event.newHealth = 90;
    event.damage = 10;
    event.source = "status";
    
    mockAnimator->displayAnimatedHealthCalls = 0;
    
    eventListener->onHealthChanged(event);
    
    EXPECT_EQ(mockAnimator->displayAnimatedHealthCalls, 1);
    // Status condition should be reflected in the display (implementation dependent)
}

// Test zero damage events
TEST_F(HealthBarEventListenerTest, ZeroDamageEvents) {
    BattleEvents::HealthChangeEvent event;
    event.pokemon = &testPokemon1;
    event.oldHealth = 100;
    event.newHealth = 100;
    event.damage = 0;
    event.source = "missed";
    
    mockAnimator->displayAnimatedHealthCalls = 0;
    
    eventListener->onHealthChanged(event);
    
    // Should still handle zero damage events
    EXPECT_EQ(mockAnimator->displayAnimatedHealthCalls, 1);
    EXPECT_EQ(mockAnimator->lastCurrentHP, 100);
    EXPECT_EQ(mockAnimator->lastPreviousHP, 100);
}

// Test knockout events
TEST_F(HealthBarEventListenerTest, KnockoutEvents) {
    BattleEvents::HealthChangeEvent knockoutEvent;
    knockoutEvent.pokemon = &testPokemon1;
    knockoutEvent.oldHealth = 15;
    knockoutEvent.newHealth = 0;
    knockoutEvent.damage = 15;
    knockoutEvent.source = "move";
    
    mockAnimator->displayAnimatedHealthCalls = 0;
    
    eventListener->onHealthChanged(knockoutEvent);
    
    EXPECT_EQ(mockAnimator->displayAnimatedHealthCalls, 1);
    EXPECT_EQ(mockAnimator->lastCurrentHP, 0);
    EXPECT_EQ(mockAnimator->lastPreviousHP, 15);
}

// Test revival events (0 HP to some HP)
TEST_F(HealthBarEventListenerTest, RevivalEvents) {
    // Set Pokemon to fainted
    testPokemon1.current_hp = 0;
    
    BattleEvents::HealthChangeEvent revivalEvent;
    revivalEvent.pokemon = &testPokemon1;
    revivalEvent.oldHealth = 0;
    revivalEvent.newHealth = 50;
    revivalEvent.damage = -50; // Negative damage for healing
    revivalEvent.source = "revive";
    
    mockAnimator->displayAnimatedHealthCalls = 0;
    
    eventListener->onHealthChanged(revivalEvent);
    
    EXPECT_EQ(mockAnimator->displayAnimatedHealthCalls, 1);
    EXPECT_EQ(mockAnimator->lastCurrentHP, 50);
    EXPECT_EQ(mockAnimator->lastPreviousHP, 0);
}

// Test custom display names
TEST_F(HealthBarEventListenerTest, CustomDisplayNames) {
    // Register Pokemon with custom name
    eventListener->registerPokemon(&testPokemon3, "My Electric Buddy");
    
    BattleEvents::HealthChangeEvent event;
    event.pokemon = &testPokemon3;
    event.oldHealth = 80;
    event.newHealth = 60;
    event.damage = 20;
    event.source = "move";
    
    mockAnimator->displayAnimatedHealthCalls = 0;
    
    eventListener->onHealthChanged(event);
    
    EXPECT_EQ(mockAnimator->displayAnimatedHealthCalls, 1);
    // The display name should be used (implementation dependent)
}

// Test event listener with null animator (error case)
TEST_F(HealthBarEventListenerTest, NullAnimatorHandling) {
    // This test checks error handling with null animator
    EXPECT_THROW({
        HealthBarEventListener badListener(nullptr);
    }, std::exception);
}

// Test re-registration of same Pokemon
TEST_F(HealthBarEventListenerTest, PokemonReregistration) {
    EXPECT_TRUE(eventListener->isPokemonRegistered(&testPokemon1));
    
    // Re-register with different name
    eventListener->registerPokemon(&testPokemon1, "New Display Name");
    EXPECT_TRUE(eventListener->isPokemonRegistered(&testPokemon1));
    
    // Should handle re-registration gracefully
    BattleEvents::HealthChangeEvent event;
    event.pokemon = &testPokemon1;
    event.oldHealth = 100;
    event.newHealth = 75;
    event.damage = 25;
    event.source = "move";
    
    EXPECT_NO_THROW({
        eventListener->onHealthChanged(event);
    });
}

// Test maximum health changes
TEST_F(HealthBarEventListenerTest, MaximumHealthChanges) {
    BattleEvents::HealthChangeEvent maxDamageEvent;
    maxDamageEvent.pokemon = &testPokemon1;
    maxDamageEvent.oldHealth = testPokemon1.stats.hp;
    maxDamageEvent.newHealth = 0;
    maxDamageEvent.damage = testPokemon1.stats.hp;
    maxDamageEvent.source = "move";
    
    mockAnimator->displayAnimatedHealthCalls = 0;
    
    eventListener->onHealthChanged(maxDamageEvent);
    
    EXPECT_EQ(mockAnimator->displayAnimatedHealthCalls, 1);
    EXPECT_EQ(mockAnimator->lastCurrentHP, 0);
    EXPECT_EQ(mockAnimator->lastMaxHP, testPokemon1.stats.hp);
}

// Test different damage sources
TEST_F(HealthBarEventListenerTest, DifferentDamageSources) {
    std::vector<std::string> sources = {"move", "weather", "status", "item", "ability", "recoil"};
    
    for (const auto& source : sources) {
        BattleEvents::HealthChangeEvent event;
        event.pokemon = &testPokemon1;
        event.oldHealth = 100;
        event.newHealth = 90;
        event.damage = 10;
        event.source = source;
        
        mockAnimator->displayAnimatedHealthCalls = 0;
        
        eventListener->onHealthChanged(event);
        
        EXPECT_EQ(mockAnimator->displayAnimatedHealthCalls, 1) << "Failed for source: " << source;
    }
}

// Test performance with many events
TEST_F(HealthBarEventListenerTest, PerformanceWithManyEvents) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        BattleEvents::HealthChangeEvent event;
        event.pokemon = &testPokemon1;
        event.oldHealth = 100;
        event.newHealth = 90;
        event.damage = 10;
        event.source = "move";
        
        eventListener->onHealthChanged(event);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should handle many events quickly
    EXPECT_LT(duration.count(), 1000); // Less than 1 second for 1000 events
}