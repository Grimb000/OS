/**
 * Примеры использования функции удаления дубликатов из массива
 */
const ArrayProcessor = require('../../src/unique-array');

console.log('=== Примеры удаления дубликатов из массива ===');

// Пример 1: Удаление дубликатов из массива чисел
console.log('\nПример 1: Удаление дубликатов из массива чисел');
try {
  const originalArray = [1, 2, 3, 2, 4, 1, 5, 6, 5, 7];
  console.log('Исходный массив:', originalArray);

  const uniqueArray = ArrayProcessor.removeDuplicates(originalArray);
  console.log('Массив без дубликатов:', uniqueArray);

  // Проверка, что исходный массив не изменен
  console.log('Исходный массив остался без изменений:', originalArray);
} catch (error) {
  console.error('Произошла ошибка:', error.message);
}

// Пример 2: Удаление дубликатов из массива строк
console.log('\nПример 2: Удаление дубликатов из массива строк');
try {
  const stringArray = ['apple', 'banana', 'apple', 'orange', 'banana', 'grape'];
  console.log('Исходный массив строк:', stringArray);

  const uniqueStrings = ArrayProcessor.removeDuplicates(stringArray);
  console.log('Массив строк без дубликатов:', uniqueStrings);
} catch (error) {
  console.error('Произошла ошибка:', error.message);
}

// Пример 3: Удаление дубликатов из массива объектов
console.log('\nПример 3: Удаление дубликатов из массива объектов');
try {
  const objectsArray = [
    { id: 1, name: 'Item 1' },
    { id: 2, name: 'Item 2' },
    { id: 1, name: 'Item 1' }, // Дубликат
    { id: 3, name: 'Item 3' },
    { id: 2, name: 'Item 2' }, // Дубликат
  ];

  console.log('Исходный массив объектов:');
  objectsArray.forEach((obj) => console.log(obj));

  const uniqueObjects = ArrayProcessor.removeDuplicates(objectsArray);
  console.log('\nМассив объектов без дубликатов:');
  uniqueObjects.forEach((obj) => console.log(obj));
} catch (error) {
  console.error('Произошла ошибка:', error.message);
}

// Пример 4: Удаление дубликатов из массива с разными типами данных
console.log('\nПример 4: Удаление дубликатов из массива с разными типами данных');
try {
  const mixedArray = [1, '1', true, null, 1, true, '1', null, undefined, 0, false, undefined];
  console.log('Исходный смешанный массив:', mixedArray);

  const uniqueMixed = ArrayProcessor.removeDuplicates(mixedArray);
  console.log('Смешанный массив без дубликатов:', uniqueMixed);
} catch (error) {
  console.error('Произошла ошибка:', error.message);
}

// Пример 5: Обработка исключительных ситуаций
console.log('\nПример 5: Обработка исключительных ситуаций');

// Передача не массива
try {
  console.log('Попытка удалить дубликаты из строки:');
  ArrayProcessor.removeDuplicates('not an array');
} catch (error) {
  console.error('Произошла ошибка:', error.message);
}

// Обработка пустого массива
try {
  console.log('\nОбработка пустого массива:');
  const emptyResult = ArrayProcessor.removeDuplicates([]);
  console.log('Результат для пустого массива:', emptyResult);
} catch (error) {
  console.error('Произошла ошибка:', error.message);
}
