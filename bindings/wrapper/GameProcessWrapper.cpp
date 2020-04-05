#include <spdlog/spdlog.h>

#include "../../src/Griddy/Core/TurnBasedGameProcess.hpp"
#include "NumpyWrapper.cpp"
#include "StepPlayerWrapper.cpp"
#include "wrapper.hpp"

namespace griddy {
class Py_GameProcessWrapper {
 public:
  Py_GameProcessWrapper(std::shared_ptr<Grid> grid, std::shared_ptr<Observer> observer)
      : gameProcess_(std::shared_ptr<TurnBasedGameProcess>(new TurnBasedGameProcess(grid, observer))) {
    spdlog::debug("Created game process wrapper");

    gameProcess_->init();
  }

  std::shared_ptr<TurnBasedGameProcess> unwrapped() {
    return gameProcess_;
  }

  std::shared_ptr<Py_StepPlayerWrapper> addPlayer(std::string playerName, ObserverType observerType) {
    if (gameProcess_->isStarted()) {
      throw std::invalid_argument("Cannot add players after the game has started");
    }

    auto observer = createObserver(observerType, gameProcess_->getGrid());

    auto nextPlayerId = ++numPlayers_;
    auto player = std::shared_ptr<Py_StepPlayerWrapper>(new Py_StepPlayerWrapper(nextPlayerId, playerName, observer));
    gameProcess_->addPlayer(player->unwrapped());
    return player;
  }

  void startGame() {
    if (gameProcess_->isStarted()) {
      throw std::invalid_argument("Cannot start a game that has already started.");
    }

    gameProcess_->startGame();
  }

  std::shared_ptr<NumpyWrapper<uint8_t>> observe() {
    auto observer = gameProcess_->getObserver();

    if (observer == nullptr) {
      throw std::invalid_argument("No global observer configured");
    }

    return std::shared_ptr<NumpyWrapper<uint8_t>>(new NumpyWrapper<uint8_t>(observer->getShape(), observer->getStrides(), gameProcess_->observe(-1)));
  }

 private:
  const std::shared_ptr<TurnBasedGameProcess> gameProcess_;
  uint32_t numPlayers_ = 0;
};
}  // namespace griddy