#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/my_class.h"

using ::testing::_;
using ::testing::Return;

// Тестирование метода MyClass::add
TEST(MyClassTest, AddTest) {
    // Создаем mock-объект
    MockMyDependency mock_dependency;
    // Задаем поведение метода getValue()
    EXPECT_CALL(mock_dependency, getValue()).WillOnce(Return(2));
    // Создаем объект нашего класса и передаем ему mock-объект
    MyClass my_class(&mock_dependency);
    // Вызываем метод add() и проверяем результат
    EXPECT_EQ(my_class.add(2), 4);
}
