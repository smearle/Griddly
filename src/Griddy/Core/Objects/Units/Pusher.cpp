#include "Pusher.hpp"
#include <spdlog/fmt/fmt.h>
#include <memory>
#include "../../Actions/Action.hpp"
#include "../../Actions/Move.hpp"
#include "../../Grid.hpp"

namespace griddy {
const ObjectType Pusher::type = ObjectType::PUSHER;

ObjectType Pusher::getObjectType() const { return type; }

std::string Pusher::getDescription() const {
  return fmt::format(
      "[{0}, {1}] {2}, health={3}",
      x,
      y,
      "Pusher",
      health_);
};

bool Pusher::canPerformAction(std::shared_ptr<griddy::Action> action) {
  return action->getActionType() == ActionType::MOVE;
}

int Pusher::onPerformAction(std::shared_ptr<griddy::Object> destinationObject, std::shared_ptr<griddy::Action> action) {
  auto actionType = action->getActionType();

  switch (actionType) {
    case MOVE:
      moveObject(action->getDestinationLocation());
      return true;
      break;
    default:
      return false;
  }
}

bool Pusher::onActionPerformed(std::shared_ptr<Object> sourceObject, std::shared_ptr<Action> action) {
  auto actionType = action->getActionType();

  switch (actionType) {
    case PUNCH:
      health_ -= 1;
      return true;
      break;
    default:
      return false;
  }
}

Pusher::Pusher(int playerId) : Unit(playerId, 40) {
}

Pusher::~Pusher() {}

}  // namespace griddy