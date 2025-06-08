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

#include "open_spiel/games/gi_tcg_ai/gi_tcg_utils.h"
#include "open_spiel/games/gi_tcg_ai/gi_tcg.h" // For GITCGGame, GITCGState potentially

#include "open_spiel/abseil-cpp/absl/strings/str_cat.h"
#include "open_spiel/spiel_utils.h" // For SpielLog, SpielFatalError
#include <cstdlib> // For gitcg_free_buffer

// TODO (Utils JSON Library): If complex JSON parsing is needed, include a library.
// e.g., #include "third_party/nlohmann/json.hpp"
// using json = nlohmann::json;
// This will simplify parsing and constructing JSON strings significantly.

namespace open_spiel {
namespace gi_tcg {
namespace utils {

// TODO (ParseActionsFromJson): This is a critical placeholder implementation.
// Replace with actual JSON parsing and mapping to OpenSpiel Action IDs.
// The structure of actions_json_str (from gitcg_game_get_available_actions)
// and the mapping logic are fundamental for the game to work.
// - How are actions represented in the JSON? (e.g., array of objects, each with type, params)
// - How do these map to unique integer Action IDs for OpenSpiel?
// - Is the number of distinct actions fixed, or does it vary? This impacts game.NumDistinctActions().
// - If actions have parameters (e.g., play card X on target Y), how are these encoded
//   into a single OpenSpiel Action ID and decoded back?
std::vector<Action> ParseActionsFromJson(const std::string& actions_json_str,
                                       Player player,
                                       const Game& game, /* const GITCGGame* gi_tcg_game */
                                       gitcg_game_t game_instance) {
    std::vector<Action> actions;
    SpielLog::Warn(absl::StrCat("ParseActionsFromJson (utils) is a placeholder. JSON for player ", player, ": ", actions_json_str.substr(0, 200)));

    // const auto* gi_tcg_game_ptr = dynamic_cast<const GITCGGame*>(&game);
    // if (!gi_tcg_game_ptr) {
    //     SpielFatalError("ParseActionsFromJson called with incorrect game type.");
    // }

    // --- Placeholder JSON Parsing Logic ---
    // This needs to be replaced with robust parsing based on the actual JSON format from gitcg.
    // Example: If JSON is `[{"id": 0, "name": "EndRound"}, {"id": 1, "name": "PlayCard", "card_idx": 5, "target_idx": 0}]`
    //
    // Using a real JSON library:
    // try {
    //   json parsed_json = json::parse(actions_json_str);
    //   if (parsed_json.is_array()) {
    //     for (const auto& item : parsed_json) {
    //       // TODO (ParseActionsFromJson): Extract action type and parameters.
    //       // TODO (ParseActionsFromJson): Map extracted action to an OpenSpiel Action ID.
    //       // This mapping is crucial. For example, if NumDistinctActions is small and fixed,
    //       // you might have a direct map. If large/dynamic, you might need a scheme
    //       // to encode action type + params into a single int.
    //       // int action_type = item.value("type", -1);
    //       // if (action_type == 0) actions.push_back(game.GetActionIdForEndRound());
    //       // else if (action_type == 1) {
    //       //   int card_id = item.value("card_id", -1);
    //       //   actions.push_back(game.GetActionIdForPlayCard(card_id));
    //       // }
    //     }
    //   }
    // } catch (const json::parse_error& e) {
    //   SpielLog::Error(absl::StrCat("JSON parse error in ParseActionsFromJson: ", e.what()));
    //   return {}; // Return empty on error
    // }

    // --- Simplified Placeholder (REMOVE THIS FOR REAL IMPLEMENTATION) ---
    if (actions_json_str.find("EndRound") != std::string::npos) { actions.push_back(0); } // Assume 0 = EndRound
    if (actions_json_str.find("PlayCard") != std::string::npos) { actions.push_back(1); } // Assume 1 = Some PlayCard
    if (actions_json_str.find("Skill") != std::string::npos) { actions.push_back(2); }    // Assume 2 = Some Skill
    // This is extremely naive and will likely not work correctly.

    if (actions.empty() && !actions_json_str.empty() && actions_json_str != "[]" && actions_json_str != "{}") {
         SpielLog::Warn("ParseActionsFromJson: Parsed no actions from non-empty, non-trivial JSON. This might be an error or an unhandled action format.");
    }
    // TODO (ParseActionsFromJson): If no actions are genuinely available for a player
    // (and it's not a terminal state or chance node), OpenSpiel might expect a single "pass" action.
    // Clarify this behavior based on OpenSpiel conventions and game rules.
    // If gitcg indicates no actions, and it's not terminal, this might be a "forced pass" situation.
    // In such cases, OpenSpiel might require a specific "pass" action to be returned.
    // if (actions.empty() && !is_terminal && !is_chance_node) { actions.push_back(PASS_ACTION_ID); }

    return actions;
}

// TODO (ConvertOpenSpielActionToGitcgActionJson): This is a critical placeholder.
// Replace with logic to convert an OpenSpiel Action ID to the JSON format gitcg_game_action expects.
// This requires knowing the full action space and how gitcg represents each action in JSON.
// - How was the OpenSpiel Action ID generated in ParseActionsFromJson? (Inverse of that mapping)
// - What JSON structure does gitcg_game_action() expect for each type of action?
//   (e.g., `{"type": "SKILL", "char_idx": 0, "skill_idx": 1}`)
std::string ConvertOpenSpielActionToGitcgActionJson(Action action_id,
                                                  Player player,
                                                  const State& state, /* const GITCGState* gi_tcg_state */
                                                  gitcg_game_t game_instance) {
    SpielLog::Warn(absl::StrCat("ConvertOpenSpielActionToGitcgActionJson (utils) is a placeholder for action: ", action_id, " for player ", player));

    // const auto* gi_tcg_state_ptr = dynamic_cast<const GITCGState*>(&state);
    // if (!gi_tcg_state_ptr) {
    //     SpielFatalError("ConvertOpenSpielActionToGitcgActionJson called with incorrect state type.");
    // }
    // const auto* gi_tcg_game_ptr = dynamic_cast<const GITCGGame*>(state.GetGame().get());
    // if (!gi_tcg_game_ptr) {
    //     SpielFatalError("ConvertOpenSpielActionToGitcgActionJson called with incorrect game type from state.");
    // }


    // --- Placeholder JSON Generation Logic ---
    // This needs to map `action_id` back to a meaningful JSON structure for gitcg.
    // This is the inverse of the logic in ParseActionsFromJson.
    //
    // Using a real JSON library:
    // json action_json;
    // // TODO (ConvertOpenSpielActionToGitcgActionJson): Decode action_id to type and params.
    // // ActionType type = gi_tcg_game_ptr->GetActionType(action_id);
    // // ActionParams params = gi_tcg_game_ptr->GetActionParams(action_id);
    // // if (type == ActionType::EndRound) {
    // //   action_json["type"] = "END_ROUND";
    // // } else if (type == ActionType::PlayCard) {
    // //   action_json["type"] = "PLAY_CARD";
    // //   action_json["card_id"] = params.card_id;
    // //   action_json["target_idx"] = params.target_idx; // if applicable
    // // }
    // // return action_json.dump();

    // --- Simplified Placeholder (REMOVE THIS FOR REAL IMPLEMENTATION) ---
    std::string action_json_str = "{}"; // Default empty/invalid action
    if (action_id == 0) { // Assuming 0 was "EndRound"
        action_json_str = "{\"type\":\"END_ROUND\"}"; // Fictional JSON
    } else if (action_id == 1) { // Assuming 1 was "PlayCard"
        // This would be more complex: need to know which card, target, etc.
        // This information must be derivable from `action_id` or `state`.
        action_json_str = "{\"type\":\"PLAY_CARD\", \"card_id\":10101, \"target_idx\":0}"; // Fictional
    } else if (action_id == 2) { // Assuming 2 was "Skill"
        action_json_str = "{\"type\":\"USE_SKILL\", \"char_idx\":0, \"skill_id\":1}"; // Fictional
    }
    // This is extremely naive.

    // TODO (ConvertOpenSpielActionToGitcgActionJson): Ensure the generated JSON is exactly what
    // gitcg_game_action expects. This is a common source of integration errors.
    // Validate the JSON structure against cbinding documentation or examples.
    SpielLog::Info(absl::StrCat("Player ", player, " sending action JSON: ", action_json_str));
    return action_json_str;
}

// TODO (PopulateObservationTensor): This is a critical placeholder.
// Replace with actual JSON parsing of the observation and mapping to the tensor.
// The tensor's shape and meaning of its elements must be clearly defined and
// consistent with GITCGGame::ObservationTensorShape().
void PopulateObservationTensor(gitcg_game_t game_instance,
                               Player player,
                               const Game& game, /* const GITCGGame* gi_tcg_game */
                               std::vector<float>* tensor_data) { // Changed to float
    SpielLog::Warn(absl::StrCat("PopulateObservationTensor (utils) for player ", player, " is a placeholder."));

    // const auto* gi_tcg_game_ptr = dynamic_cast<const GITCGGame*>(&game);
    // if (!gi_tcg_game_ptr) {
    //     SpielFatalError("PopulateObservationTensor called with incorrect game type.");
    // }
    // SPIEL_CHECK_EQ(tensor_data->size(), gi_tcg_game_ptr->ObservationTensorSize());


    std::string obs_json_str = GetObservationJsonString(game_instance, player);
    if (obs_json_str.empty() || obs_json_str == "{}" || obs_json_str == "null") {
        SpielLog::Warn("PopulateObservationTensor: Observation JSON is empty, default, or null. Filling tensor with zeros.");
        std::fill(tensor_data->begin(), tensor_data->end(), 0.0f);
        return;
    }
    SpielLog::Info(absl::StrCat("PopulateObservationTensor for player ", player, ", JSON: ", obs_json_str.substr(0, 200)));

    // TODO (PopulateObservationTensor): Parse obs_json_str and fill tensor_data.
    // This requires knowing the structure of the JSON from gitcg_game_get_observation_json
    // and how it maps to the flat tensor_data vector.
    // - What features are in the JSON? (e.g., player health, card counts, character states)
    // - How are these features ordered and scaled in the tensor?
    // - Ensure the total number of features matches game.ObservationTensorShape().
    //
    // Using a real JSON library:
    // try {
    //   json obs = json::parse(obs_json_str);
    //   int feature_idx = 0;
    //   auto add_feature = [&](float val) {
    //     if (feature_idx < tensor_data->size()) {
    //       (*tensor_data)[feature_idx++] = val;
    //     } else {
    //       SpielLog::Error("PopulateObservationTensor: Too many features for tensor size!");
    //     }
    //   };
    //
    //   // Example: Player 0's active character's HP
    //   // add_feature(obs.value_at_path("/player0/active_char/hp", 0.0f));
    //   // add_feature(obs.value_at_path("/player0/hand_size", 0.0f));
    //   // ... many more features
    //
    //   // Fill remaining tensor with 0s if not all features were parsed or if JSON was incomplete.
    //   std::fill(tensor_data->begin() + feature_idx, tensor_data->end(), 0.0f);
    //
    // } catch (const json::parse_error& e) {
    //   SpielLog::Error(absl::StrCat("JSON parse error in PopulateObservationTensor: ", e.what()));
    //   std::fill(tensor_data->begin(), tensor_data->end(), 0.0f); // Zero out on error
    //   return;
    // }


    // --- Simplified Placeholder (REMOVE THIS FOR REAL IMPLEMENTATION) ---
    // For now, as a placeholder, fill with a pattern if JSON is not empty.
    // This is just to have some non-zero data for testing and will not be meaningful.
    for (size_t i = 0; i < tensor_data->size(); ++i) {
        (*tensor_data)[i] = static_cast<float>((i + player + obs_json_str.length()) % 20) / 20.0f; // Placeholder
    }
    // This is extremely naive.
}

// Helper function to get observation JSON string from gitcg
std::string GetObservationJsonString(gitcg_game_t game_instance, Player player) {
    if (game_instance == nullptr) {
        SpielLog::Warn("GetObservationJsonString: gitcg_game_instance is null.");
        return "{}"; // Return empty JSON object as string
    }

    char* obs_json_c_str = nullptr;
    // TODO (GetObservationJsonString): Clarify if player_id for gitcg_game_get_observation_json
    // should be the OpenSpiel Player (0 or 1) or some other ID if gitcg uses a different convention.
    // Assuming OpenSpiel Player ID (0 for P1, 1 for P2) is directly usable by gitcg.
    // The `gitcg.h` uses `player_id` which typically implies 0-indexed.
    int err = gitcg_game_get_observation_json(game_instance, static_cast<int>(player), &obs_json_c_str);

    if (err != 0) {
        SpielLog::Warn(absl::StrCat("GetObservationJsonString: Failed to get observation JSON for player ", player, ": error ", err, " (see gitcg_error_code_t)"));
        if (obs_json_c_str) gitcg_free_buffer(obs_json_c_str); // Free if allocated despite error
        return "{}"; // Or perhaps throw, or return an error indicator
    }

    if (obs_json_c_str == nullptr) {
        // This case might be valid if gitcg returns null for "no observation available yet"
        // or it could be an unexpected error.
        SpielLog::Warn(absl::StrCat("GetObservationJsonString: gitcg_game_get_observation_json returned null C-string for player ", player, ". This might be an error or intended if no observation is ready."));
        return "{}"; // Default to empty JSON
    }

    std::string obs_json_str(obs_json_c_str);
    gitcg_free_buffer(obs_json_c_str); // IMPORTANT: Free the buffer from gitcg
    return obs_json_str;
}

}  // namespace utils
}  // namespace gi_tcg
}  // namespace open_spiel
