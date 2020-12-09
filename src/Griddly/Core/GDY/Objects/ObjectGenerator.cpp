#include "ObjectGenerator.hpp"

#include <spdlog/spdlog.h>

#define SPDLOG_HEADER_ONLY
#include <spdlog/fmt/fmt.h>

#include "Object.hpp"

namespace griddly {
ObjectGenerator::ObjectGenerator() {
}

ObjectGenerator::~ObjectGenerator() {
}

void ObjectGenerator::defineNewObject(std::string objectName, uint32_t zIdx, char mapChar, std::unordered_map<std::string, uint32_t> variableDefinitions) {
  spdlog::debug("Defining new object {0}", objectName);

  ObjectDefinition objectDefinition;
  objectDefinition.objectName = objectName;
  objectDefinition.zIdx = zIdx;
  objectDefinition.variableDefinitions = variableDefinitions;

  objectDefinitions_.insert({objectName, std::make_shared<ObjectDefinition>(objectDefinition)});

  objectChars_[mapChar] = objectName;
  objectIds_.insert({objectName, objectCount_++});
}

void ObjectGenerator::defineActionBehaviour(
    std::string objectName,
    ActionBehaviourDefinition behaviourDefinition) {
  spdlog::debug("Defining object {0} behaviour {1}:{2}", objectName, behaviourDefinition.actionName, behaviourDefinition.commandName);
  auto objectDefinition = getObjectDefinition(objectName);
  objectDefinition->actionBehaviourDefinitions.push_back(behaviourDefinition);
}

void ObjectGenerator::addInitialAction(std::string objectName, std::string actionName, uint32_t actionId, uint32_t delay, bool randomize) {
  spdlog::debug("Defining object {0} initial action {1}", objectName, actionName);
  auto objectDefinition = getObjectDefinition(objectName);
  objectDefinition->initialActionDefinitions.push_back({actionName, actionId, delay, randomize});
}

std::shared_ptr<Object> ObjectGenerator::cloneInstance(std::shared_ptr<Object> toClone, std::unordered_map<std::string, std::shared_ptr<int32_t>> globalVariables) {

  auto objectName = toClone->getObjectName();
  auto objectDefinition = getObjectDefinition(objectName);

  spdlog::debug("Cloning object {0}. {1} variables, {2} behaviours.",
                objectName,
                objectDefinition->variableDefinitions.size(),
                objectDefinition->actionBehaviourDefinitions.size());

  // Initialize the variables for the Object
  std::unordered_map<std::string, std::shared_ptr<int32_t>> availableVariables;
  for (auto &variableDefinitions : objectDefinition->variableDefinitions) {

    // Copy the variable from the old object
    auto copiedVariableValue = *toClone->getVariableValue(variableDefinitions.first);

    auto initializedVariable = std::make_shared<int32_t>(copiedVariableValue);
    availableVariables.insert({variableDefinitions.first, initializedVariable});
  }

  for (auto &globalVariable : globalVariables) {
    auto variableName = globalVariable.first;
    auto initializedVariable = globalVariable.second;
    availableVariables.insert({variableName, initializedVariable});
  }

  auto objectZIdx = objectDefinition->zIdx;
  auto id = objectIds_[objectName];
  auto initializedObject = std::shared_ptr<Object>(new Object(objectName, id, objectZIdx, availableVariables, shared_from_this()));

  if (objectName == avatarObject_) {
    initializedObject->markAsPlayerAvatar();
  }

  for (auto &actionBehaviourDefinition : objectDefinition->actionBehaviourDefinitions) {
    switch (actionBehaviourDefinition.behaviourType) {
      case ActionBehaviourType::SOURCE:

        // Adding the acion preconditions
        for (auto actionPrecondition : actionBehaviourDefinition.actionPreconditions) {
          auto precondition = actionPrecondition.begin();
          initializedObject->addPrecondition(
              actionBehaviourDefinition.actionName,
              actionBehaviourDefinition.destinationObjectName,
              precondition->first,
              precondition->second);
        }

        initializedObject->addActionSrcBehaviour(
            actionBehaviourDefinition.actionName,
            actionBehaviourDefinition.destinationObjectName,
            actionBehaviourDefinition.commandName,
            actionBehaviourDefinition.commandArguments,
            actionBehaviourDefinition.conditionalCommands);
        break;
      case ActionBehaviourType::DESTINATION:
        initializedObject->addActionDstBehaviour(
            actionBehaviourDefinition.actionName,
            actionBehaviourDefinition.sourceObjectName,
            actionBehaviourDefinition.commandName,
            actionBehaviourDefinition.commandArguments,
            actionBehaviourDefinition.conditionalCommands);
        break;
    }
  }

  initializedObject->setInitialActionDefinitions(objectDefinition->initialActionDefinitions);

  return initializedObject;
}

std::shared_ptr<Object> ObjectGenerator::newInstance(std::string objectName, std::unordered_map<std::string, std::shared_ptr<int32_t>> globalVariables) {
  auto objectDefinition = getObjectDefinition(objectName);

  spdlog::debug("Creating new object {0}.", objectName);

  // Initialize the variables for the Object
  std::unordered_map<std::string, std::shared_ptr<int32_t>> availableVariables;
  for (auto &variableDefinitions : objectDefinition->variableDefinitions) {
    auto variableName = variableDefinitions.first;
    auto initializedVariable = std::make_shared<int32_t>(variableDefinitions.second);
    spdlog::debug("Creating local variable {0} with value {1} for object {2}", variableName, *initializedVariable, objectName);
    availableVariables.insert({variableDefinitions.first, initializedVariable});
  }

  for (auto &globalVariable : globalVariables) {
    auto variableName = globalVariable.first;
    auto initializedVariable = globalVariable.second;
    spdlog::debug("Adding reference to global variable {0} with value {1} to object {2}", variableName, *initializedVariable, objectName);
    availableVariables.insert({variableName, initializedVariable});
  }

  auto objectZIdx = objectDefinition->zIdx;
  auto id = objectIds_[objectName];
  auto initializedObject = std::shared_ptr<Object>(new Object(objectName, id, objectZIdx, availableVariables, shared_from_this()));

  if (objectName == avatarObject_) {
    initializedObject->markAsPlayerAvatar();
  }

  for (auto &actionBehaviourDefinition : objectDefinition->actionBehaviourDefinitions) {
    switch (actionBehaviourDefinition.behaviourType) {
      case ActionBehaviourType::SOURCE:

        // Adding the acion preconditions
        for (auto actionPrecondition : actionBehaviourDefinition.actionPreconditions) {
          auto precondition = actionPrecondition.begin();
          initializedObject->addPrecondition(
              actionBehaviourDefinition.actionName,
              actionBehaviourDefinition.destinationObjectName,
              precondition->first,
              precondition->second);
        }

        initializedObject->addActionSrcBehaviour(
            actionBehaviourDefinition.actionName,
            actionBehaviourDefinition.destinationObjectName,
            actionBehaviourDefinition.commandName,
            actionBehaviourDefinition.commandArguments,
            actionBehaviourDefinition.conditionalCommands);
        break;
      case ActionBehaviourType::DESTINATION:
        initializedObject->addActionDstBehaviour(
            actionBehaviourDefinition.actionName,
            actionBehaviourDefinition.sourceObjectName,
            actionBehaviourDefinition.commandName,
            actionBehaviourDefinition.commandArguments,
            actionBehaviourDefinition.conditionalCommands);
        break;
    }
  }

  initializedObject->setInitialActionDefinitions(objectDefinition->initialActionDefinitions);

  return initializedObject;
}

void ObjectGenerator::setAvatarObject(std::string objectName) {
  avatarObject_ = objectName;
}

void ObjectGenerator::setActionInputDefinitions(std::unordered_map<std::string, ActionInputsDefinition> actionInputsDefinitions) {
  actionInputsDefinitions_ = actionInputsDefinitions;
}

std::unordered_map<std::string, ActionInputsDefinition> ObjectGenerator::getActionInputDefinitions() const {
  return actionInputsDefinitions_;
}

std::unordered_map<std::string, std::shared_ptr<ObjectDefinition>> ObjectGenerator::getObjectDefinitions() const {
  return objectDefinitions_;
}

std::string &ObjectGenerator::getObjectNameFromMapChar(char character) {
  auto objectCharIt = objectChars_.find(character);
  if (objectCharIt == objectChars_.end()) {
    throw std::invalid_argument(fmt::format("Object with map character {0} not defined.", character));
  }
  return objectCharIt->second;
}

std::shared_ptr<ObjectDefinition> &ObjectGenerator::getObjectDefinition(std::string objectName) {
  auto objectDefinitionIt = objectDefinitions_.find(objectName);
  if (objectDefinitionIt == objectDefinitions_.end()) {
    throw std::invalid_argument(fmt::format("Object {0} not defined.", objectName));
  }

  return objectDefinitionIt->second;
}

}  // namespace griddly