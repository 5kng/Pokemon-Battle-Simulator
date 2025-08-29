# Google Test configuration for Pokemon Battle Simulator
cmake_minimum_required(VERSION 3.16)

include(GoogleTest)

# ────────────────────────────────
#  Paths & common sources
# ────────────────────────────────
set(TEST_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/integration
)

set(CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/move.cpp
    ${CMAKE_SOURCE_DIR}/src/core/pokemon.cpp
    ${CMAKE_SOURCE_DIR}/src/core/team.cpp
    ${CMAKE_SOURCE_DIR}/src/core/battle.cpp
    ${CMAKE_SOURCE_DIR}/src/core/weather.cpp
    ${CMAKE_SOURCE_DIR}/src/core/battle_events.cpp
    ${CMAKE_SOURCE_DIR}/src/core/team_builder.cpp
    ${CMAKE_SOURCE_DIR}/src/core/pokemon_data.cpp
    ${CMAKE_SOURCE_DIR}/src/core/tournament_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/gym_leader.cpp
    ${CMAKE_SOURCE_DIR}/src/core/championship_system.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/type_effectiveness.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/move_type_mapping.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/input_validator.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/health_bar_animator.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/health_bar_event_listener.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/ai_strategy.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/ai_factory.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/easy_ai.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/medium_ai.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/hard_ai.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/expert_ai.cpp
)

# ────────────────────────────────
#  Test utilities
# ────────────────────────────────
add_library(test_utils utils/test_utils.cpp)
target_include_directories(test_utils PUBLIC ${TEST_INCLUDE_DIRS})
target_link_libraries(test_utils gtest)

# Integration test utilities
add_library(integration_test_utils integration/integration_test_utilities.cpp)
target_include_directories(integration_test_utils PUBLIC ${TEST_INCLUDE_DIRS})
target_link_libraries(integration_test_utils gtest test_utils)

# ────────────────────────────────
#  Helper to define each test target
# ────────────────────────────────
function(create_test test_name test_source)
    add_executable(${test_name} ${test_source} ${CORE_SOURCES})
    target_include_directories(${test_name} PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(${test_name} gtest gtest_main test_utils)

    # Copy any data files the test expects
    add_custom_command(TARGET ${test_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/data
                $<TARGET_FILE_DIR:${test_name}>/data
        COMMENT "Copying test data files"
    )

    gtest_discover_tests(${test_name})
endfunction()

# Helper function for integration tests (needs integration_test_utils)
function(create_integration_test test_name test_source)
    add_executable(${test_name} ${test_source} ${CORE_SOURCES})
    target_include_directories(${test_name} PRIVATE ${TEST_INCLUDE_DIRS})
    target_link_libraries(${test_name} gtest gtest_main test_utils integration_test_utils)

    # Copy any data files the test expects
    add_custom_command(TARGET ${test_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/data
                $<TARGET_FILE_DIR:${test_name}>/data
        COMMENT "Copying test data files"
    )

    gtest_discover_tests(${test_name})
endfunction()

# ────────────────────────────────
#  Unit tests
# ────────────────────────────────
create_test(test_pokemon            unit/test_pokemon.cpp)
create_test(test_move               unit/test_move.cpp)
create_test(test_type_effectiveness unit/test_type_effectiveness.cpp)
create_test(test_input_validator    unit/test_input_validator.cpp)
create_test(test_team               unit/test_team.cpp)
create_test(test_battle             unit/test_battle.cpp)
create_test(test_weather            unit/test_weather.cpp)
create_test(test_ai                 unit/test_ai.cpp)
create_test(test_easy_ai            unit/test_easy_ai.cpp)
create_test(test_medium_ai          unit/test_medium_ai.cpp)
create_test(test_hard_ai            unit/test_hard_ai.cpp)
create_test(test_expert_ai          unit/test_expert_ai.cpp)
create_test(test_paralysis_determinism unit/test_paralysis_determinism.cpp)
create_test(test_team_builder_phase4  unit/test_team_builder_phase4.cpp)
create_test(test_tournament_manager unit/test_tournament_manager.cpp)
create_test(test_gym_leader         unit/test_gym_leader.cpp)
create_test(test_battle_events      unit/test_battle_events.cpp)
create_test(test_championship_system unit/test_championship_system.cpp)
create_test(test_battle_regression  unit/test_battle_regression.cpp)
create_test(test_ai_regression      unit/test_ai_regression.cpp)
create_test(test_data_regression    unit/test_data_regression.cpp)
create_test(test_performance_regression unit/test_performance_regression.cpp)

# ────────────────────────────────
#  Integration tests
# ────────────────────────────────
create_test(test_full_battle        integration/test_full_battle.cpp)
create_test(test_status_integration integration/test_status_integration.cpp)
create_test(test_weather_integration integration/test_weather_integration.cpp)

# ────────────────────────────────
#  Phase 3 Multi-System Integration tests
# ────────────────────────────────
create_integration_test(test_tournament_battle_integration integration/test_tournament_battle_integration.cpp)
create_integration_test(test_tournament_ai_integration integration/test_tournament_ai_integration.cpp)
create_integration_test(test_championship_system_integration integration/test_championship_system_integration.cpp)
create_integration_test(test_team_state_persistence_integration integration/test_team_state_persistence_integration.cpp)

# ────────────────────────────────
#  Convenience aggregate target
# ────────────────────────────────
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS
        test_pokemon
        test_move
        test_type_effectiveness
        test_input_validator
        test_team
        test_battle
        test_weather
        test_ai
        test_easy_ai
        test_medium_ai
        test_hard_ai
        test_expert_ai
        test_paralysis_determinism
        test_team_builder_phase4
        test_tournament_manager
        test_gym_leader
        test_battle_events
        test_championship_system
        test_full_battle
        test_status_integration
        test_weather_integration
        test_tournament_battle_integration
        test_tournament_ai_integration
        test_championship_system_integration
        test_team_state_persistence_integration
        test_battle_regression
        test_ai_regression
        test_data_regression
        test_performance_regression
    COMMENT "Running all tests"
)

# ────────────────────────────────
#  Phase 3 Integration Test Target
# ────────────────────────────────
add_custom_target(run_phase3_integration_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose -R "test_tournament_.*_integration|test_championship_.*_integration|test_team_state_.*_integration"
    DEPENDS
        test_tournament_battle_integration
        test_tournament_ai_integration
        test_championship_system_integration
        test_team_state_persistence_integration
    COMMENT "Running Phase 3 Multi-System Integration Tests"
)
