module.exports = {
  env: {
    node: true,
    commonjs: true,
    es2021: true,
    jest: true,
  },
  extends: [
    'airbnb-base',
    'plugin:jest/recommended',
  ],
  plugins: [
    'jest',
  ],
  parserOptions: {
    ecmaVersion: 'latest',
  },
  rules: {
    // Разрешаем консоль в примерах, но запрещаем в основном коде
    'no-console': ['error', { allow: ['warn', 'error'] }],
    'no-console': 'off',

    // Разрешаем классы без конструкторов
    'class-methods-use-this': 'off',

    // Стили кодирования
    'comma-dangle': ['error', 'always-multiline'],
    'max-len': ['error', { code: 100, ignoreComments: true }],

    // Правила для Jest
    'jest/no-disabled-tests': 'warn',
    'jest/no-focused-tests': 'error',
    'jest/no-identical-title': 'error',
    'jest/prefer-to-have-length': 'warn',
    'jest/valid-expect': 'error',
  },
  overrides: [
    {
      // Отключаем правило no-console для примеров
      files: ['**/tests/**/*.examples.js'],
      rules: {
        'no-console': 'off',
      },
    },
  ],
};
