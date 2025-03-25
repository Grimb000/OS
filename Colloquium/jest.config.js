module.exports = {
  // Каталоги, в которых будут искаться тестовые файлы
  testMatch: ['**/tests/**/*.test.js'],

  // Окружение, в котором будут запускаться тесты
  testEnvironment: 'node',

  // Добавление покрытия кода
  collectCoverageFrom: [
    'src/**/*.js',
    '!src/**/index.js',
  ],

  // Минимальное покрытие кода
  coverageThreshold: {
    global: {
      branches: 80,
      functions: 80,
      lines: 80,
      statements: 80,
    },
  },

  // Вывод информации о покрытии в консоль
  coverageReporters: ['text', 'lcov', 'clover'],

  // Директория для хранения файлов кэша Jest
  cacheDirectory: '.jest-cache',

  // Показывать предупреждения о необработанных промисах
  testTimeout: 30000,

  // Параллельное выполнение тестов
  maxWorkers: '50%',
};
