| 类名 (Class Name)              | 主要用途 (Primary Use Case)                                                                 | 文件 (File)                         |
| :----------------------------- | :------------------------------------------------------------------------------------------ | :---------------------------------- |
| `Game`                         | 表示任何博弈的核心接口，定义博弈的规则和属性。                                                       | `spiel.h`                           |
| `State`                        | 表示博弈中特定状态的核心接口，提供有关当前玩家、合法行动、历史等信息。                                           | `spiel.h`                           |
| `ActionView`                   | 处理同时行动博弈中的行动，提供迭代器和实用工具。                                                               | `action_view.h`                     |
| `GameParameter`                | 定义和管理博弈的特定参数，允许对博弈进行不同配置。                                                               | `game_parameters.h`                 |
| `MatrixGame`                   | 表示双人范式博弈（也称为矩阵博弈）。                                                                   | `matrix_game.h`                     |
| `NormalFormGame`               | 作为所有玩家同时行动的博弈（一次性博弈）的基类。                                                               | `normal_form_game.h`                |
| `Observer`                     | 从博弈状态获取观察结果，例如信息状态字符串或张量表示。                                                               | `observer.h`                        |
| `Policy`                       | 表示和操作博弈中的策略，即从状态到行动（或行动概率分布）的映射。                                                           | `policy.h`                          |
| `SimMoveGame` / `SimMoveState` | 作为玩家可以同时进行移动的博弈的基类。                                                                   | `simultaneous_move_game.h`          |
| `Bot`                          | 定义可以玩博弈的智能体（agent）的接口。                                                                  | `spiel_bots.h`                      |
| `TensorGame`                   | 表示 n 人范式博弈，其中玩家的效用（utility）由张量定义。                                                              | `tensor_game.h`                     |

**注意:**

*   `spiel_globals.h` 主要包含全局常量和枚举（如 `PlayerId`），在编写博弈逻辑时会经常用到。
*   `spiel_utils.h` 包含各种通用辅助函数，在实现博弈或算法时非常有用。
*   `canonical_game_strings.h` 提供用于生成特定博弈（如扑克）的规范字符串的函数，这些字符串可用于加载博弈。
