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

#ifndef OPEN_SPIEL_GAMES_GI_TCG_H_ // Standard include guard
#define OPEN_SPIEL_GAMES_GI_TCG_H_

#include <memory>      // For std::shared_ptr, std::unique_ptr
#include <string>      // For std::string
#include <vector>      // For std::vector
#include <map>         // For std::map (example for action mapping)

#include "open_spiel/spiel.h" // OpenSpiel core header
// Include the C binding header for the Genius Invokation TCG.
// The path "gitcg/gitcg.h" assumes that the include directory
// containing `gitcg` (i.e., /workspaces/genius-invokation/packages/cbinding/include/)
// has been added to the compiler's include search paths in CMakeLists.txt.
#include "gitcg/gitcg.h"

namespace open_spiel {
namespace gi_tcg { // Namespace for this specific game

// Forward declaration of the game class.
// GITCGState will hold a pointer/reference to it.
class GITCGGame;

// Structure to manage pending RPC (Remote Procedure Call) requests from the gitcg library.
// The gitcg library might use an RPC-like mechanism where it pauses execution
// (e.g., during gitcg_game_step) and calls a registered C callback (StaticRpcCallback)
// to ask the OpenSpiel side for a player's action.
struct GITCGPendingRpcRequest {
  bool active = false; // True if there is an outstanding request waiting for a response.
  std::vector<char> request_data_buffer; // Stores the raw request data from gitcg.
                                         // The format of this data is defined by gitcg.
  // Potentially add more fields here, e.g.:
  // Player for_player; // Which player is this request for?
  // int request_type;  // If gitcg can make different types of requests.
};

// Represents the state of a Genius Invokation TCG game.
// Inherits from OpenSpiel's `State` class.
class GITCGState : public State {
 public:
  // Constructor.
  // `game`: A shared pointer to the parent GITCGGame object (provides game-wide info).
  // `gitcg_game_instance`: A handle to the game simulation instance managed by the gitcg library.
  GITCGState(std::shared_ptr<const Game> game, gitcg_game_t gitcg_game_instance);

  // Copy constructor, primarily used for the `Clone()` method.
  // Needs to correctly duplicate the underlying gitcg game state.
  GITCGState(const GITCGState& other);

  // Destructor. Should free any resources held by this state, particularly related to gitcg.
  ~GITCGState() override;

  // --- Standard OpenSpiel State API methods ---
  Player CurrentPlayer() const override; // Returns the player whose turn it is.
  // Converts an action ID (integer) to a human-readable string (for logging/debugging).
  std::string ActionToString(Player player, Action action_id) const override;
  // Returns a string representation of the current game state (for logging/debugging).
  std::string ToString() const override;
  // Checks if the game has finished.
  bool IsTerminal() const override;
  // Returns the game-end scores for each player if the game is terminal.
  std::vector<double> Returns() const override;
  // Returns a string observation for a given player (e.g., for UIs or simple agents).
  std::string ObservationString(Player player) const override;
  // Fills a pre-allocated tensor with a numerical observation for a given player.
  void ObservationTensor(Player player, absl::Span<float> values) const override;
  // Creates a deep copy of the current state.
  std::unique_ptr<State> Clone() const override;
  // Returns a list of legal actions (as integer IDs) for the current player.
  std::vector<Action> LegalActions() const override;

 protected:
  // Applies the given action (integer ID) to the current state.
  // This is where the interaction with `gitcg_game_instance_` to perform an action happens.
  void DoApplyAction(Action action_id) override;

 private:
  // Allows GITCGGame to access private members of GITCGState, useful for initialization.
  friend class GITCGGame;

  // --- Static C-style callbacks for gitcg ---
  // These functions are passed as function pointers to the gitcg library.
  // They act as bridges, forwarding calls to the appropriate GITCGState instance methods.
  // `instance_ptr` will be a `void*` pointer to the `GITCGState` object.

  // Callback for gitcg to request an action or other RPC-like interaction.
  static void StaticRpcCallback(void* instance_ptr, const char* request_data,
                                 size_t request_len, char* response_data,
                                 size_t* response_len);
  // Callback for gitcg to send notifications (e.g., game events not directly tied to actions).
  static void StaticNotificationCallback(void* instance_ptr,
                                          const char* notification_data,
                                          size_t notification_len);
  // Callback for gitcg to report I/O or other internal errors.
  static void StaticIoErrorCallback(void* instance_ptr,
                                     const char* error_message);

  // --- Instance methods called by the static callbacks ---
  // These methods contain the actual logic for handling callbacks for this specific state object.

  void InstanceRpcCallback(const char* request_data, size_t request_len,
                           char* response_data, size_t* response_len);
  void InstanceNotificationCallback(const char* notification_data,
                                    size_t notification_len);
  void InstanceIoErrorCallback(const char* error_message);

  // Helper method to register the static callbacks with the `gitcg_game_instance_`.
  void InitializeGitcgCallbacks();
  // Helper method to fetch the current state from `gitcg_game_instance_` and update `cached_gitcg_state_`.
  // Marked `mutable` if `cached_gitcg_state_` is updated within const methods.
  void UpdateCachedGameState() const;
  // Helper method to determine legal actions, likely by querying `cached_gitcg_state_` or `gitcg_game_instance_`.
  std::vector<Action> DetermineLegalActions() const;

  // --- Member Variables ---
  std::shared_ptr<const GITCGGame> parent_game_; // Pointer to the parent game object.
  gitcg_game_t gitcg_game_instance_; // Handle to the C game object from gitcg library.
                                     // This is the primary interface to the game's C API.

  // A cached copy of the gitcg state. `gitcg_game_get_state` can be used to populate this.
  // Useful for `const` methods in OpenSpiel that need to inspect state without altering the C game object.
  // Marked `mutable` because `gitcg_game_get_state` might not be const-correct, or caching happens in const methods.
  mutable gitcg_state_t cached_gitcg_state_;

  // Structures to hold information about pending RPC requests from gitcg.
  // If gitcg operates by turns, one might be sufficient, or one per player if simultaneous decisions can be requested.
  GITCGPendingRpcRequest pending_rpc_request_player0_;
  GITCGPendingRpcRequest pending_rpc_request_player1_;

  // Buffer to prepare the response data for an RPC callback.
  std::vector<char> rpc_response_buffer_;

  // Cached game status and winner to avoid redundant calls to gitcg.
  // Marked `mutable` as they might be updated in `const` methods like IsTerminal() or Returns().
  mutable int current_game_status_ = GITCG_GAME_STATUS_NOT_STARTED; // From gitcg_game_get_status.
  mutable int winner_ = -1; // -1 if no winner, 0 or 1 for the winning player.

  // Example for mapping between OpenSpiel Action IDs (integers) and game-specific action representations.
  // std::map<std::string, Action> action_name_to_id_;
  // std::vector<std::string> action_id_to_name_;
};

// Represents the Genius Invokation TCG game itself.
// Inherits from OpenSpiel's `Game` class.
class GITCGGame : public Game {
 public:
  // Constructor. Takes game parameters (e.g., from command line or config file).
  explicit GITCGGame(const GameParameters& params);
  // Destructor. Should ensure global gitcg resources are cleaned up if this is the last game instance.
  ~GITCGGame() override;

  // --- Standard OpenSpiel Game API methods ---
  // Returns the total number of distinct actions possible in the game.
  int NumDistinctActions() const override;
  // Creates and returns a new initial state for the game.
  std::unique_ptr<State> NewInitialState() const override;
  // Returns the number of players in the game (typically 2 for GCG).
  int NumPlayers() const override;
  // Minimum possible utility (score) a player can achieve.
  double MinUtility() const override;
  // Maximum possible utility (score) a player can achieve.
  double MaxUtility() const override;
  // For zero-sum games, this is typically 0. For others, absl::nullopt.
  absl::optional<double> UtilitySum() const override { return 0.0; }
  // Creates a clone of the game object (not the state).
  std::shared_ptr<const Game> Clone() const override;
  // Returns the shape of the observation tensor for ObservationTensor().
  std::vector<int> ObservationTensorShape() const override;
  // Returns the maximum theoretical game length (e.g., in moves or rounds).
  int MaxGameLength() const override;

  // --- Game-specific parameter accessors ---
  // These methods can be used by GITCGState to get game-wide configurations.
  // Example:
  // const std::string& GetPlayerDeckConfig(Player player) const;
  // long GetInitialRandomSeed() const;

  // --- Global gitcg library initialization and cleanup ---
  // Ensures `gitcg_initialize()` is called once before any game instances use the library.
  static void EnsureGitcgInitialized();
  // Ensures `gitcg_cleanup()` is called once when no more game instances are active.
  static void PerformGitcgCleanup();

 private:
  // Helper method to parse `GameParameters` and store them in member variables.
  void ParseGameParameters(const GameParameters& params);

  // --- Member Variables for game-wide settings ---
  // Store parsed game parameters that are needed to initialize `gitcg_state_createparam_t`.
  // Example:
  // std::map<int, std::vector<int>> player_character_cards_; // player_id -> list of character definition IDs
  // std::map<int, std::vector<int>> player_action_cards_;    // player_id -> list of card definition IDs for deck
  // long random_seed_;
  // bool no_shuffle_player0_;
  // bool no_shuffle_player1_;
  // int initial_hands_count_;
  // ... other relevant parameters from gitcg_state_createparam_set_attr_int/string

  int num_distinct_actions_; // Cached value for NumDistinctActions().
  int max_game_length_;      // Cached value for MaxGameLength().
  std::vector<int> observation_tensor_shape_; // Cached value for ObservationTensorShape().

  // --- Static members for managing global gitcg library state ---
  static bool gitcg_library_initialized_; // Flag to track if gitcg_initialize() has been called.
  static int active_game_instances_;    // Counter for active GITCGGame instances.
};

}  // namespace gi_tcg
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_GI_TCG_H_
