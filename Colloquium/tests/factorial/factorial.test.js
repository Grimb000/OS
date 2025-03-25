const FactorialGenerator = require('../../src/factorial');

describe('FactorialGenerator', () => {
  describe('calculateFactorial', () => {
    it('должен правильно вычислять факториал для неотрицательных целых чисел', () => {
      expect(FactorialGenerator.calculateFactorial(0).toString()).toBe('1');
      expect(FactorialGenerator.calculateFactorial(1).toString()).toBe('1');
      expect(FactorialGenerator.calculateFactorial(2).toString()).toBe('2');
      expect(FactorialGenerator.calculateFactorial(3).toString()).toBe('6');
      expect(FactorialGenerator.calculateFactorial(5).toString()).toBe('120');
      expect(FactorialGenerator.calculateFactorial(10).toString()).toBe('3628800');
      expect(FactorialGenerator.calculateFactorial(20).toString()).toBe('2432902008176640000');
    });

    it('должен выбрасывать ошибку для отрицательных чисел', () => {
      expect(() => {
        FactorialGenerator.calculateFactorial(-1);
      }).toThrow('Факториал можно вычислить только для неотрицательных целых чисел');
    });

    it('должен выбрасывать ошибку для нецелых чисел', () => {
      expect(() => {
        FactorialGenerator.calculateFactorial(3.5);
      }).toThrow('Факториал можно вычислить только для неотрицательных целых чисел');
    });
  });

  describe('generateFactorials', () => {
    it('должен возвращать пустой массив для n = 0', () => {
      const result = FactorialGenerator.generateFactorials(0);
      expect(result).toHaveLength(0);
      expect(result).toEqual([]);
    });

    it('должен правильно генерировать массив первых n факториалов', () => {
      const result1 = FactorialGenerator.generateFactorials(1);
      expect(result1).toHaveLength(1);
      expect(result1.map((n) => n.toString())).toEqual(['1']);

      const result5 = FactorialGenerator.generateFactorials(5);
      expect(result5).toHaveLength(5);
      expect(result5.map((n) => n.toString())).toEqual(['1', '1', '2', '6', '24']);

      const result6 = FactorialGenerator.generateFactorials(6);
      expect(result6).toHaveLength(6);
      expect(result6.map((n) => n.toString())).toEqual(['1', '1', '2', '6', '24', '120']);
    });

    it('должен выбрасывать ошибку для отрицательного n', () => {
      expect(() => {
        FactorialGenerator.generateFactorials(-3);
      }).toThrow('Количество факториалов должно быть неотрицательным числом');
    });

    it('должен выбрасывать ошибку для нецелого n', () => {
      expect(() => {
        FactorialGenerator.generateFactorials(4.7);
      }).toThrow('Аргумент должен быть целым числом');
    });

    it('должен выбрасывать ошибку для n, превышающего максимально допустимое значение', () => {
      const MAX_FACTORIALS = 1000;
      expect(() => {
        FactorialGenerator.generateFactorials(MAX_FACTORIALS + 1);
      }).toThrow(new RegExp(`Количество факториалов не должно превышать ${MAX_FACTORIALS}`));
    });
  });
});
