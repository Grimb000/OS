const ArrayProcessor = require('../../src/unique-array');

describe('ArrayProcessor', () => {
  describe('removeDuplicates', () => {
    it('должен возвращать пустой массив для пустого входного массива', () => {
      expect(ArrayProcessor.removeDuplicates([])).toEqual([]);
    });

    it('должен возвращать массив без изменений, если в нем нет дубликатов', () => {
      const input = [1, 2, 3, 4, 5];
      const result = ArrayProcessor.removeDuplicates(input);
      expect(result).toEqual([1, 2, 3, 4, 5]);
      // Проверка, что возвращается новый массив, а не тот же самый
      expect(result).not.toBe(input);
    });

    it('должен удалять дубликаты примитивных значений с сохранением порядка', () => {
      expect(ArrayProcessor.removeDuplicates([1, 2, 2, 3, 4, 1, 5])).toEqual([1, 2, 3, 4, 5]);
      expect(ArrayProcessor.removeDuplicates([5, 5, 5, 5, 5])).toEqual([5]);
      expect(ArrayProcessor.removeDuplicates(['a', 'b', 'a', 'c', 'b'])).toEqual(['a', 'b', 'c']);
    });

    it('должен правильно обрабатывать массивы с разными типами данных', () => {
      expect(
        ArrayProcessor.removeDuplicates([1, '1', true, 1, true, '1']),
      ).toEqual([1, '1', true]);

      expect(
        ArrayProcessor.removeDuplicates([null, undefined, false, 0, '', null, undefined]),
      ).toEqual([null, undefined, false, 0, '']);
    });

    it('должен правильно обрабатывать объекты', () => {
      const obj1 = { a: 1 };
      const obj2 = { a: 1 }; // Одинаковое содержимое с obj1
      const obj3 = { b: 2 };

      const input = [obj1, obj2, obj3, { a: 1 }];
      const result = ArrayProcessor.removeDuplicates(input);

      expect(result).toHaveLength(2);
      expect(result[0]).toEqual({ a: 1 });
      expect(result[1]).toEqual({ b: 2 });
    });

    it('должен правильно обрабатывать массивы массивов', () => {
      expect(
        ArrayProcessor.removeDuplicates([[1, 2], [3, 4], [1, 2], [5, 6]]),
      ).toEqual([[1, 2], [3, 4], [5, 6]]);
    });

    it('должен правильно обрабатывать даты', () => {
      const date1 = new Date('2023-01-01');
      const date2 = new Date('2023-01-01'); // Та же дата
      const date3 = new Date('2023-02-01');

      const result = ArrayProcessor.removeDuplicates([date1, date2, date3]);
      expect(result).toHaveLength(2);
      expect(result[0]).toEqual(date1);
      expect(result[1]).toEqual(date3);
    });

    it('должен выбрасывать ошибку для неверных входных данных', () => {
      expect(() => {
        ArrayProcessor.removeDuplicates('не массив');
      }).toThrow(TypeError);

      expect(() => {
        ArrayProcessor.removeDuplicates(123);
      }).toThrow(TypeError);

      expect(() => {
        ArrayProcessor.removeDuplicates(null);
      }).toThrow(TypeError);

      expect(() => {
        ArrayProcessor.removeDuplicates(undefined);
      }).toThrow(TypeError);
    });
  });
});
