// Copyright 2025 genius-invokation
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

#ifndef OPEN_SPIEL_GAMES_GI_TCG_GI_TCG_UTILS_H_
#define OPEN_SPIEL_GAMES_GI_TCG_GI_TCG_UTILS_H_

#include <string>
#include <vector>
#include "open_spiel/spiel.h" // For Action, Player, State, Game
// TODO (Utils Build): Ensure gitcg.h is findable by the build system.
// This path assumes cbinding/include is in the include path.
// Verify that CMakeLists.txt for open_spiel correctly adds this include directory
// from the genius-invokation workspace, e.g., using an absolute path or a relative
// path if the build structure guarantees it.
// Example CMake: include_directories(/path/to/genius-invokation/packages/cbinding/include)
#include "gitcg/gitcg.h" // For gitcg_game_t and other gitcg types

// TODO (Utils General): Consider using a more robust JSON parsing library (e.g., nlohmann/json or Abseil's JSON)
// if the JSON structures for actions or observations become complex.
// The current placeholders are very basic and not suitable for production.

namespace open_spiel {
namespace gi_tcg {

// Forward declaration for GITCGGame to access game-specific parameters if needed by utils.
class GITCGGame;
// Forward declaration for GITCGState if utils need to inspect state details (less common).
// class GITCGState;


namespace utils {

// TODO (ParseActionsFromJson): Define the actual structure of the actions JSON from gitcg.
// The current implementation in .cc file is a placeholder.
// This function needs to map the JSON actions to OpenSpiel Action IDs (int).
// This mapping might be:
// 1. Dynamic: Based on the JSON content, assign IDs on the fly. This makes NumDistinctActions complex.
// 2. Static: A predefined mapping if actions are fixed or can be enumerated.
// 3. Hybrid: Some actions static, some dynamic.
// Consider how to handle NumDistinctActions() in the Game class based on this.
// It might need to be an upper bound, or actions might be mapped to a fixed-size space.
// The `game` parameter can be used to access game-specific information like NumDistinctActions()
// or specific action mappings if stored in the GITCGGame object.
std::vector<Action> ParseActionsFromJson(const std::string& actions_json_str,
                                       Player player,
                                       const Game& game,
                                       gitcg_game_t game_instance); // Added game_instance if needed for context

// TODO (ConvertOpenSpielActionToGitcgActionJson): Define how an OpenSpiel Action (int)
// maps to a gitcg action JSON string that gitcg_game_action() expects.
// This is highly dependent on the gitcg API and the specific actions the game supports.
// The current implementation in .cc file is a placeholder.
// The `state` parameter can provide context if the JSON depends on the current game situation.
// This function is critical for sending player moves to the C library.
std::string ConvertOpenSpielActionToGitcgActionJson(Action action_id,
                                                  Player player,
                                                  const State& state,
                                                  gitcg_game_t game_instance); // Added game_instance

// TODO (PopulateObservationTensor): Define the structure of the observation JSON from gitcg
// (obtained via gitcg_game_get_observation_json) and how it maps to the flat observation tensor.
// The tensor shape defined in GITCGGame::ObservationTensorShape() must be consistent with this.
// The current implementation in .cc file is a placeholder.
// The `game` parameter provides access to ObservationTensorShape().
// The `player` parameter specifies for whom the observation is.
void PopulateObservationTensor(gitcg_game_t game_instance,
                               Player player,
                               const Game& game,
                               std::vector<float>* tensor_data); // Changed to float as per OpenSpiel convention

// TODO (GetObservationJsonString): Ensure error handling for gitcg_game_get_observation_json is robust.
// The current implementation in .cc file has basic error checks.
// Consider what to return or how to signal an error if JSON retrieval fails critically.
// The `player` parameter is the perspective for the observation.
std::string GetObservationJsonString(gitcg_game_t game_instance, Player player);

// TODO (ActionToStringUtils): Create a utility function, possibly here or in GITCGState,
// that converts an OpenSpiel Action ID to a human-readable string.
// This might involve parsing the action JSON from gitcg or having a predefined mapping.
// This is useful for ActionToString in GITCGState.
// std::string ActionIdToDetailedString(Action action_id, Player player, gitcg_game_t game_instance);

// TODO (ParseDeckJsonForGame): The ParseDeckJson function is currently a private helper in gi_tcg.cc.
// If deck parsing logic becomes more complex or needs to be used by other utilities
// (e.g., for validation or analysis tools), consider moving it to gi_tcg_utils.h/cc.
// For now, it's specific to GITCGGame initialization.
// void ParseDeckJson(const std::string& deck_json, std::vector<int>& char_ids, std::vector<int>& card_ids);

}  // namespace utils
}  // namespace gi_tcg
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_GI_TCG_GI_TCG_UTILS_H_
