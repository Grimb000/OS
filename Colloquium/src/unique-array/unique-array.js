/**
 * Класс для работы с массивами и удаления дубликатов
 */
class ArrayProcessor {
  /**
   * Удаляет дубликаты из массива с сохранением порядка элементов
   * @param {Array} array - Исходный массив
   * @returns {Array} Новый массив без дубликатов с сохранением порядка элементов
   * @throws {TypeError} Если входной параметр не является массивом
   */
  static removeDuplicates(array) {
    // Проверка входных данных
    if (!Array.isArray(array)) {
      throw new TypeError('Входной параметр должен быть массивом');
    }

    // Для пустого массива сразу возвращаем пустой массив
    if (array.length === 0) {
      return [];
    }

    try {
      // Используем Set для эффективного удаления дубликатов
      // и сохраняем порядок элементов с помощью цикла
      const uniqueSet = new Set();
      const result = [];

      for (const item of array) {
        // Проверяем, был ли элемент уже добавлен
        if (!uniqueSet.has(this.getValueIdentifier(item))) {
          uniqueSet.add(this.getValueIdentifier(item));
          result.push(item); // Добавляем оригинальный элемент, чтобы сохранить ссылку
        }
      }

      return result;
    } catch (error) {
      throw new Error(`Ошибка при удалении дубликатов: ${error.message}`);
    }
  }

  /**
   * Получает строковый идентификатор значения для корректного сравнения объектов
   * @param {*} value - Значение для идентификации
   * @returns {string} Строковый идентификатор значения
   * @private
   */
  static getValueIdentifier(value) {
    // Для примитивных типов и null/undefined используем тип + значение
    if (value === null || value === undefined || typeof value !== 'object') {
      return `${typeof value}:${String(value)}`;
    }

    // Для объектов используем JSON.stringify для сравнения по содержимому
    // Пытаемся сделать глубокое сравнение, но учитываем циклические ссылки
    try {
      // Для Date возвращаем timestamp
      if (value instanceof Date) {
        return `date:${value.getTime()}`;
      }

      // Для других объектов используем JSON сериализацию
      return JSON.stringify(value);
    } catch (error) {
      // В случае ошибки (например, циклические ссылки)
      // используем уникальный идентификатор объекта
      return `obj:${Object.keys(value).sort().join(',')}`;
    }
  }
}

module.exports = ArrayProcessor;
