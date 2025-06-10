# 🤝 项目贡献指南（CONTRIBUTING）

欢迎参与本项目的开发！为了保持代码库整洁、可维护，请遵守以下贡献规范。

---

## 1️⃣ 分支管理规范（Branch Naming）

所有开发都应基于 `dev` 分支，禁止直接在 `main` 分支开发。

### 分支命名格式：

```

<类型>/<模块名>-<简要描述>

```

### 分支类型说明：

| 类型       | 说明                 | 示例                     |
| ---------- | -------------------- | ------------------------ |
| `feature`  | 新功能开发           | `feature/auth-login`     |
| `fix`      | Bug 修复             | `fix/user-pagination`    |
| `hotfix`   | 紧急修复生产环境问题 | `hotfix/crash-login`     |
| `release`  | 发布准备，打版本 tag | `release/v1.3.0`         |
| `chore`    | 构建脚本、CI、文档等 | `chore/ci-github-action` |
| `refactor` | 重构代码，无新功能   | `refactor/user-service`  |
| `test`     | 添加或修改测试代码   | `test/api-endpoints`     |

---

## 2️⃣ Commit 规范（Conventional Commit）

请使用 [Conventional Commits](https://www.conventionalcommits.org/) 规范化提交信息。建议配置本地提交模板 `.gitmessage`。

### 格式：

```

<type>(<scope>): <subject>

```

### 提交类型说明：

| 类型       | 含义               |
| ---------- | ------------------ |
| `feat`     | 新功能             |
| `fix`      | 修复 Bug           |
| `docs`     | 修改文档           |
| `style`    | 代码格式（无逻辑） |
| `refactor` | 重构代码           |
| `test`     | 测试               |
| `chore`    | 构建工具/脚手架    |
| `perf`     | 性能优化           |
| `revert`   | 回滚提交           |

### 示例：

```text
feat(auth): 添加注册接口
fix(api): 修复分页返回数据错误
docs(readme): 增加开发启动说明
```

### 本地配置：

```bash
git config commit.template .gitmessage
```

---

## 3️⃣ Pull Request 规范（PR）

* 所有 PR **必须基于 dev 分支**。
* 每个 PR 请关联 Issue 编号（如有）并附加变更说明。
* PR 标题格式建议：

```
[type] 模块简述 (#issue编号)
```

#### 示例：

```text
[feat] 用户注册功能 (#23)
[fix] 登录接口失败 (#42)
```

* PR 审核通过需至少一位 Reviewer Review；
* Reviewer 可指派：`@dev-lead` 或对应模块维护人；

---

## 4️⃣ Issue 管理规范

### Issue 标题格式：

```
[类型] 简要描述
```

### 类型标签推荐：

* `bug`
* `enhancement`
* `documentation`
* `discussion`
* `question`
* `wontfix`

### 描述模板：

```markdown
## 描述问题
（简要描述当前问题）

## 重现步骤
1. 打开页面 xxx
2. 点击按钮 xxx

## 预期结果
应展示 xxx

## 实际结果
出现错误 xxx

## 环境信息
系统 / 浏览器 / 版本
```

---

## 5️⃣ CHANGELOG 自动生成

项目使用 [`git-cliff`](https://github.com/orhun/git-cliff) 自动生成 CHANGELOG，按提交信息分类整理。

* 请严格遵循 Commit 格式，才能正确生成日志；
* 每次合并 PR 前建议运行：

```bash
git cliff -o CHANGELOG.md
```

---

感谢您的贡献！🚀