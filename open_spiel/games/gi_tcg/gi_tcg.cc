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

#include "open_spiel/abseil-cpp/absl/algorithm/container.h"
#include "open_spiel/abseil-cpp/absl/strings/str_cat.h"
#include "open_spiel/abseil-cpp/absl/strings/str_format.h"
#include "open_spiel/games/gi_tcg/gi_tcg_utils.h"
#include "open_spiel/games/gi_tcg/include/gitcg/gitcg.h"

namespace open_spiel {
namespace gi_tcg {
namespace {

const GameType kGameType{/*short_name=*/"gi_tcg",
                         /*long_name=*/"Genius Invokation",
                         GameType::Dynamics::kSequential,
                         GameType::ChanceMode::kSampledStochastic,
                         GameType::Information::kImperfectInformation,
                         GameType::Utility::kZeroSum,
                         GameType::RewardModel::kTerminal,
                         /*max_num_players=*/kNumPlayers,
                         /*min_num_players=*/kNumPlayers,
                         /*provides_information_state_string=*/true,
                         /*provides_information_state_tensor=*/false,
                         /*provides_observation_string=*/true,
                         /*provides_observation_tensor=*/true};
                          /*parameter_specification=*/
                        //  {{"filename", GameParameter(std::string(""))}}, //未必
                        //  /*default_loadable=*/false,
                        //  /*provides_factored_observation_string=*/false,
                        //  /*is_concrete=*/false};};

std::shared_ptr<const Game> Factory(const GameParameters& params) {
  return std::shared_ptr<const Game>(new GITCGGame(params));
}

REGISTER_SPIEL_GAME(kGameType, Factory);

RegisterSingleTensorObserver single_tensor(kGameType.short_name);

}  // namespace


// class GITCGObserver : public Observer {
//  public:
//   explicit GITCGObserver(IIGObservationType iig_obs_type)
//       : Observer(/*has_string=*/ObserverHasString(iig_obs_type),
//                  /*has_tensor=*/ObserverHasTensor(iig_obs_type)),
//         iig_obs_type_(iig_obs_type) {}


//   void WriteTensor(const State& observed_state, int player,
//                    Allocator* allocator) const override {
//     auto& state = open_spiel::down_cast<const GITCGState&>(observed_state);
//     SPIEL_CHECK_GE(player, 0);
//     SPIEL_CHECK_LT(player, state.num_players_);

//     if (iig_obs_type_.perfect_recall) {
//       SpielFatalError(
//           "GITCGObserver: tensor with perfect recall not implemented.");
//     }

//     // Observing player.
//     WriteObservingPlayer(state, player, allocator);

//     // Private hand(s).
//     if (iig_obs_type_.private_info == PrivateInfoType::kSinglePlayer) {
//       WriteSinglePlayerHand(state, player, allocator);
//     } else if (iig_obs_type_.private_info == PrivateInfoType::kAllPlayers) {
//       WriteAllPlayerHands(state, allocator);
//     }

//     // Public information.
//     if (iig_obs_type_.public_info) {
//       WriteCurrentPlayer(state, allocator);
//       WriteKnockCard(state, allocator);
//       WriteUpcard(state, allocator);
//       WriteDiscardPile(state, allocator);
//       WriteStockSize(state, allocator);
//       WriteLayedMelds(state, allocator);
//     }
//   }

//   std::string StringFrom(const State& observed_state,
//                          int player) const override {
//     auto& state = open_spiel::down_cast<const GITCGState&>(observed_state);
//     SPIEL_CHECK_GE(player, 0);
//     SPIEL_CHECK_LT(player, state.num_players_);

//     if (iig_obs_type_.perfect_recall) {
//       if (iig_obs_type_.public_info &&
//           iig_obs_type_.private_info == PrivateInfoType::kSinglePlayer) {
//         return state.aohs_[player].ToString();
//       } else {
//         SpielFatalError(
//             "GITCGObserver: string with perfect recall is implemented only"
//             " for the (default) info state observation type.");
//       }
//     }

//     std::string rv;
//     absl::StrAppend(&rv, "\nKnock card: ", state.knock_card_);
//     absl::StrAppend(&rv, "\nPrev upcard: ",
//                     state.utils_.CardString(state.prev_upcard_));
//     absl::StrAppend(&rv, "\nRepeated move: ", state.repeated_move_);
//     absl::StrAppend(&rv, "\nCurrent player: ", state.cur_player_);
//     absl::StrAppend(&rv, "\nPhase: ",
//                     GITCGState::kPhaseString[static_cast<int>(state.phase_)],
//                     "\n");
//     if (iig_obs_type_.private_info == PrivateInfoType::kAllPlayers ||
//         (iig_obs_type_.private_info == PrivateInfoType::kSinglePlayer &&
//         player == 0)) {
//       absl::StrAppend(&rv, "\nPlayer0: Deadwood=", state.deadwood_[0]);
//     } else {
//       absl::StrAppend(&rv, "\nPlayer0:");
//     }
//     if (state.knocked_[1] && !state.layoffs_.empty()) {
//       absl::StrAppend(&rv, "\nLayoffs: ");
//       for (int card : state.layoffs_) {
//         absl::StrAppend(&rv, state.utils_.CardString(card));
//       }
//     }
//     if (!state.layed_melds_[0].empty()) {
//       absl::StrAppend(&rv, "\nLayed melds:");
//       for (int meld_id : state.layed_melds_[0]) {
//         absl::StrAppend(&rv, " ");
//         std::vector<int> meld = state.utils_.int_to_meld.at(meld_id);
//         for (int card : meld) {
//           absl::StrAppend(&rv, state.utils_.CardString(card));
//         }
//       }
//     }
//     if (iig_obs_type_.private_info == PrivateInfoType::kAllPlayers ||
//         (iig_obs_type_.private_info == PrivateInfoType::kSinglePlayer &&
//         player == 0)) {
//       absl::StrAppend(&rv, "\n", state.utils_.HandToString(state.hands_[0]));
//     } else {
//       absl::StrAppend(&rv, "\n", state.utils_.HandToString(std::vector<int>()));
//     }

//     absl::StrAppend(&rv, "\nStock size: ", state.stock_size_);
//     absl::StrAppend(&rv, "  Upcard: ", state.utils_.CardString(state.upcard_));
//     absl::StrAppend(&rv, "\nDiscard pile: ");
//     for (int card : state.discard_pile_) {
//       absl::StrAppend(&rv, state.utils_.CardString(card));
//     }
//     if (iig_obs_type_.private_info == PrivateInfoType::kAllPlayers ||
//         (iig_obs_type_.private_info == PrivateInfoType::kSinglePlayer &&
//         player == 1)) {
//       absl::StrAppend(&rv, "\n\nPlayer1: Deadwood=", state.deadwood_[1]);
//     } else {
//       absl::StrAppend(&rv, "\n\nPlayer1:");
//     }
//     if (state.knocked_[0] && !state.layoffs_.empty()) {
//       absl::StrAppend(&rv, "\nLayoffs: ");
//       for (int card : state.layoffs_) {
//         absl::StrAppend(&rv, state.utils_.CardString(card));
//       }
//     }
//     if (!state.layed_melds_[1].empty()) {
//       absl::StrAppend(&rv, "\nLayed melds:");
//       for (int meld_id : state.layed_melds_[1]) {
//         absl::StrAppend(&rv, " ");
//         std::vector<int> meld = state.utils_.int_to_meld.at(meld_id);
//         for (int card : meld) {
//           absl::StrAppend(&rv, state.utils_.CardString(card));
//         }
//       }
//     }

//     if (iig_obs_type_.private_info == PrivateInfoType::kAllPlayers ||
//         (iig_obs_type_.private_info == PrivateInfoType::kSinglePlayer &&
//         player == 1)) {
//       absl::StrAppend(&rv, "\n", state.utils_.HandToString(state.hands_[1]));
//     } else {
//       absl::StrAppend(&rv, "\n", state.utils_.HandToString(std::vector<int>()));
//     }
//     return rv;
//   }

//  private:
//   static void WriteObservingPlayer(const GITCGState& state, int player,
//                                    Allocator* allocator) {
//     auto out = allocator->Get("player", {kNumPlayers});
//     out.at(player) = 1;
//   }

//   static void WriteSinglePlayerHand(const GITCGState& state, int player,
//                                     Allocator* allocator) {
//     auto out = allocator->Get("private_hand", {kNumPlayers, kDefaultNumCards});
//     for (auto card : state.hands_[player]) out.at(player, card) = 1;
//   }

//   static void WriteAllPlayerHands(const GITCGState& state,
//                                   Allocator* allocator) {
//     auto out = allocator->Get("private_hands", {kNumPlayers, kDefaultNumCards});
//     for (Player p = 0; p < kNumPlayers; ++p) {
//       for (auto card : state.hands_[p]) out.at(p, card) = 1;
//     }
//   }

//   static void WriteCurrentPlayer(const GITCGState& state,
//                                  Allocator* allocator) {
//     auto out = allocator->Get("current_player", {kNumPlayers});
//     if (state.cur_player_ >= 0) out.at(state.cur_player_) = 1;
//   }

//   static void WriteKnockCard(const GITCGState& state,
//                              Allocator* allocator) {
//     auto out = allocator->Get("knock_card", {kDefaultKnockCard});
//     for (int i = 0; i < state.knock_card_; ++i) out.at(i) = 1;
//   }

//   static void WriteUpcard(const GITCGState& state, Allocator* allocator) {
//     auto out = allocator->Get("upcard", {kDefaultNumCards});
//     if (state.upcard_.has_value()) out.at(state.upcard_.value()) = 1;
//   }

//   static void WriteDiscardPile(const GITCGState& state,
//                                Allocator* allocator) {
//     auto out = allocator->Get("discard_pile", {kDefaultNumCards});
//     for (auto card : state.discard_pile_) out.at(card) = 1;
//   }

//   static void WriteStockSize(const GITCGState& state,
//                              Allocator* allocator) {
//     auto out = allocator->Get("stock_size", {kDefaultNumCards});
//     for (int i = 0; i < state.stock_size_; ++i) out.at(i) = 1;
//   }

//   static void WriteLayedMelds(const GITCGState& state,
//                               Allocator* allocator) {
//     auto out = allocator->Get("layed_melds", {kNumPlayers, kNumMeldActions});
//     for (Player p = 0; p < kNumPlayers; ++p) {
//       for (auto meld : state.layed_melds_[p]) out.at(p, meld) = 1;
//     }
//   }

//   IIGObservationType iig_obs_type_;
// };


GITCGGame::GITCGGame(const GameParameters& params) : Game(kGameType, params) {
  gitcg_initialize();
}

GITCGState::GITCGState(std::shared_ptr<const Game> game, gitcg_state_createparam_t createparam) : State(game) {
  gitcg_thread_initialize();
  absl::c_fill(dealer_deck_, 1);
}


std::string GITCGState::ActionToString(Player player, Action action) const {
  if (player == kChancePlayerId) {
    if (action < kDealingActionBase) {
      return absl::StrCat("Decide first card up position ", action);
    } else if (action < kDealingActionBase + kNumCards) {
      return absl::StrCat("Deal ", CardString(action - kDealingActionBase));
    } else {
      SpielFatalError(
          absl::StrFormat("Non valid ID %d for chance player", action));
    }
  }

  if (action == kPass) {
    return "Pass";
  } else if (action > kPass && action < kPlayActionBase) {
    return absl::StrCat("Bid ", action - kBiddingActionBase);
  } else if (action >= kPlayActionBase && action <= kRocketActionBase) {
    // For aiplane combinations, need special treatment to resolve ambiguity
    if (action >= kAirplaneWithSoloActionBase && action < kBombActionBase) {
      return FormatAirplaneCombHand(action);
    }
    return FormatSingleHand(ActionToHand(action));
  } else {
    SpielFatalError("Non valid action ID!");
  }
}

std::string GITCGState::ToString() const {
  std::string rv = FormatDeal();

  if (history_.size() > kNumCards - kNumCardsLeftOver + 1)
    absl::StrAppend(&rv, FormatAuction());

  if (num_played_ > 0) absl::StrAppend(&rv, FormatPlay());
  if (IsTerminal()) absl::StrAppend(&rv, FormatResult());

  return rv;
}

std::string GITCGState::FormatAuction() const {
  SPIEL_CHECK_GT(history_.size(), kNumCards - kNumCardsLeftOver + 1);
  std::string rv = "Bidding phase begin\n";
  for (int i = kNumCards - kNumCardsLeftOver + 1;
       i < history_.size() - num_played_; ++i) {
    absl::StrAppend(
        &rv, absl::StrFormat(
                 "Player %d played %s\n", history_[i].player,
                 ActionToString(history_[i].player, history_[i].action)));
  }
  return rv;
}

std::string GITCGState::FormatPlay() const {
  SPIEL_CHECK_GT(num_played_, 0);
  std::string rv = "Playing phase begin \n";
  for (int i = history_.size() - num_played_; i < history_.size(); ++i) {
    absl::StrAppend(
        &rv, absl::StrFormat(
                 "Player %d played %s\n", history_[i].player,
                 ActionToString(history_[i].player, history_[i].action)));
  }
  return rv;
}

std::string GITCGState::FormatResult() const {
  std::string rv = "The results are: \n";
  for (int player = 0; player < kNumPlayers; ++player) {
    absl::StrAppend(
        &rv, absl::StrFormat("Player %d got %f\n", player, returns_[player]));
  }
  return rv;
}

std::array<std::string, kNumRanks> FormatHand(
    int player, bool mark_voids,
    const std::array<std::array<int, kNumRanks>, kNumPlayers>& deal) {
  std::array<std::string, kNumRanks> cards{};
  for (int rank = 0; rank < kNumRanks - 2; ++rank) {
    bool is_void = true;
    for (int i = 0; i < deal[player][rank]; ++i) {
      cards[rank].push_back(kRankChar[rank]);
      is_void = false;
    }
    if (is_void && mark_voids) absl::StrAppend(&cards[rank], "none");
  }
  if (deal[player][kNumRanks - 2])
    absl::StrAppend(&cards[kNumRanks - 2], "(BWJ)");
  else if (mark_voids)
    absl::StrAppend(&cards[kNumRanks - 2], "none");

  if (deal[player][kNumRanks - 1])
    absl::StrAppend(&cards[kNumRanks - 1], "(CJ)");
  else if (mark_voids)
    absl::StrAppend(&cards[kNumRanks - 1], "none");

  return cards;
}

std::array<std::array<int, kNumRanks>, kNumPlayers>
GITCGState::OriginalDeal() const {
  SPIEL_CHECK_GE(history_.size(), kNumCards + 1);
  std::array<std::array<int, kNumRanks>, kNumPlayers> deal{};
  for (int i = 1; i < kNumCards - kNumCardsLeftOver + 1; ++i)
    deal[((i - 1 + first_player_) % kNumPlayers)]
        [CardToRank(history_[i].action - kDealingActionBase)]++;

  for (int i = 0; i < kNumCardsLeftOver; ++i)
    deal[dizhu_][cards_left_over_[i]]++;
  return deal;
}

std::string GITCGState::FormatDeal() const {
  std::array<std::array<std::string, kNumRanks>, kNumPlayers> cards{};
  if (IsTerminal()) {
    // Include all cards in the terminal state to make reviewing the deal easier
    auto deal = OriginalDeal();
    for (int player = 0; player < kNumPlayers; ++player) {
      cards[player] = FormatHand(player, /*mark_voids=*/false, deal);
    }
  } else {
    for (int player = 0; player < kNumPlayers; ++player) {
      cards[player] = FormatHand(player, /*mark_voids=*/false, holds_);
    }
  }
  constexpr int kColumnWidth = 8;
  std::string padding(kColumnWidth, ' ');
  std::string rv;
  for (int rank = 0; rank < kNumRanks; ++rank)
    absl::StrAppend(&rv, absl::StrFormat("%-8s", cards[1][rank]), padding,
                    cards[2][rank], "\n");
  for (int rank = 0; rank < kNumRanks; ++rank)
    absl::StrAppend(&rv, padding, cards[0][rank], "\n");
  return rv;
}

std::string GITCGState::ObservationString(Player player) const {
  SPIEL_CHECK_GE(player, 0);
  SPIEL_CHECK_LT(player, num_players_);
  std::string rv =
      absl::StrFormat("My hand %s\n", FormatSingleHand(holds_[player]));
  absl::StrAppend(&rv, absl::StrFormat("Played cards %s\n",
                                       FormatSingleHand(played_deck_)));
  absl::StrAppend(&rv,
                  absl::StrFormat("face up card rank: %d", card_rank_face_up_));
  absl::StrAppend(&rv, absl::StrFormat("start player: %d", first_player_));
  absl::StrAppend(
      &rv, absl::StrFormat("My position from Dizhu: %d",
                           (player - dizhu_ + kNumPlayers) % kNumPlayers));
  return rv;
}

void GITCGState::ObservationTensor(Player player,
                                      absl::Span<float> values) const {
  SPIEL_CHECK_EQ(values.size(), game_->ObservationTensorSize());
  WriteObservationTensor(player, values);
}

void GITCGState::WriteObservationTensor(Player player,
                                           absl::Span<float> values) const {
  SPIEL_CHECK_GE(player, 0);
  SPIEL_CHECK_LT(player, num_players_);

  absl::c_fill(values, 0.);
  if (phase_ == Phase::kDeal) return;
  auto values_iterator = values.begin();
  const int played_deck_base = (kNumRanks - 2) * (kNumSuits + 1) + 2 * 2;
  for (int i = 0; i < kNumRanks; ++i) {
    values_iterator[i * (kNumSuits + 1) + holds_[player][i]] = 1;
    values_iterator[played_deck_base + i * (kNumSuits + 1) + played_deck_[i]] =
        1;
  }

  if (dizhu_ != kInvalidPlayer) {
    const int from_dizhu_base = 2 * played_deck_base;
    const int from_dizhu = (player - dizhu_ + kNumPlayers) % kNumPlayers;
    values_iterator[from_dizhu_base + from_dizhu] = 1;
  }

  if (first_player_ != kInvalidPlayer) {
    const int start_player_base = 2 * played_deck_base + kNumPlayers;
    values_iterator[start_player_base + first_player_] = 1;
    values_iterator[start_player_base + kNumPlayers + card_rank_face_up_] = 1;
  }
}

std::vector<Action> GITCGState::LegalActions() const {
  switch (phase_) {
    case Phase::kDeal:
      return DealLegalActions();
    case Phase::kAuction:
      return BiddingLegalActions();
    case Phase::kPlay:
      return PlayLegalActions();
    default:
      return {};
  }
}

std::vector<Action> GITCGState::DealLegalActions() const {
  std::vector<Action> legal_actions;
  legal_actions.reserve(kNumCards - history_.size() + 1);

  if (card_face_up_position_ == -1) {
    for (int i = 0; i < kDealingActionBase; ++i) legal_actions.push_back(i);
  } else {
    for (int i = 0; i < kNumCards; ++i) {
      if (dealer_deck_[i]) legal_actions.push_back(i + kDealingActionBase);
    }
  }

  return legal_actions;
}

std::vector<Action> GITCGState::BiddingLegalActions() const {
  std::vector<Action> legal_actions = {kPass};
  legal_actions.reserve(kNumBids + 1);

  for (int bid = winning_bid_ + 1; bid <= kNumBids; ++bid) {
    legal_actions.push_back(kBiddingActionBase + bid);
  }
  return legal_actions;
}

std::vector<Action> GITCGState::PlayLegalActions() const {
  std::vector<Action> legal_actions;
  // the leader of a trick must play./ an action and cannot pass
  if (!new_trick_begin_) legal_actions.push_back(kPass);

  std::array<int, kNumRanks> hand = holds_[current_player_];
  const int prev_action = CurrentTrick().WinningAction();
  SearchForLegalActions(&legal_actions, hand, prev_action);

  absl::c_sort(legal_actions);
  return legal_actions;
}

std::vector<std::pair<Action, double>> GITCGState::ChanceOutcomes() const {
  std::vector<std::pair<Action, double>> outcomes;
  int num_cards_remaining = 0;
  for (int i = 0; i < kNumCards; ++i) num_cards_remaining += dealer_deck_[i];
  outcomes.reserve(num_cards_remaining);

  if (card_face_up_position_ == -1) {
    for (int i = 0; i < kDealingActionBase; ++i)
      outcomes.emplace_back(i, 1.0 / static_cast<double>(kDealingActionBase));
  } else {
    for (int card = 0; card < kNumCards; ++card)
      if (dealer_deck_[card])
        outcomes.emplace_back(card + kDealingActionBase,
                              1.0 / static_cast<double>(num_cards_remaining));
  }

  return outcomes;
}

void GITCGState::DoApplyAction(Action action) {
  switch (phase_) {
    case Phase::kDeal:
      return ApplyDealAction(action);
    case Phase::kAuction:
      return ApplyBiddingAction(action);
    case Phase::kPlay:
      return ApplyPlayAction(action);
    case Phase::kGameOver:
      SpielFatalError("Cannot act in terminal states");
  }
}

void GITCGState::ApplyDealAction(int action) {
  // First decide the face up card
  if (card_face_up_position_ == -1) {
    card_face_up_position_ = action;
    return;
  }

  const int dealing_round = static_cast<int>(history_.size()) - 1;
  // if the current player is dealt the face up card, make it the first one to
  // bid
  if (dealing_round == history_[0].action) {
    first_player_ = dealing_round % kNumPlayers;
    card_rank_face_up_ = CardToRank(action - kDealingActionBase);
  }
  const int dealt_player_idx = ((history_.size() - 1) % kNumPlayers);
  const int dealt_rank = CardToRank(action - kDealingActionBase);
  holds_[dealt_player_idx][dealt_rank]++;
  dealer_deck_[action - kDealingActionBase]--;
  if (history_.size() == kNumCards - kNumCardsLeftOver) {
    phase_ = Phase::kAuction;
    current_player_ = first_player_;
    SPIEL_CHECK_GE(current_player_, 0);
    SPIEL_CHECK_LE(current_player_, num_players_);
    for (int card = 0; card < kNumCards; ++card)
      if (dealer_deck_[card]) {
        cards_left_over_.push_back(CardToRank(card));
      }
  }
}

void GITCGState::ApplyBiddingAction(int action) {
  // Track the number of consecutive passes since the last bid (if any).
  if (action == kPass) {
    ++num_passes_;
  } else {
    num_passes_ = 0;
  }

  bool has_winner = false;

  if (action == kPass) {
    if (num_passes_ == kNumPlayers)
      phase_ = Phase::kGameOver;
    else if (num_passes_ == kNumPlayers - 1 && winning_bid_ > 0)
      has_winner = true;
  } else {
    dizhu_ = current_player_;
    winning_bid_ = action - kBiddingActionBase;
    if (winning_bid_ == kNumBids) has_winner = true;
  }
  if (has_winner) {
    for (int i = 0; i < kNumCardsLeftOver; ++i)
      holds_[dizhu_][cards_left_over_[i]]++;
    phase_ = Phase::kPlay;
    current_player_ = dizhu_;
    new_trick_begin_ = true;
    tricks_.push_back(Trick(dizhu_, kInvalidAction));
    num_passes_ = 0;
  } else {
    current_player_ = (current_player_ + 1) % kNumPlayers;
  }
}

bool GITCGState::AfterPlayHand(int player, int action) {
  std::array<int, kNumRanks> used_hand = ActionToHand(action);
  bool flag = true;
  for (int rank = 0; rank < kNumRanks; ++rank) {
    SPIEL_CHECK_GE(holds_[player][rank], used_hand[rank]);
    holds_[player][rank] -= used_hand[rank];
    flag &= !holds_[player][rank];
    played_deck_[rank] += used_hand[rank];
  }
  return flag;
}

void GITCGState::ApplyPlayAction(int action) {
  num_played_++;

  if (action == kPass) {
    ++num_passes_;
  } else {
    num_passes_ = 0;
  }

  if (action == kPass) {
    if (num_passes_ == kNumPlayers - 1) {
      current_player_ = CurrentTrick().Winner();
      trick_played_++;
      num_passes_ = 0;
      tricks_.push_back(Trick());
      new_trick_begin_ = true;
      return;
    }
  } else {
    if (action >= kBombActionBase) bombs_played_++;
    players_hands_played[current_player_]++;

    if (new_trick_begin_) new_trick_begin_ = false;

    CurrentTrick().Play(current_player_, action);

    bool all_played = AfterPlayHand(current_player_, action);
    if (all_played) {
      final_winner_ = current_player_;
      ScoreUp();
      phase_ = Phase::kGameOver;
      return;
    }
  }
  current_player_ = (current_player_ + 1) % kNumPlayers;
}

Player GITCGState::CurrentPlayer() const {
  if (phase_ == Phase::kDeal) {
    return kChancePlayerId;
  } else if (phase_ == Phase::kGameOver) {
    return kTerminalPlayerId;
  } else {
    return current_player_;
  }
}

void GITCGState::ScoreUp() {
  // If no one bids, 0 for everyone
  if (dizhu_ == kInvalidPlayer) return;

  // if none of the farmers played, or the dizhu only played once
  // then it is spring!
  bool is_spring = false;
  is_spring |= (players_hands_played[dizhu_] == 1);
  is_spring |= ((!players_hands_played[(dizhu_ + 1) % 3]) &&
                (!players_hands_played[(dizhu_ + 2) % 3]));

  int paying = winning_bid_;
  for (int i = 0; i < is_spring + bombs_played_; ++i) paying *= 2;
  const int dizhu_sign = (final_winner_ == dizhu_) ? 1 : -1;

  returns_[dizhu_] = dizhu_sign * 2 * paying;
  returns_[(dizhu_ + 1) % 3] = -dizhu_sign * paying;
  returns_[(dizhu_ + 2) % 3] = -dizhu_sign * paying;
}

Trick::Trick(Player leader, int action)
    : winning_action_(action), leader_(leader), winning_player_(leader) {}

}  // namespace gi_tcg
}  // namespace open_spiel
