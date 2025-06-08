// Copyright 2025 genius-invokation
//
// Based on code from open_spiel:
// Copyright 2022 DeepMind Technologies Limited
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "open_spiel/games/gi_tcg/gi_tcg.h"

#include <algorithm> // For std::min, std::max
#include <memory>      // For std::shared_ptr, std::unique_ptr
#include <utility>     // For std::move
#include <vector>      // For std::vector
#include <string>      // For std::string
#include <stdexcept>   // For std::runtime_error

#include "open_spiel/abseil-cpp/absl/strings/str_cat.h"
#include "open_spiel/abseil-cpp/absl/strings/str_join.h"
#include "open_spiel/game_parameters.h"
#include "open_spiel/spiel_utils.h" // For SpielFatalError, SpielLog

// Necessary for gitcg_free_buffer if used for freeing JSON strings etc.
// And for malloc/free if directly used by gitcg for some buffers.
#include <cstdlib> // For free (used by gitcg_free_buffer indirectly)

namespace open_spiel {
namespace gi_tcg {

// --- Static member initialization ---
bool GITCGGame::gitcg_library_initialized_ = false;
int GITCGGame::active_game_instances_ = 0;

// --- Game parameters defaults ---
namespace {
const int kNumPlayersDefault = 2;
const int kNumDistinctActionsDefault = 1000; // Placeholder - IMPORTANT: Define accurately!
const int kMaxGameRoundsDefault = 15;
const int kMaxTurnsPerRoundDefault = 20;
const int kMaxGameLengthDefault = kMaxGameRoundsDefault * kNumPlayersDefault * kMaxTurnsPerRoundDefault;
const std::vector<int> kObservationTensorShapeDefault = {200}; // Placeholder - Define based on observation.
const long kDefaultRandomSeed = 12345L; // Suffix L for long
const int kDefaultInitialHandsCount = 5;
const bool kDefaultNoShuffleP0 = false;
const bool kDefaultNoShuffleP1 = false;
// Example empty deck JSON. Replace with actual default deck structures or load from files.
const std::string kDefaultDeckP0Json = "{\"characters\": [], \"cards\": []}";
const std::string kDefaultDeckP1Json = "{\"characters\": [], \"cards\": []}";

// Placeholder for parsing deck from JSON. Replace with a proper JSON library or simpler format.
void ParseDeckJson(const std::string& deck_json, std::vector<int>& char_ids, std::vector<int>& card_ids) {
    SpielLog::Warn("ParseDeckJson is a placeholder. Implement proper JSON parsing or use a simpler deck format.");
    // Example: if your JSON is very simple like "char_ids:101,102;card_ids:201,202"
    // This is NOT robust JSON parsing.
    if (deck_json.find("101") != std::string::npos) char_ids.push_back(101);
    if (deck_json.find("102") != std::string::npos) char_ids.push_back(102);
    if (deck_json.find("201") != std::string::npos) card_ids.push_back(201);
}

} // namespace

// --- GITCGGame Methods ---

GITCGGame::GITCGGame(const GameParameters& params)
    : Game(GameType{"gi_tcg",
                    "Genius Invokation TCG",
                    GameType::Dynamics::kSequential, // Turns are strictly alternating
                    GameType::ChanceMode::kSampled,    // Dice, card draws
                    GameType::Information::kImperfectInformation, // Private hands
                    GameType::Utility::kZeroSum,
                    GameType::RewardModel::kTerminal,
                    kNumPlayersDefault,
                    kNumPlayersDefault,
                    true, // Provides observation_string
                    true, // Provides observation_tensor
                    {},   // TODO: Define GameParameter specification if exposing params to UIs/tools
                    false},// Default loadable? Set to true if registered with RegisterGame
      num_distinct_actions_(kNumDistinctActionsDefault),
      max_game_length_(kMaxGameLengthDefault),
      observation_tensor_shape_(kObservationTensorShapeDefault)
      {
  EnsureGitcgInitialized();
  active_game_instances_++;
  ParseGameParameters(params); // Parse and store parameters from `params`
})

GITCGGame::~GITCGGame() {
  active_game_instances_--;
  if (active_game_instances_ == 0) {
    PerformGitcgCleanup();
  }
}

void GITCGGame::EnsureGitcgInitialized() {
  if (!gitcg_library_initialized_) {
    gitcg_initialize();
    // Note: gitcg_thread_initialize() should be called by each thread that will interact with gitcg.
    // This is complex to manage globally here. It's often better handled by the State object
    // or by the parts of the system that manage threads (e.g., an AlphaZero worker thread).
    gitcg_library_initialized_ = true;
    SpielLog::Info("gitcg library initialized globally.");
  }
}

void GITCGGame::PerformGitcgCleanup() {
  if (gitcg_library_initialized_) {
    // Similarly, gitcg_thread_cleanup() should be called by each thread before global cleanup.
    gitcg_cleanup();
    gitcg_library_initialized_ = false;
    SpielLog::Info("gitcg library cleaned up globally.");
  }
}

void GITCGGame::ParseGameParameters(const GameParameters& params) {
  // Retrieve parameters or use defaults.
  random_seed_ = params.Get<long>("random_seed", kDefaultRandomSeed);
  initial_hands_count_ = params.Get<int>("initial_hands", kDefaultInitialHandsCount);
  no_shuffle_player0_ = params.Get<bool>("no_shuffle_p0", kDefaultNoShuffleP0);
  no_shuffle_player1_ = params.Get<bool>("no_shuffle_p1", kDefaultNoShuffleP1);

  std::string deck_p0_str = params.Get<std::string>("deck_p0", kDefaultDeckP0Json);
  std::string deck_p1_str = params.Get<std::string>("deck_p1", kDefaultDeckP1Json);

  ParseDeckJson(deck_p0_str, player0_char_ids_, player0_card_ids_);
  ParseDeckJson(deck_p1_str, player1_char_ids_, player1_card_ids_);

  num_distinct_actions_ = params.Get<int>("num_actions", kNumDistinctActionsDefault);
  max_game_length_ = params.Get<int>("max_length", kMaxGameLengthDefault);
  // observation_tensor_shape_ could also be parsed if it's dynamic.

  SpielLog::Info(absl::StrCat("GITCGGame Parameters: seed=", random_seed_,
                              ", initial_hands=", initial_hands_count_,
                              ", num_actions=", num_distinct_actions_));
}

int GITCGGame::NumDistinctActions() const {
  return num_distinct_actions_;
}

std::unique_ptr<State> GITCGGame::NewInitialState() const {
  gitcg_state_createparam_t create_params = nullptr;
  int err = gitcg_state_createparam_new(&create_params);
  if (err != 0 || create_params == nullptr) {
    SpielFatalError(absl::StrCat("NewInitialState: Failed to create gitcg_state_createparam: error ", err));
  }

  // Set attributes on create_params using parsed member variables.
  gitcg_state_createparam_set_attr_int(create_params, GITCG_ATTR_STATE_CONFIG_RANDOM_SEED, random_seed_);
  gitcg_state_createparam_set_attr_int(create_params, GITCG_ATTR_STATE_CONFIG_INITIAL_HANDS_COUNT, initial_hands_count_);

  if (no_shuffle_player0_) {
    gitcg_state_createparam_set_attr_int(create_params, GITCG_ATTR_CREATEPARAM_NO_SHUFFLE_0, 1);
  }
  if (no_shuffle_player1_) {
    gitcg_state_createparam_set_attr_int(create_params, GITCG_ATTR_CREATEPARAM_NO_SHUFFLE_1, 1);
  }

  // Set decks for player 0
  if (!player0_char_ids_.empty()) {
    err = gitcg_state_createparam_set_deck(create_params, 0, GITCG_SET_DECK_CHARACTERS, player0_char_ids_.data(), player0_char_ids_.size());
    if (err != 0) SpielLog::Warn(absl::StrCat("Failed to set P0 character deck: error ", err));
  }
  if (!player0_card_ids_.empty()) {
    err = gitcg_state_createparam_set_deck(create_params, 0, GITCG_SET_DECK_CARDS, player0_card_ids_.data(), player0_card_ids_.size());
    if (err != 0) SpielLog::Warn(absl::StrCat("Failed to set P0 action card deck: error ", err));
  }

  // Set decks for player 1
  if (!player1_char_ids_.empty()) {
    err = gitcg_state_createparam_set_deck(create_params, 1, GITCG_SET_DECK_CHARACTERS, player1_char_ids_.data(), player1_char_ids_.size());
    if (err != 0) SpielLog::Warn(absl::StrCat("Failed to set P1 character deck: error ", err));
  }
  if (!player1_card_ids_.empty()) {
    err = gitcg_state_createparam_set_deck(create_params, 1, GITCG_SET_DECK_CARDS, player1_card_ids_.data(), player1_card_ids_.size());
    if (err != 0) SpielLog::Warn(absl::StrCat("Failed to set P1 action card deck: error ", err));
  }

  // TODO: Set any other necessary parameters on create_params.

  gitcg_state_t initial_gitcg_state = nullptr;
  err = gitcg_state_new(create_params, &initial_gitcg_state);
  gitcg_state_createparam_free(create_params); // Always free create_params after use.
  create_params = nullptr; // Avoid dangling pointer

  if (err != 0 || initial_gitcg_state == nullptr) {
    SpielFatalError(absl::StrCat("NewInitialState: Failed to create new gitcg_state: error ", err));
  }

  gitcg_game_t gitcg_game_instance = nullptr;
  err = gitcg_game_new(initial_gitcg_state, &gitcg_game_instance);
  // gitcg_game_new is documented to take ownership of the state if successful.
  // If it fails, initial_gitcg_state must be freed by the caller.
  if (err != 0 || gitcg_game_instance == nullptr) {
    gitcg_state_free(initial_gitcg_state); // Clean up if game_new failed.
    SpielFatalError(absl::StrCat("NewInitialState: Failed to create new gitcg_game instance: error ", err));
  }

  // The GITCGState constructor will manage the gitcg_game_instance.
  return std::make_unique<GITCGState>(shared_from_this(), gitcg_game_instance);
}

int GITCGGame::NumPlayers() const {
  return kNumPlayersDefault; // Or parse from GameParameters if variable
}

double GITCGGame::MinUtility() const {
  return -1.0; // Standard for win/loss
}

double GITCGGame::MaxUtility() const {
  return 1.0; // Standard for win/loss
}

std::shared_ptr<const Game> GITCGGame::Clone() const {
  // Game objects are typically stateless beyond their parameters.
  return std::make_shared<GITCGGame>(game_parameters_);
}

std::vector<int> GITCGGame::ObservationTensorShape() const {
  return observation_tensor_shape_;
}

int GITCGGame::MaxGameLength() const {
  return max_game_length_;
}

// --- End of GITCGGame Methods ---

// --- GITCGState Methods ---

GITCGState::GITCGState(std::shared_ptr<const Game> game, gitcg_game_t gitcg_game_instance)
    : State(game),
      gitcg_game_instance_(gitcg_game_instance),
      current_player_(kInvalidPlayer) { // Initialize to invalid, will be updated
  // It's generally safer to call thread-specific initialization here if each state
  // might be handled by a different thread, or if the game instance itself is not
  // inherently thread-safe for all operations without this.
  // However, if gitcg_game_new already handles this or if your threading model
  // ensures calls from the same thread that did global init, this might not be needed.
  // For now, we assume global init is sufficient or gitcg handles internal threading.
  // gitcg_thread_initialize(); // Potentially needed

  // Update current player and other initial state properties from gitcg_game_instance_
  UpdateStateFromGameInstance();
}

GITCGState::~GITCGState() {
  if (gitcg_game_instance_ != nullptr) {
    gitcg_game_free(gitcg_game_instance_);
    gitcg_game_instance_ = nullptr;
  }
  // gitcg_thread_cleanup(); // Potentially needed if thread_initialize was called
}

Player GITCGState::CurrentPlayer() const {
  if (IsTerminal()) {
    return kTerminalPlayerId;
  }
  return current_player_;
}

std::unique_ptr<State> GITCGState::Clone() const {
  if (gitcg_game_instance_ == nullptr) {
    SpielFatalError("Clone called on a null gitcg_game_instance_");
  }

  gitcg_game_t cloned_gitcg_game_instance = nullptr;
  int err = gitcg_game_clone(gitcg_game_instance_, &cloned_gitcg_game_instance);
  if (err != 0 || cloned_gitcg_game_instance == nullptr) {
    SpielFatalError(absl::StrCat("GITCGState::Clone: Failed to clone gitcg_game: error ", err));
  }

  // The new GITCGState will take ownership of the cloned_gitcg_game_instance.
  return std::make_unique<GITCGState>(game_, cloned_gitcg_game_instance);
}

void GITCGState::UpdateStateFromGameInstance() {
  if (gitcg_game_instance_ == nullptr) {
    SpielFatalError("UpdateStateFromGameInstance called with null gitcg_game_instance_");
    return;
  }

  int is_terminated = 0;
  int err = gitcg_game_get_attr_int(gitcg_game_instance_, GITCG_ATTR_GAME_IS_TERMINATED, &is_terminated);
  if (err != 0) {
    SpielLog::Warn(absl::StrCat("Failed to get GITCG_ATTR_GAME_IS_TERMINATED: error ", err));
    // Default to not terminated or handle error appropriately
  }
  is_terminal_ = (is_terminated != 0);

  if (is_terminal_) {
    current_player_ = kTerminalPlayerId;
    // Cache returns if terminal
    cached_returns_.resize(game_->NumPlayers());
    for (Player p = 0; p < game_->NumPlayers(); ++p) {
      int player_status = 0; // 0: Playing, 1: Won, 2: Lost, 3: Draw
      err = gitcg_game_get_player_status(gitcg_game_instance_, p, &player_status);
      if (err != 0) {
        SpielLog::Warn(absl::StrCat("Failed to get player status for player ", p, ": error ", err));
        cached_returns_[p] = 0.0; // Default or error value
      } else {
        if (player_status == GITCG_PLAYER_STATUS_WON) {
          cached_returns_[p] = 1.0;
        } else if (player_status == GITCG_PLAYER_STATUS_LOST) {
          cached_returns_[p] = -1.0;
        } else { // Playing or Draw
          cached_returns_[p] = 0.0;
        }
      }
    }
  } else {
    int player_id = kInvalidPlayer;
    err = gitcg_game_get_attr_int(gitcg_game_instance_, GITCG_ATTR_GAME_CURRENT_PLAYER_ID, &player_id);
    if (err != 0) {
      SpielFatalError(absl::StrCat("Failed to get GITCG_ATTR_GAME_CURRENT_PLAYER_ID: error ", err));
    }
    current_player_ = static_cast<Player>(player_id);

    // Check for chance node if current player is kChancePlayerId
    // This depends on how gitcg represents chance nodes.
    // For now, assuming direct player turns or terminal.
    // If gitcg has specific chance events (e.g. dice rolls before player turn),
    // this logic needs to be more sophisticated.
    // Example:
    // int phase = 0;
    // gitcg_game_get_attr_int(gitcg_game_instance_, GITCG_ATTR_GAME_PHASE, &phase);
    // if (phase == GITCG_PHASE_ROLL_DICE_P0 || phase == GITCG_PHASE_ROLL_DICE_P1) {
    //   current_player_ = kChancePlayerId;
    //   // Populate chance_outcomes_ if this is a chance node
    // }
  }
  // Clear cached legal actions as they are now invalid.
  cached_legal_actions_.reset();
}

// Placeholder for parsing actions from gitcg.
// This will be highly dependent on the format of actions from gitcg_game_get_available_actions.
// For now, assume it returns a JSON string that needs parsing.
// We also need a mapping from these parsed actions to OpenSpiel Action (int).
std::vector<Action> GITCGState::ParseLegalActions() const {
    std::vector<Action> actions;
    char* actions_json_c_str = nullptr;
    int err = gitcg_game_get_available_actions(gitcg_game_instance_, current_player_, &actions_json_c_str);

    if (err != 0 || actions_json_c_str == nullptr) {
        SpielLog::Warn(absl::StrCat("ParseLegalActions: Failed to get available actions: error ", err));
        if (actions_json_c_str) gitcg_free_buffer(actions_json_c_str); // Free if allocated despite error
        return actions; // Return empty list
    }

    std::string actions_json(actions_json_c_str);
    gitcg_free_buffer(actions_json_c_str); // IMPORTANT: Free the buffer from gitcg

    SpielLog::Info(absl::StrCat("Player ", current_player_, " available actions JSON: ", actions_json));

    // TODO: Implement robust JSON parsing here.
    // The structure of actions_json needs to be known.
    // Example: If JSON is like `[{"id": 0, "desc": "EndRound"}, {"id": 1, "desc": "PlayCard", "card_idx": 5}]`
    // For now, let's assume a very simple scenario where the JSON contains a list of integer action IDs.
    // This is a MAJOR placeholder.
    if (actions_json.find("EndRound") != std::string::npos) { // Highly simplified example
        actions.push_back(0); // Assume 0 is "EndRound"
    }
    if (actions_json.find("PlayCard") != std::string::npos) {
        actions.push_back(1); // Assume 1 is some "PlayCard" action
    }
    // ... and so on. A proper mapping or dynamic action generation is needed.

    if (actions.empty() && !is_terminal_) {
      // This might indicate an issue or a state where "pass" is the only option,
      // or the JSON parsing is incomplete.
      SpielLog::Warn("ParseLegalActions: No actions parsed from JSON, but state is not terminal. JSON was: " + actions_json);
      // Depending on game rules, an empty action list might be valid (e.g. forced pass)
      // or it might be an error. If it's a forced pass, OpenSpiel expects a single "pass" action.
      // For now, we return the (potentially empty) list.
    }
    return actions;
}

std::vector<Action> GITCGState::LegalActions() const {
  if (IsTerminal()) {
    return {};
  }
  // TODO (State LegalActions): Handle kChancePlayerId separately if gitcg has explicit chance nodes
  // that require specific actions (e.g., outcomes of a dice roll).
  // if (current_player_ == kChancePlayerId) {
  //   return GetChanceOutcomes(); // This method would need to be implemented based on gitcg's chance mechanics.
  // }

  if (!cached_legal_actions_) {
      // Use the utility function from gi_tcg_utils.
      // It needs the raw JSON string from gitcg.
      char* actions_json_c_str = nullptr;
      int err = gitcg_game_get_available_actions(gitcg_game_instance_, current_player_, &actions_json_c_str);

      if (err != 0 || actions_json_c_str == nullptr) {
          SpielLog::Warn(absl::StrCat("GITCGState::LegalActions: Failed to get available actions JSON from gitcg: error ", err));
          if (actions_json_c_str) gitcg_free_buffer(actions_json_c_str);
          cached_legal_actions_ = std::vector<Action>(); // Return empty list on error
      } else {
          std::string actions_json_str(actions_json_c_str);
          gitcg_free_buffer(actions_json_c_str);
          // TODO (State LegalActions): Ensure `*game_` can be safely passed as `const Game&`.
          // The utils::ParseActionsFromJson might need more specific game type (const GITCGGame&)
          // if it needs to access game-specific action mapping details not on the base Game class.
          cached_legal_actions_ = utils::ParseActionsFromJson(actions_json_str, current_player_, *game_, gitcg_game_instance_);
      }
  }
  return *cached_legal_actions_;
}

void GITCGState::DoApplyAction(Action action_id) {
  if (gitcg_game_instance_ == nullptr) {
    SpielFatalError("DoApplyAction called on a null gitcg_game_instance_");
  }

  // TODO (State DoApplyAction): Convert OpenSpiel Action ID to gitcg JSON action string.
  // This is a critical step and needs a robust implementation in gi_tcg_utils.cc.
  // The current utils function is a placeholder.
  std::string action_json_str = utils::ConvertOpenSpielActionToGitcgActionJson(action_id, current_player_, *this, gitcg_game_instance_);

  SpielLog::Info(absl::StrCat("Player ", current_player_, " applying action ID ", action_id, ", JSON: ", action_json_str));

  int err = gitcg_game_action(gitcg_game_instance_, current_player_, action_json_str.c_str());
  if (err != 0) {
    // TODO (State DoApplyAction): Improve error handling. What if an action is illegal according to gitcg?
    // Should this be caught earlier by LegalActions, or is this a fallback?
    // Consider if SpielFatalError is too harsh or if a warning/specific error state is better.
    // The error code from gitcg might give more insight.
    SpielFatalError(absl::StrCat("DoApplyAction: gitcg_game_action failed for player ",
                                 current_player_, " with action JSON '", action_json_str,
                                 "': error ", err, " (see gitcg_error_code_t)"));
  }

  // After applying the action, the internal state of gitcg_game_instance_ has changed.
  // We need to update our cached view of the state.
  UpdateStateFromGameInstance();

  // TODO (State DoApplyAction): Handle RPC/Notifications if gitcg_game_action can trigger them.
  // The gitcg.h mentions callbacks. If gitcg_game_action can enter a state where it
  // needs to call back (e.g., for a complex sequence or an opponent's reaction outside their turn),
  // that logic needs to be handled here or in a loop after this call.
  // For now, assuming gitcg_game_action fully processes the action and updates state directly.
}

std::string GITCGState::ActionToString(Player player, Action action_id) const {
  // TODO (State ActionToString): Implement a meaningful conversion of action ID to string.
  // This might involve:
  // 1. Calling a utility function (e.g., utils::ActionIdToDetailedString) that
  //    reconstructs or looks up the action details.
  // 2. If actions are simple and few, a direct switch/map here.
  // 3. For complex actions, it might involve generating the JSON via
  //    utils::ConvertOpenSpielActionToGitcgActionJson and returning that, or a summary of it.
  // The current placeholder just returns the ID.
  // A more descriptive string is highly recommended for debugging and UIs.
  // Example: "PlayCard(Ganyu, Target: OpponentActive)" or "EndRound"
  // This might require access to game-specific action mappings or the gitcg_game_instance
  // to query action details if the ID alone is not enough.
  return absl::StrCat("Action(", action_id, ")");
}

std::string GITCGState::ToString() const {
  if (gitcg_game_instance_ == nullptr) {
    return "State(null game instance)";
  }
  // TODO (State ToString): Provide a more comprehensive string representation.
  // This could involve:
  // 1. Getting the full JSON observation for both players (or a neutral observer view if available).
  // 2. Summarizing key aspects: current player, phase, scores, active characters, hand sizes.
  // 3. Using gitcg_game_get_attr_string if there are useful general state strings.
  // The current implementation is minimal.
  std::string obs_p0 = utils::GetObservationJsonString(gitcg_game_instance_, 0);
  std::string obs_p1 = utils::GetObservationJsonString(gitcg_game_instance_, 1);

  return absl::StrCat(
      "GITCGState(Player: ", CurrentPlayer(),"\n",
      "IsTerminal: ", IsTerminal() ? "Yes" : "No","\n",
      "P0_Obs_JSON: ", obs_p0.substr(0, 100), "...","\n",
      "P1_Obs_JSON: ", obs_p1.substr(0, 100), "...","\n",
      // TODO (State ToString): Add more details like turn number, phase, etc.
      // "Round: ", round_number_from_gitcg, ", Phase: ", phase_from_gitcg
      ")");
}

bool GITCGState::IsTerminal() const {
  // is_terminal_ is updated by UpdateStateFromGameInstance()
  return is_terminal_;
}

std::vector<double> GITCGState::Returns() const {
  if (!IsTerminal()) {
    // Standard behavior: returns are 0 for non-terminal states.
    return std::vector<double>(game_->NumPlayers(), 0.0);
  }
  // cached_returns_ is populated in UpdateStateFromGameInstance when terminal.
  return cached_returns_;
}

std::string GITCGState::ObservationString(Player player) const {
  if (gitcg_game_instance_ == nullptr) {
    SpielLog::Warn("ObservationString called on a null gitcg_game_instance_");
    return "Error: Null game instance";
  }
  // TODO (State ObservationString): Ensure the JSON from gitcg is suitable for a string observation.
  // It might be very verbose. Consider if a summary or a specific part of the JSON
  // would be more appropriate, or if gitcg offers a different "summary string" API.
  // For now, returning the full JSON observation for the specified player.
  return utils::GetObservationJsonString(gitcg_game_instance_, player);
}

// ObservationTensor is defined in OpenSpiel to use float.
void GITCGState::ObservationTensor(Player player, absl::Span<float> values) const {
  if (gitcg_game_instance_ == nullptr) {
    SpielLog::Error("ObservationTensor called on a null gitcg_game_instance_");
    std::fill(values.begin(), values.end(), 0.0f);
    return;
  }
  // TODO (State ObservationTensor): Verify that the size of `values` span matches
  // the game's ObservationTensorShape(). This check should ideally be in OpenSpiel's
  // framework or done by the caller, but a local check can be useful.
  // SPIEL_CHECK_EQ(values.size(), game_->ObservationTensorSize());

  // Create a temporary std::vector<float> to pass to the utility function.
  // This is because the utility function is defined to take std::vector<float>*.
  // Alternatively, the utility function could be templated or overloaded for absl::Span.
  std::vector<float> tensor_data_vec(values.size());

  // Use the utility function to populate the tensor data.
  // The utility function (utils::PopulateObservationTensor) is responsible for
  // getting the JSON from gitcg and converting it to a flat tensor.
  // TODO (State ObservationTensor): Ensure utils::PopulateObservationTensor correctly handles errors
  // and fills the tensor appropriately (e.g., with zeros on error).
  utils::PopulateObservationTensor(gitcg_game_instance_, player, *game_, &tensor_data_vec);

  // Copy the data from the temporary vector to the provided absl::Span.
  std::copy(tensor_data_vec.begin(), tensor_data_vec.end(), values.begin());
}


// --- End of GITCGState Methods ---

}  // namespace gi_tcg
}  // namespace open_spiel
