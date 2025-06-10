module.exports = {
  branches: ['main'],
  plugins: [
    ['@semantic-release/commit-analyzer', {
      preset: 'conventionalcommits',
      releaseRules: [
        { type: 'feat', release: 'minor' },
        { type: 'fix', release: 'patch' },
        { type: 'chore', release: 'patch' }, // 添加这一行以支持chore触发发布
      ]
    }],
    '@semantic-release/release-notes-generator', // 自动生成 changelog
    '@semantic-release/github' // 发布 GitHub Release
  ],
  preset: 'conventionalcommits',
};
