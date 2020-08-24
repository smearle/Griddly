#pragma once
#include <memory>
#include <sstream>

#include "../LevelGenerators/MapReader.hpp"
#include "../Observers/BlockObserver.hpp"
#include "../Observers/SpriteObserver.hpp"
#include "../Players/Player.hpp"
#include "Objects/ObjectGenerator.hpp"
#include "TerminationGenerator.hpp"

namespace YAML {
class Node;
}


namespace griddly {

class GDYFactory {
 public:
  GDYFactory(std::shared_ptr<ObjectGenerator> objectGenerator, std::shared_ptr<TerminationGenerator> terminationGenerator);
  ~GDYFactory();

  static ActionBehaviourDefinition makeBehaviourDefinition(ActionBehaviourType behaviourType,
                                                           std::string objectName,
                                                           std::string associatedObjectName,
                                                           std::string actionName,
                                                           std::string commandName,
                                                           BehaviourCommandArguments commandArguments,
                                                           std::vector<std::unordered_map<std::string, BehaviourCommandArguments>> actionPreconditions,
                                                           std::unordered_map<std::string, BehaviourCommandArguments> conditionalCommands);

  void createLevel(uint32_t width, uint32_t height, std::shared_ptr<Grid> grid);

  void loadLevel(uint32_t level);

  void loadLevelString(std::string levelString);

  void initializeFromFile(std::string filename);

  void parseFromStream(std::istream& stream);

  void loadEnvironment(YAML::Node environment);
  void loadObjects(YAML::Node objects);
  void loadActions(YAML::Node actions);

  virtual std::shared_ptr<TerminationGenerator> getTerminationGenerator() const;
  virtual std::shared_ptr<LevelGenerator> getLevelGenerator() const;
  virtual std::shared_ptr<ObjectGenerator> getObjectGenerator() const;
  virtual std::unordered_map<std::string, SpriteDefinition> getSpriteObserverDefinitions() const;
  virtual std::unordered_map<std::string, BlockDefinition> getBlockObserverDefinitions() const;

  virtual std::unordered_map<std::string, int32_t> getGlobalVariableDefinitions() const;

  virtual std::shared_ptr<TerminationHandler> createTerminationHandler(std::shared_ptr<Grid> grid, std::vector<std::shared_ptr<Player>> players) const;

  virtual void overrideTileSize(uint32_t tileSize);
  virtual void setMaxSteps(uint32_t maxSteps);
  virtual uint32_t getTileSize() const;
  virtual std::string getName() const;
  virtual uint32_t getNumLevels() const;

  virtual uint32_t getPlayerCount() const;

  std::unordered_map<std::string, ActionInputsDefinition> getActionInputsDefinitions() const;
  std::vector<std::string> getAllAvailableAction();
  std::vector<std::string> getPlayerAvailableAction();
  std::vector<std::string> getNonPlayerAvailableAction();
  virtual ActionInputsDefinition findActionInputsDefinition(std::string actionName) const;
  std::vector<uint32_t> getInputsIds(std::string actionName);
  virtual PlayerObserverDefinition getPlayerObserverDefinition() const;
  virtual std::string getAvatarObject() const;

 private:
  void parseActionBehaviours(
      ActionBehaviourType actionBehaviourType,
      std::string objectName,
      std::string actionName,
      std::vector<std::string> associatedObjectNames,
      YAML::Node commandsNode,
      YAML::Node preconditionsNode);

  std::vector<std::string> singleOrListNodeToList(YAML::Node singleOrList);
  BehaviourCommandArguments singleOrListNodeToCommandArguments(YAML::Node singleOrList);

  void parseGlobalVariables(YAML::Node variablesNode);
  void parseTerminationConditions(YAML::Node terminationNode);
  void parseBlockObserverDefinitions(std::string objectName, YAML::Node blockNode);
  void parseSpriteObserverDefinitions(std::string objectName, YAML::Node spriteNode);
  void parseBlockObserverDefinition(std::string objectName, uint32_t renderTileId, YAML::Node blockNode);
  void parseSpriteObserverDefinition(std::string objectName, uint32_t renderTileId, YAML::Node spriteNode);
  void parsePlayerDefinition(YAML::Node playerNode);
  void parseCommandNode(
      std::string commandName,
      YAML::Node commandNode,
      ActionBehaviourType actionBehaviourType,
      std::string objectName,
      std::string actionName,
      std::vector<std::string> associatedObjectNames,
      std::vector<std::unordered_map<std::string, BehaviourCommandArguments>> actionPreconditions);

  std::unordered_map<uint32_t, InputMapping> defaultActionInputMappings() const;
  void loadActionInputsDefinition(std::string actionName, YAML::Node actionInputMappingNode);

  std::unordered_map<std::string, BlockDefinition> blockObserverDefinitions_;
  std::unordered_map<std::string, SpriteDefinition> spriteObserverDefinitions_;

  PlayerObserverDefinition playerObserverDefinition_{};

  std::unordered_map<std::string, int32_t> globalVariableDefinitions_;

  uint32_t numActions_ = 6;
  uint32_t tileSize_ = 10;
  std::string name_ = "UnknownEnvironment";
  uint32_t playerCount_;
  std::string avatarObject_ = "";
  std::unordered_map<std::string, ActionInputsDefinition> actionInputsDefinitions_;

  std::shared_ptr<MapReader> mapReaderLevelGenerator_;
  const std::shared_ptr<ObjectGenerator> objectGenerator_;
  const std::shared_ptr<TerminationGenerator> terminationGenerator_;

  std::vector<std::string> levelStrings_;
};
}  // namespace griddly