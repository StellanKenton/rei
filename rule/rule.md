---
doc_role: repo-rule
layer: rule
module: repo
status: active
portability: project-bound
core_files:
    - rule.md
    - map.md
    - projectrule.md
    - coderule.md
    - memory.md
read_next:
    - map.md
    - projectrule.md
    - coderule.md
---

# venthmi 仓库入口规则

这是当前仓库的权威入口文档。`rei/rule/` 下的规则必须优先于其他零散说明，并且必须与仓库真实结构保持一致。

## 1. 适用范围

本仓库当前是一个基于 CMake 的 C++20 工程，已存在的核心路径只有：

- `CMakeLists.txt`
- `src/main.cpp`
- `rei/rule/*.md`
- `build/` 生成目录

如果后续目录结构发生变化，应先更新 `rei/rule/`，再去更新其他说明文档。

## 2. 固定阅读顺序

进入仓库后按下面顺序理解上下文：

1. 先读 `rule.md`，确认规则边界和文档权威关系。
2. 再读 `map.md`，确认目录职责、编辑入口和禁止修改区域。
3. 再读 `projectrule.md`，确认项目级技术约束与 CMake 约束。
4. 最后读 `coderule.md`，执行具体的 C++20 编码与文档风格。

`memory.md` 不负责下指令，只负责说明哪些事实值得长期沉淀。

## 3. 文档权威关系

- `rule.md`：仓库规则入口，定义阅读顺序和文档层级。
- `map.md`：目录地图、任务入口、修改落点。
- `projectrule.md`：项目结构、构建规则、边界约束。
- `coderule.md`：C++20 代码风格、命名、实现细则。
- `memory.md`：可沉淀事实的范围与格式。

出现冲突时，以更靠近真实工程结构、且层级更高的规则为准；同层规则冲突时，以最近一次按仓库现状修订的内容为准。

## 4. 修改规则文档的原则

- 不保留与当前仓库无关的模板内容。
- 不引用不存在的目录、文件或分层概念。
- 不把生成目录 `build/` 写成源码目录。
- 不把未来可能存在的模块当成既有事实写入规则。
- 规则文档要能直接指导当前仓库，而不是泛泛而谈。

## 5. 文档更新顺序

当工程结构或约束变化时，按下面顺序更新：

1. 先更新 `rule.md` 和 `map.md`。
2. 再更新 `projectrule.md`。
3. 再更新 `coderule.md`。
4. 最后补充 `memory.md` 中值得沉淀的稳定事实。

## 6. 当前仓库的核心结论

- 语言标准以 C++20 为准。
- 构建系统以 CMake 为准。
- `src/` 是源码目录，`build/` 是生成目录。
- 规则文档使用中文；代码标识符保持英文。
