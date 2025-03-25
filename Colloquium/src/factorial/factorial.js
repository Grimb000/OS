/**
 * Класс для работы с факториалами
 */
class FactorialGenerator {
  /**
     * Вычисляет значение факториала числа
     * @param {number} num - Натуральное число, для которого нужно вычислить факториал
     * @returns {bigint} Значение факториала в формате bigint
     * @throws {Error} Если аргумент не является натуральным числом
     */
  static calculateFactorial(num) {
    // Валидация входных данных
    if (!Number.isInteger(num) || num < 0) {
      throw new Error('Факториал можно вычислить только для неотрицательных целых чисел');
    }

    // Частные случаи
    if (num === 0 || num === 1) {
      return 1n;
    }

    // Вычисление факториала с использованием BigInt для больших чисел
    let result = 1n;
    for (let i = 2; i <= num; i++) {
      result *= BigInt(i);
    }
    return result;
  }

  /**
     * Генерирует массив из первых n факториалов
     * @param {number} n - Количество факториалов для генерации
     * @returns {bigint[]} Массив первых n факториалов
     * @throws {Error} Если n не является натуральным числом или превышает максимально допустимое значение
     */
  static generateFactorials(n) {
    // Валидация входных данных
    if (!Number.isInteger(n)) {
      throw new TypeError('Аргумент должен быть целым числом');
    }

    if (n < 0) {
      throw new RangeError('Количество факториалов должно быть неотрицательным числом');
    }

    // Ограничение на максимальное значение n для предотвращения переполнения памяти
    const MAX_FACTORIALS = 1000;
    if (n > MAX_FACTORIALS) {
      throw new RangeError(`Количество факториалов не должно превышать ${MAX_FACTORIALS}`);
    }

    // Генерация массива факториалов
    const factorials = [];
    for (let i = 0; i < n; i++) {
      factorials.push(this.calculateFactorial(i));
    }

    return factorials;
  }
}

module.exports = FactorialGenerator;
