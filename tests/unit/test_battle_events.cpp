#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <chrono>
#include <thread>
#include "test_utils.h"
#include "battle_events.h"
#include "pokemon.h"
#include "move.h"
#include "weather.h"

using namespace BattleEvents;

// Mock event listener for testing
class MockEventListener : public BattleEventListener {
public:
    // Event counters
    int healthChangedCount = 0;
    int statusChangedCount = 0;
    int moveUsedCount = 0;
    int weatherChangedCount = 0;
    int pokemonSwitchCount = 0;
    int battleStartCount = 0;
    int battleEndCount = 0;
    int turnStartCount = 0;
    int turnEndCount = 0;
    int multiTurnMoveCount = 0;

    // Event data storage
    std::vector<HealthChangeEvent> healthEvents;
    std::vector<StatusChangeEvent> statusEvents;
    std::vector<MoveUsedEvent> moveEvents;
    std::vector<WeatherChangeEvent> weatherEvents;
    std::vector<PokemonSwitchEvent> switchEvents;
    std::vector<BattleStartEvent> battleStartEvents;
    std::vector<BattleEndEvent> battleEndEvents;
    std::vector<MultiTurnMoveEvent> multiTurnEvents;
    std::vector<int> turnStartNumbers;
    std::vector<int> turnEndNumbers;

    // Exception simulation
    bool shouldThrowException = false;
    std::string exceptionMessage = "Mock exception";

    void onHealthChanged(const HealthChangeEvent& event) override {
        if (shouldThrowException) {
            throw std::runtime_error(exceptionMessage);
        }
        healthChangedCount++;
        healthEvents.push_back(event);
    }

    void onStatusChanged(const StatusChangeEvent& event) override {
        if (shouldThrowException) {
            throw std::runtime_error(exceptionMessage);
        }
        statusChangedCount++;
        statusEvents.push_back(event);
    }

    void onMoveUsed(const MoveUsedEvent& event) override {
        if (shouldThrowException) {
            throw std::runtime_error(exceptionMessage);
        }
        moveUsedCount++;
        moveEvents.push_back(event);
    }

    void onWeatherChanged(const WeatherChangeEvent& event) override {
        if (shouldThrowException) {
            throw std::runtime_error(exceptionMessage);
        }
        weatherChangedCount++;
        weatherEvents.push_back(event);
    }

    void onPokemonSwitch(const PokemonSwitchEvent& event) override {
        if (shouldThrowException) {
            throw std::runtime_error(exceptionMessage);
        }
        pokemonSwitchCount++;
        switchEvents.push_back(event);
    }

    void onBattleStart(const BattleStartEvent& event) override {
        if (shouldThrowException) {
            throw std::runtime_error(exceptionMessage);
        }
        battleStartCount++;
        battleStartEvents.push_back(event);
    }

    void onBattleEnd(const BattleEndEvent& event) override {
        if (shouldThrowException) {
            throw std::runtime_error(exceptionMessage);
        }
        battleEndCount++;
        battleEndEvents.push_back(event);
    }

    void onTurnStart(int turnNumber) override {
        if (shouldThrowException) {
            throw std::runtime_error(exceptionMessage);
        }
        turnStartCount++;
        turnStartNumbers.push_back(turnNumber);
    }

    void onTurnEnd(int turnNumber) override {
        if (shouldThrowException) {
            throw std::runtime_error(exceptionMessage);
        }
        turnEndCount++;
        turnEndNumbers.push_back(turnNumber);
    }

    void onMultiTurnMove(const MultiTurnMoveEvent& event) override {
        if (shouldThrowException) {
            throw std::runtime_error(exceptionMessage);
        }
        multiTurnMoveCount++;
        multiTurnEvents.push_back(event);
    }

    // Reset all counters and data
    void reset() {
        healthChangedCount = statusChangedCount = moveUsedCount = 0;
        weatherChangedCount = pokemonSwitchCount = battleStartCount = 0;
        battleEndCount = turnStartCount = turnEndCount = multiTurnMoveCount = 0;
        
        healthEvents.clear();
        statusEvents.clear();
        moveEvents.clear();
        weatherEvents.clear();
        switchEvents.clear();
        battleStartEvents.clear();
        battleEndEvents.clear();
        multiTurnEvents.clear();
        turnStartNumbers.clear();
        turnEndNumbers.clear();
        
        shouldThrowException = false;
    }

    // Helper to get total event count
    int getTotalEventCount() const {
        return healthChangedCount + statusChangedCount + moveUsedCount +
               weatherChangedCount + pokemonSwitchCount + battleStartCount +
               battleEndCount + turnStartCount + turnEndCount + multiTurnMoveCount;
    }
};

class BattleEventsTest : public TestUtils::PokemonTestFixture {
protected:
    void SetUp() override {
        TestUtils::PokemonTestFixture::SetUp();
        
        // Create event manager
        eventManager = std::make_unique<BattleEventManager>();
        
        // Create test pokemon
        testPokemon1 = TestUtils::createTestPokemon("TestMon1", 100, 80, 70, 90, 85, 75);
        testPokemon2 = TestUtils::createTestPokemon("TestMon2", 90, 85, 65, 95, 80, 85);
        
        // Create test move
        testMove = TestUtils::createTestMove("TestMove", 80, 100, 15);
        
        // Create mock listeners
        listener1 = std::make_shared<MockEventListener>();
        listener2 = std::make_shared<MockEventListener>();
        listener3 = std::make_shared<MockEventListener>();
    }

    void TearDown() override {
        eventManager->clear();
    }

    std::unique_ptr<BattleEventManager> eventManager;
    Pokemon testPokemon1, testPokemon2;
    Move testMove;
    std::shared_ptr<MockEventListener> listener1, listener2, listener3;
};

// Test event manager construction and basic functionality
TEST_F(BattleEventsTest, EventManagerConstruction) {
    EXPECT_NE(eventManager, nullptr);
    EXPECT_EQ(eventManager->getListenerCount(), 0);
    EXPECT_FALSE(eventManager->hasListeners());
}

// Test listener subscription and unsubscription
TEST_F(BattleEventsTest, ListenerSubscription) {
    // Initial state
    EXPECT_EQ(eventManager->getListenerCount(), 0);
    EXPECT_FALSE(eventManager->hasListeners());
    
    // Subscribe listeners
    eventManager->subscribe(listener1);
    EXPECT_EQ(eventManager->getListenerCount(), 1);
    EXPECT_TRUE(eventManager->hasListeners());
    
    eventManager->subscribe(listener2);
    EXPECT_EQ(eventManager->getListenerCount(), 2);
    
    eventManager->subscribe(listener3);
    EXPECT_EQ(eventManager->getListenerCount(), 3);
    
    // Test duplicate subscription (should not add duplicate)
    eventManager->subscribe(listener1);
    EXPECT_EQ(eventManager->getListenerCount(), 3);
    
    // Test null listener subscription (should not crash)
    eventManager->subscribe(nullptr);
    EXPECT_EQ(eventManager->getListenerCount(), 3);
}

TEST_F(BattleEventsTest, ListenerUnsubscription) {
    // Subscribe listeners
    eventManager->subscribe(listener1);
    eventManager->subscribe(listener2);
    eventManager->subscribe(listener3);
    EXPECT_EQ(eventManager->getListenerCount(), 3);
    
    // Unsubscribe one listener
    eventManager->unsubscribe(listener2);
    EXPECT_EQ(eventManager->getListenerCount(), 2);
    
    // Unsubscribe non-existent listener (should not crash)
    auto nonExistentListener = std::make_shared<MockEventListener>();
    eventManager->unsubscribe(nonExistentListener);
    EXPECT_EQ(eventManager->getListenerCount(), 2);
    
    // Unsubscribe null listener (should not crash)
    eventManager->unsubscribe(nullptr);
    EXPECT_EQ(eventManager->getListenerCount(), 2);
    
    // Clear all listeners
    eventManager->clear();
    EXPECT_EQ(eventManager->getListenerCount(), 0);
    EXPECT_FALSE(eventManager->hasListeners());
}

// Test health change event creation and notification
TEST_F(BattleEventsTest, HealthChangeEvents) {
    eventManager->subscribe(listener1);
    eventManager->subscribe(listener2);
    
    // Create health change event
    auto event = eventManager->createHealthChangeEvent(&testPokemon1, 100, 75, "attack");
    
    // Verify event data
    EXPECT_EQ(event.pokemon, &testPokemon1);
    EXPECT_EQ(event.oldHealth, 100);
    EXPECT_EQ(event.newHealth, 75);
    EXPECT_EQ(event.damage, 25); // Positive for damage
    EXPECT_EQ(event.source, "attack");
    
    // Notify listeners
    eventManager->notifyHealthChanged(event);
    
    // Verify listeners received event
    EXPECT_EQ(listener1->healthChangedCount, 1);
    EXPECT_EQ(listener2->healthChangedCount, 1);
    ASSERT_EQ(listener1->healthEvents.size(), 1);
    EXPECT_EQ(listener1->healthEvents[0].pokemon, &testPokemon1);
    EXPECT_EQ(listener1->healthEvents[0].damage, 25);
    
    // Test healing event (negative damage)
    auto healEvent = eventManager->createHealthChangeEvent(&testPokemon1, 75, 90, "potion");
    EXPECT_EQ(healEvent.damage, -15); // Negative for healing
    
    eventManager->notifyHealthChanged(healEvent);
    EXPECT_EQ(listener1->healthChangedCount, 2);
    EXPECT_EQ(listener1->healthEvents[1].damage, -15);
}

// Test status change event creation and notification
TEST_F(BattleEventsTest, StatusChangeEvents) {
    eventManager->subscribe(listener1);
    
    // Create status change event
    auto event = eventManager->createStatusChangeEvent(&testPokemon1, StatusCondition::NONE, 
                                                      StatusCondition::POISON, 3, "toxic");
    
    // Verify event data
    EXPECT_EQ(event.pokemon, &testPokemon1);
    EXPECT_EQ(event.oldStatus, StatusCondition::NONE);
    EXPECT_EQ(event.newStatus, StatusCondition::POISON);
    EXPECT_EQ(event.turnsRemaining, 3);
    EXPECT_EQ(event.source, "toxic");
    
    // Notify listeners
    eventManager->notifyStatusChanged(event);
    
    // Verify listeners received event
    EXPECT_EQ(listener1->statusChangedCount, 1);
    ASSERT_EQ(listener1->statusEvents.size(), 1);
    EXPECT_EQ(listener1->statusEvents[0].newStatus, StatusCondition::POISON);
    
    // Test status recovery
    auto recoveryEvent = eventManager->createStatusChangeEvent(&testPokemon1, StatusCondition::POISON,
                                                              StatusCondition::NONE, 0, "natural");
    eventManager->notifyStatusChanged(recoveryEvent);
    EXPECT_EQ(listener1->statusChangedCount, 2);
    EXPECT_EQ(listener1->statusEvents[1].newStatus, StatusCondition::NONE);
}

// Test move used event creation and notification
TEST_F(BattleEventsTest, MoveUsedEvents) {
    eventManager->subscribe(listener1);
    
    // Create move used event
    auto event = eventManager->createMoveUsedEvent(&testPokemon1, &testMove, &testPokemon2, 
                                                  true, false, 1.0);
    
    // Verify event data
    EXPECT_EQ(event.user, &testPokemon1);
    EXPECT_EQ(event.move, &testMove);
    EXPECT_EQ(event.target, &testPokemon2);
    EXPECT_TRUE(event.wasSuccessful);
    EXPECT_FALSE(event.wasCritical);
    EXPECT_DOUBLE_EQ(event.effectiveness, 1.0);
    
    // Notify listeners
    eventManager->notifyMoveUsed(event);
    
    // Verify listeners received event
    EXPECT_EQ(listener1->moveUsedCount, 1);
    ASSERT_EQ(listener1->moveEvents.size(), 1);
    EXPECT_EQ(listener1->moveEvents[0].user, &testPokemon1);
    EXPECT_EQ(listener1->moveEvents[0].move, &testMove);
    
    // Test critical hit with super effective move
    auto criticalEvent = eventManager->createMoveUsedEvent(&testPokemon2, &testMove, &testPokemon1,
                                                          true, true, 2.0);
    eventManager->notifyMoveUsed(criticalEvent);
    EXPECT_EQ(listener1->moveUsedCount, 2);
    EXPECT_TRUE(listener1->moveEvents[1].wasCritical);
    EXPECT_DOUBLE_EQ(listener1->moveEvents[1].effectiveness, 2.0);
}

// Test weather change event notification
TEST_F(BattleEventsTest, WeatherChangeEvents) {
    eventManager->subscribe(listener1);
    
    // Create weather change event
    WeatherChangeEvent event;
    event.oldWeather = WeatherCondition::NONE;
    event.newWeather = WeatherCondition::RAIN;
    event.turnsRemaining = 5;
    
    // Notify listeners
    eventManager->notifyWeatherChanged(event);
    
    // Verify listeners received event
    EXPECT_EQ(listener1->weatherChangedCount, 1);
    ASSERT_EQ(listener1->weatherEvents.size(), 1);
    EXPECT_EQ(listener1->weatherEvents[0].newWeather, WeatherCondition::RAIN);
    EXPECT_EQ(listener1->weatherEvents[0].turnsRemaining, 5);
    
    // Test weather ending
    WeatherChangeEvent endEvent;
    endEvent.oldWeather = WeatherCondition::RAIN;
    endEvent.newWeather = WeatherCondition::NONE;
    endEvent.turnsRemaining = 0;
    
    eventManager->notifyWeatherChanged(endEvent);
    EXPECT_EQ(listener1->weatherChangedCount, 2);
    EXPECT_EQ(listener1->weatherEvents[1].newWeather, WeatherCondition::NONE);
}

// Test Pokemon switch event notification
TEST_F(BattleEventsTest, PokemonSwitchEvents) {
    eventManager->subscribe(listener1);
    
    // Create Pokemon switch event
    PokemonSwitchEvent event;
    event.oldPokemon = &testPokemon1;
    event.newPokemon = &testPokemon2;
    event.isPlayerSwitch = true;
    
    // Notify listeners
    eventManager->notifyPokemonSwitch(event);
    
    // Verify listeners received event
    EXPECT_EQ(listener1->pokemonSwitchCount, 1);
    ASSERT_EQ(listener1->switchEvents.size(), 1);
    EXPECT_EQ(listener1->switchEvents[0].oldPokemon, &testPokemon1);
    EXPECT_EQ(listener1->switchEvents[0].newPokemon, &testPokemon2);
    EXPECT_TRUE(listener1->switchEvents[0].isPlayerSwitch);
    
    // Test AI switch
    PokemonSwitchEvent aiEvent;
    aiEvent.oldPokemon = &testPokemon2;
    aiEvent.newPokemon = &testPokemon1;
    aiEvent.isPlayerSwitch = false;
    
    eventManager->notifyPokemonSwitch(aiEvent);
    EXPECT_EQ(listener1->pokemonSwitchCount, 2);
    EXPECT_FALSE(listener1->switchEvents[1].isPlayerSwitch);
}

// Test battle start and end events
TEST_F(BattleEventsTest, BattleStartEndEvents) {
    eventManager->subscribe(listener1);
    
    // Create battle start event
    BattleStartEvent startEvent;
    startEvent.playerStartPokemon = &testPokemon1;
    startEvent.aiStartPokemon = &testPokemon2;
    
    // Notify listeners
    eventManager->notifyBattleStart(startEvent);
    
    // Verify listeners received event
    EXPECT_EQ(listener1->battleStartCount, 1);
    ASSERT_EQ(listener1->battleStartEvents.size(), 1);
    EXPECT_EQ(listener1->battleStartEvents[0].playerStartPokemon, &testPokemon1);
    
    // Create battle end event
    BattleEndEvent endEvent;
    endEvent.winner = BattleEndEvent::Winner::PLAYER;
    endEvent.totalTurns = 15;
    
    // Notify listeners
    eventManager->notifyBattleEnd(endEvent);
    
    // Verify listeners received event
    EXPECT_EQ(listener1->battleEndCount, 1);
    ASSERT_EQ(listener1->battleEndEvents.size(), 1);
    EXPECT_EQ(listener1->battleEndEvents[0].winner, BattleEndEvent::Winner::PLAYER);
    EXPECT_EQ(listener1->battleEndEvents[0].totalTurns, 15);
}

// Test turn start and end notifications
TEST_F(BattleEventsTest, TurnEvents) {
    eventManager->subscribe(listener1);
    
    // Notify turn start
    eventManager->notifyTurnStart(1);
    EXPECT_EQ(listener1->turnStartCount, 1);
    ASSERT_EQ(listener1->turnStartNumbers.size(), 1);
    EXPECT_EQ(listener1->turnStartNumbers[0], 1);
    
    // Notify turn end
    eventManager->notifyTurnEnd(1);
    EXPECT_EQ(listener1->turnEndCount, 1);
    ASSERT_EQ(listener1->turnEndNumbers.size(), 1);
    EXPECT_EQ(listener1->turnEndNumbers[0], 1);
    
    // Test multiple turns
    for (int turn = 2; turn <= 5; ++turn) {
        eventManager->notifyTurnStart(turn);
        eventManager->notifyTurnEnd(turn);
    }
    
    EXPECT_EQ(listener1->turnStartCount, 5);
    EXPECT_EQ(listener1->turnEndCount, 5);
    EXPECT_EQ(listener1->turnStartNumbers[4], 5);
    EXPECT_EQ(listener1->turnEndNumbers[4], 5);
}

// Test multi-turn move events
TEST_F(BattleEventsTest, MultiTurnMoveEvents) {
    eventManager->subscribe(listener1);
    
    // Create multi-turn move event
    auto event = eventManager->createMultiTurnMoveEvent(&testPokemon1, &testMove,
                                                       MultiTurnMoveEvent::Phase::CHARGING,
                                                       "TestMon1 is charging up!");
    
    // Verify event data
    EXPECT_EQ(event.pokemon, &testPokemon1);
    EXPECT_EQ(event.move, &testMove);
    EXPECT_EQ(event.phase, MultiTurnMoveEvent::Phase::CHARGING);
    EXPECT_EQ(event.message, "TestMon1 is charging up!");
    
    // Notify listeners
    eventManager->notifyMultiTurnMove(event);
    
    // Verify listeners received event
    EXPECT_EQ(listener1->multiTurnMoveCount, 1);
    ASSERT_EQ(listener1->multiTurnEvents.size(), 1);
    EXPECT_EQ(listener1->multiTurnEvents[0].phase, MultiTurnMoveEvent::Phase::CHARGING);
    
    // Test execution phase
    auto execEvent = eventManager->createMultiTurnMoveEvent(&testPokemon1, &testMove,
                                                           MultiTurnMoveEvent::Phase::EXECUTING,
                                                           "TestMon1 unleashes its power!");
    eventManager->notifyMultiTurnMove(execEvent);
    EXPECT_EQ(listener1->multiTurnMoveCount, 2);
    EXPECT_EQ(listener1->multiTurnEvents[1].phase, MultiTurnMoveEvent::Phase::EXECUTING);
    
    // Test recharge phase
    auto rechargeEvent = eventManager->createMultiTurnMoveEvent(&testPokemon1, &testMove,
                                                               MultiTurnMoveEvent::Phase::RECHARGING,
                                                               "TestMon1 must recharge!");
    eventManager->notifyMultiTurnMove(rechargeEvent);
    EXPECT_EQ(listener1->multiTurnMoveCount, 3);
    EXPECT_EQ(listener1->multiTurnEvents[2].phase, MultiTurnMoveEvent::Phase::RECHARGING);
}

// Test multiple listener notification
TEST_F(BattleEventsTest, MultipleListenerNotification) {
    eventManager->subscribe(listener1);
    eventManager->subscribe(listener2);
    eventManager->subscribe(listener3);
    
    // Create and notify health change event
    auto healthEvent = eventManager->createHealthChangeEvent(&testPokemon1, 100, 80, "damage");
    eventManager->notifyHealthChanged(healthEvent);
    
    // All listeners should receive event
    EXPECT_EQ(listener1->healthChangedCount, 1);
    EXPECT_EQ(listener2->healthChangedCount, 1);
    EXPECT_EQ(listener3->healthChangedCount, 1);
    
    // Create and notify turn start
    eventManager->notifyTurnStart(1);
    
    // All listeners should receive event
    EXPECT_EQ(listener1->turnStartCount, 1);
    EXPECT_EQ(listener2->turnStartCount, 1);
    EXPECT_EQ(listener3->turnStartCount, 1);
}

// Test event notification order consistency
TEST_F(BattleEventsTest, EventNotificationOrder) {
    eventManager->subscribe(listener1);
    eventManager->subscribe(listener2);
    
    // Send multiple events
    for (int i = 1; i <= 5; ++i) {
        auto healthEvent = eventManager->createHealthChangeEvent(&testPokemon1, 100, 100 - i*10, "damage");
        eventManager->notifyHealthChanged(healthEvent);
    }
    
    // Verify order is consistent across listeners
    ASSERT_EQ(listener1->healthEvents.size(), 5);
    ASSERT_EQ(listener2->healthEvents.size(), 5);
    
    for (size_t i = 0; i < 5; ++i) {
        EXPECT_EQ(listener1->healthEvents[i].damage, listener2->healthEvents[i].damage);
        EXPECT_EQ(listener1->healthEvents[i].newHealth, listener2->healthEvents[i].newHealth);
    }
}

// Test exception handling in event notification
TEST_F(BattleEventsTest, ExceptionHandling) {
    eventManager->subscribe(listener1);
    eventManager->subscribe(listener2);
    eventManager->subscribe(listener3);
    
    // Make listener2 throw exception
    listener2->shouldThrowException = true;
    
    // Create and notify event
    auto healthEvent = eventManager->createHealthChangeEvent(&testPokemon1, 100, 80, "damage");
    
    // Should not throw - exceptions should be swallowed
    EXPECT_NO_THROW(eventManager->notifyHealthChanged(healthEvent));
    
    // Other listeners should still receive event
    EXPECT_EQ(listener1->healthChangedCount, 1);
    EXPECT_EQ(listener3->healthChangedCount, 1);
    
    // The throwing listener should still be called (but exception caught)
    // We can't directly test this without modifying the implementation
}

// Test event data integrity and consistency
TEST_F(BattleEventsTest, EventDataIntegrity) {
    eventManager->subscribe(listener1);
    
    // Test health change event data integrity
    const int oldHp = 100;
    const int newHp = 75;
    const std::string source = "thunderbolt";
    
    auto healthEvent = eventManager->createHealthChangeEvent(&testPokemon1, oldHp, newHp, source);
    eventManager->notifyHealthChanged(healthEvent);
    
    ASSERT_EQ(listener1->healthEvents.size(), 1);
    const auto& receivedEvent = listener1->healthEvents[0];
    
    // Verify all data fields are preserved correctly
    EXPECT_EQ(receivedEvent.pokemon, &testPokemon1);
    EXPECT_EQ(receivedEvent.oldHealth, oldHp);
    EXPECT_EQ(receivedEvent.newHealth, newHp);
    EXPECT_EQ(receivedEvent.damage, oldHp - newHp);
    EXPECT_EQ(receivedEvent.source, source);
    
    // Test that pointer references remain valid
    EXPECT_EQ(receivedEvent.pokemon->name, testPokemon1.name);
}

// Test event system performance under load
TEST_F(BattleEventsTest, PerformanceUnderLoad) {
    // Subscribe many listeners
    std::vector<std::shared_ptr<MockEventListener>> listeners;
    for (int i = 0; i < 10; ++i) {
        auto listener = std::make_shared<MockEventListener>();
        listeners.push_back(listener);
        eventManager->subscribe(listener);
    }
    
    EXPECT_EQ(eventManager->getListenerCount(), 10);
    
    const int numEvents = 1000;
    auto start = std::chrono::high_resolution_clock::now();
    
    // Send many events
    for (int i = 0; i < numEvents; ++i) {
        auto healthEvent = eventManager->createHealthChangeEvent(&testPokemon1, 100, 100 - (i % 50), "damage");
        eventManager->notifyHealthChanged(healthEvent);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Verify all listeners received all events
    for (const auto& listener : listeners) {
        EXPECT_EQ(listener->healthChangedCount, numEvents);
        EXPECT_EQ(listener->healthEvents.size(), numEvents);
    }
    
    // Performance should be reasonable (less than 1 second for 10k total notifications)
    EXPECT_LT(duration.count(), 1000) << "Event system too slow: " << duration.count() << "ms";
}

// Test memory management and cleanup
TEST_F(BattleEventsTest, MemoryManagement) {
    // Test that clearing listeners doesn't crash
    eventManager->subscribe(listener1);
    eventManager->subscribe(listener2);
    EXPECT_EQ(eventManager->getListenerCount(), 2);
    
    eventManager->clear();
    EXPECT_EQ(eventManager->getListenerCount(), 0);
    
    // Should not crash when notifying with no listeners
    auto healthEvent = eventManager->createHealthChangeEvent(&testPokemon1, 100, 80, "damage");
    EXPECT_NO_THROW(eventManager->notifyHealthChanged(healthEvent));
    
    // Re-add listeners after clear
    eventManager->subscribe(listener1);
    EXPECT_EQ(eventManager->getListenerCount(), 1);
    
    // Should work normally
    eventManager->notifyHealthChanged(healthEvent);
    EXPECT_EQ(listener1->healthChangedCount, 1);
}

// Test listener lifecycle management
TEST_F(BattleEventsTest, ListenerLifecycle) {
    eventManager->subscribe(listener1);
    eventManager->subscribe(listener2);
    
    // Create event and notify
    auto healthEvent = eventManager->createHealthChangeEvent(&testPokemon1, 100, 80, "damage");
    eventManager->notifyHealthChanged(healthEvent);
    
    EXPECT_EQ(listener1->healthChangedCount, 1);
    EXPECT_EQ(listener2->healthChangedCount, 1);
    
    // Remove one listener
    eventManager->unsubscribe(listener1);
    
    // Notify again
    eventManager->notifyHealthChanged(healthEvent);
    
    // Only remaining listener should receive event
    EXPECT_EQ(listener1->healthChangedCount, 1); // Should not change
    EXPECT_EQ(listener2->healthChangedCount, 2); // Should increment
}

// Test event creation utility methods
TEST_F(BattleEventsTest, EventCreationUtilities) {
    // Test various health change scenarios
    auto damageEvent = eventManager->createHealthChangeEvent(&testPokemon1, 100, 70, "fire blast");
    EXPECT_EQ(damageEvent.damage, 30);
    EXPECT_GT(damageEvent.damage, 0); // Positive for damage
    
    auto healEvent = eventManager->createHealthChangeEvent(&testPokemon1, 70, 85, "synthesis");
    EXPECT_EQ(healEvent.damage, -15);
    EXPECT_LT(healEvent.damage, 0); // Negative for healing
    
    auto noChangeEvent = eventManager->createHealthChangeEvent(&testPokemon1, 100, 100, "miss");
    EXPECT_EQ(noChangeEvent.damage, 0);
    
    // Test move used event variations
    auto missedMove = eventManager->createMoveUsedEvent(&testPokemon1, &testMove, &testPokemon2,
                                                       false, false, 0.0);
    EXPECT_FALSE(missedMove.wasSuccessful);
    EXPECT_DOUBLE_EQ(missedMove.effectiveness, 0.0);
    
    auto superEffectiveMove = eventManager->createMoveUsedEvent(&testPokemon1, &testMove, &testPokemon2,
                                                               true, true, 2.0);
    EXPECT_TRUE(superEffectiveMove.wasSuccessful);
    EXPECT_TRUE(superEffectiveMove.wasCritical);
    EXPECT_DOUBLE_EQ(superEffectiveMove.effectiveness, 2.0);
}

// Test concurrent event notifications (basic thread safety)
TEST_F(BattleEventsTest, BasicThreadSafety) {
    eventManager->subscribe(listener1);
    
    std::vector<std::thread> threads;
    const int numThreads = 2;
    const int eventsPerThread = 50;
    
    // Launch threads that send events concurrently
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([this]() {
            for (int i = 0; i < 50; ++i) {
                auto healthEvent = eventManager->createHealthChangeEvent(&testPokemon1, 100, 90, "damage");
                eventManager->notifyHealthChanged(healthEvent);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Should have received all events (though order may vary)
    EXPECT_EQ(listener1->healthChangedCount, numThreads * eventsPerThread);
}