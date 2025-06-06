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

#include "open_spiel/spiel.h"
#include "open_spiel/tests/basic_tests.h"

namespace open_spiel {
namespace gi_tcg {
namespace {

void BasicGameTests() {
  testing::LoadGameTest("gi_tcg");
  testing::RandomSimTest(*LoadGame("gi_tcg"), 20);
}

}  // namespace
}  // namespace gi_tcg
}  // namespace open_spiel

int main() {
  open_spiel::gi_tcg::BasicGameTests();
}
