#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "game_state.h"
#include "state_serializer.h"
#include <fstream>
#include <filesystem>

using namespace game_serialization;

TEST_CASE("Game state serialization and deserialization", "[serialization]") {
    // Создаем исходное состояние
    GameState original;
    original.next_dog_id = 10;
    original.next_lost_object_id = 5;
    original.next_player_id = 3;
    original.game_time = std::chrono::milliseconds(15000);
    
    // Добавляем собаку
    DogState dog;
    dog.id = 1;
    dog.name = "Buddy";
    dog.position = {10.5, 20.3};
    dog.speed = {1.0, 0.5};
    dog.direction = Direction::RIGHT;
    dog.score = 100;
    dog.bag_capacity = 5;
    dog.bag.push_back({1, 2, 50});
    dog.bag.push_back({2, 3, 30});
    original.dogs.push_back(dog);
    
    // Добавляем потерянный предмет
    LostObjectState lost;
    lost.id = 100;
    lost.type = 1;
    lost.value = 20;
    lost.position = {15.0, 25.0};
    original.lost_objects.push_back(lost);
    
    // Добавляем игрока
    PlayerState player;
    player.id = 1000;
    player.name = "Player1";
    player.token = "abc123token";
    player.dog_id = 1;
    player.map_id = "map1";
    original.players.push_back(player);
    
    // Сохраняем в файл
    std::string test_file = "test_state.sav";
    REQUIRE(StateSerializer::Save(original, test_file));
    REQUIRE(std::filesystem::exists(test_file));
    
    // Загружаем из файла
    GameState loaded;
    REQUIRE(StateSerializer::Load(loaded, test_file));
    
    // Проверяем данные
    REQUIRE(loaded.next_dog_id == original.next_dog_id);
    REQUIRE(loaded.next_lost_object_id == original.next_lost_object_id);
    REQUIRE(loaded.next_player_id == original.next_player_id);
    REQUIRE(loaded.game_time == original.game_time);
    
    REQUIRE(loaded.dogs.size() == 1);
    REQUIRE(loaded.dogs[0].id == dog.id);
    REQUIRE(loaded.dogs[0].name == dog.name);
    REQUIRE(loaded.dogs[0].position.x == Approx(dog.position.x));
    REQUIRE(loaded.dogs[0].position.y == Approx(dog.position.y));
    REQUIRE(loaded.dogs[0].speed.vx == Approx(dog.speed.vx));
    REQUIRE(loaded.dogs[0].speed.vy == Approx(dog.speed.vy));
    REQUIRE(loaded.dogs[0].direction == dog.direction);
    REQUIRE(loaded.dogs[0].score == dog.score);
    REQUIRE(loaded.dogs[0].bag.size() == 2);
    REQUIRE(loaded.dogs[0].bag[0].value == 50);
    
    REQUIRE(loaded.lost_objects.size() == 1);
    REQUIRE(loaded.lost_objects[0].id == lost.id);
    REQUIRE(loaded.lost_objects[0].type == lost.type);
    
    REQUIRE(loaded.players.size() == 1);
    REQUIRE(loaded.players[0].token == player.token);
    
    // Очищаем
    std::filesystem::remove(test_file);
    std::filesystem::remove(test_file + ".tmp");
}

TEST_CASE("State serializer handles missing file", "[serialization]") {
    GameState state;
    std::string missing_file = "missing_file.sav";
    
    REQUIRE_FALSE(StateSerializer::Load(state, missing_file));
    REQUIRE_FALSE(StateSerializer::StateFileExists(missing_file));
}

TEST_CASE("State serializer atomic save", "[serialization]") {
    GameState state1, state2;
    state1.next_dog_id = 100;
    
    std::string test_file = "atomic_test.sav";
    
    // Сохраняем первое состояние
    REQUIRE(StateSerializer::Save(state1, test_file));
    
    // Читаем его
    REQUIRE(StateSerializer::Load(state2, test_file));
    REQUIRE(state2.next_dog_id == 100);
    
    // Сохраняем новое состояние
    state1.next_dog_id = 200;
    REQUIRE(StateSerializer::Save(state1, test_file));
    
    // Читаем обновленное состояние
    REQUIRE(StateSerializer::Load(state2, test_file));
    REQUIRE(state2.next_dog_id == 200);
    
    // Очищаем
    std::filesystem::remove(test_file);
    std::filesystem::remove(test_file + ".tmp");
}
