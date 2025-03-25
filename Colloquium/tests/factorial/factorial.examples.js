/**
 * Примеры использования генератора факториалов
 */
const FactorialGenerator = require('../../src/factorial');

console.log('=== Примеры работы с факториалами ===');

// Пример 1: Вычисление факториала числа
console.log('\nПример 1: Вычисление факториала числа');
try {
  console.log(`0! = ${FactorialGenerator.calculateFactorial(0)}`);
  console.log(`1! = ${FactorialGenerator.calculateFactorial(1)}`);
  console.log(`5! = ${FactorialGenerator.calculateFactorial(5)}`);
  console.log(`10! = ${FactorialGenerator.calculateFactorial(10)}`);

  // Большое число
  const bigFactorial = FactorialGenerator.calculateFactorial(30);
  console.log(`30! = ${bigFactorial}`);
} catch (error) {
  console.error('Произошла ошибка:', error.message);
}

// Пример 2: Генерация массива факториалов
console.log('\nПример 2: Генерация массива факториалов');
try {
  const n = 10;
  console.log(`Генерация первых ${n} факториалов:`);

  const factorials = FactorialGenerator.generateFactorials(n);

  // Вывод результатов
  for (let i = 0; i < factorials.length; i++) {
    console.log(`${i}! = ${factorials[i].toString()}`);
  }
} catch (error) {
  console.error('Произошла ошибка:', error.message);
}

// Пример 3: Обработка исключительных ситуаций
console.log('\nПример 3: Обработка исключительных ситуаций');

// Отрицательное число
try {
  console.log('Попытка вычислить факториал отрицательного числа:');
  FactorialGenerator.calculateFactorial(-5);
} catch (error) {
  console.error('Произошла ошибка:', error.message);
}

// Дробное число
try {
  console.log('\nПопытка вычислить факториал дробного числа:');
  FactorialGenerator.calculateFactorial(3.5);
} catch (error) {
  console.error('Произошла ошибка:', error.message);
}

// Слишком большое количество факториалов
try {
  console.log('\nПопытка сгенерировать слишком много факториалов:');
  FactorialGenerator.generateFactorials(1001);
} catch (error) {
  console.error('Произошла ошибка:', error.message);
}
