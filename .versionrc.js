module.exports = {
    types: [
      { type: 'fix', section: '🐛 Bug Fixes', hidden: false },
      { type: 'feat', section: '✨ Features', hidden: false },
  
      // 以下类型设置 hidden: true，表示不会出现在 changelog 中，且不会影响版本更新
      { type: 'docs', section: '📝 Documentation', hidden: true },
      { type: 'style', section: '💄 Styles', hidden: true },
      { type: 'refactor', section: '♻️ Code Refactoring', hidden: true },
      { type: 'perf', section: '⚡️ Performance Improvements', hidden: true },
      { type: 'test', section: '✅ Tests', hidden: true },
      { type: 'ci', section: '👷 CI/CD', hidden: true },
      { type: 'chore', section: '🔧 Chores', hidden: true },
      { type: 'revert', section: '⏪️ Reverts', hidden: true }
    ]
  };
  