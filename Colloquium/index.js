/**
 * Экспорт всех модулей из библиотеки
 */
const FactorialGenerator = require('./src/factorial');
const ArrayProcessor = require('./src/unique-array');
const { LinkedList, ListNode } = require('./src/linked-list');

module.exports = {
  FactorialGenerator,
  ArrayProcessor,
  LinkedList,
  ListNode,
};
