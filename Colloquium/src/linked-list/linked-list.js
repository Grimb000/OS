/**
 * Класс, представляющий узел односвязного списка
 */
class ListNode {
  /**
     * Создает новый узел связного списка
     * @param {*} value - Значение, хранящееся в узле
     * @param {ListNode|null} next - Ссылка на следующий узел (по умолчанию null)
     */
  constructor(value, next = null) {
    this.value = value;
    this.next = next;
  }
}

/**
   * Класс для работы со связными списками
   */
class LinkedList {
  /**
     * Создает новый связный список
     */
  constructor() {
    this.head = null;
    this.size = 0;
  }

  /**
     * Добавляет новый узел в конец списка
     * @param {*} value - Значение для добавления в список
     * @returns {LinkedList} Текущий список для поддержки цепочки вызовов
     */
  append(value) {
    const newNode = new ListNode(value);
    this.size++;

    if (!this.head) {
      this.head = newNode;
      return this;
    }

    let current = this.head;
    while (current.next) {
      current = current.next;
    }
    current.next = newNode;
    return this;
  }

  /**
     * Создает связный список из массива значений
     * @param {Array} values - Массив значений для создания списка
     * @returns {LinkedList} Новый связный список
     */
  static fromArray(values) {
    if (!Array.isArray(values)) {
      throw new TypeError('Входной параметр должен быть массивом');
    }

    const list = new LinkedList();
    for (const value of values) {
      list.append(value);
    }
    return list;
  }

  /**
     * Преобразует связный список в массив значений
     * @returns {Array} Массив значений из списка
     */
  toArray() {
    const result = [];
    let current = this.head;

    while (current) {
      result.push(current.value);
      current = current.next;
    }

    return result;
  }

  /**
     * Разворачивает список с использованием рекурсии
     * @returns {LinkedList} Развернутый список
     * @throws {Error} Если возникла ошибка при развороте списка
     */
  reverse() {
    try {
      // Пустой список или список с одним элементом не требует разворота
      if (!this.head || !this.head.next) {
        return this;
      }

      // Вызываем рекурсивную функцию для разворота списка
      this.head = this._reverseRecursive(this.head);
      return this;
    } catch (error) {
      throw new Error(`Ошибка при развороте списка: ${error.message}`);
    }
  }

  /**
     * Рекурсивная функция для разворота списка
     * @param {ListNode} node - Текущий узел
     * @param {ListNode|null} prev - Предыдущий узел
     * @returns {ListNode} Новая голова списка
     * @private
     */
  _reverseRecursive(node, prev = null) {
    // Базовый случай: если достигли конца списка, возвращаем предыдущий узел
    // (он станет новой головой развернутого списка)
    if (!node) {
      return prev;
    }

    // Сохраняем ссылку на следующий узел
    const { next } = node;

    // Меняем указатель текущего узла на предыдущий узел
    node.next = prev;

    // Рекурсивно вызываем функцию для следующего узла,
    // при этом текущий узел становится предыдущим
    return this._reverseRecursive(next, node);
  }

  /**
     * Очищает список
     * @returns {LinkedList} Текущий пустой список
     */
  clear() {
    this.head = null;
    this.size = 0;
    return this;
  }

  /**
     * Возвращает количество элементов в списке
     * @returns {number} Количество элементов
     */
  getSize() {
    return this.size;
  }

  /**
     * Проверяет, пуст ли список
     * @returns {boolean} true, если список пуст, иначе false
     */
  isEmpty() {
    return this.size === 0;
  }
}

module.exports = {
  ListNode,
  LinkedList,
};
