# Finance Accounting System

Система учета финансов для банковского приложения, реализованная с применением принципов SOLID, GRASP и паттернов проектирования GoF.

##  Архитектура проекта

### Структура проекта
```
kpo2/
├── include/
│ ├── domain.hpp 
│ ├── repository.hpp 
│ ├── facades.hpp
│ ├── factory.hpp
│ ├── analytics.hpp 
│ ├── export_visitors.hpp
│ ├── importer.hpp 
│ ├── commands.hpp 
│ └── di_container.hpp 
├── src/ 
│ ├── main.cpp
│ ├── facades.cpp 
│ ├── analytics.cpp
│ ├── export_visitors.cpp
| ├── commands.cpp
| ├── repository.cpp
│ └── CMakeLists.txt
├── tests/ 
│ ├── test_core.cpp
│ └── CMakeLists.txt
├── third_party/
│ ├── catch.hpp
├── README.md
└── CMakeLists.txt

```

##  Реализованная функциональность

### Основные требования
-  Создание, редактирование и удаление счетов
-  Создание, редактирование и удаление категорий  
-  Создание, редактирование и удаление операций (доходов/расходов)

### Дополнительная функциональность
-  Аналитика: разница доходов/расходов за период
-  Аналитика: группировка по категориям
-  Экспорт данных в CSV, JSON форматы
-  Импорт данных (шаблонная структура)

##  Паттерны проектирования

### Порождающие паттерны
1. **Factory** (`DomainFactory`)
   - Создание валидированных доменных объектов
   - Централизованное управление ID
   - Запрет создания операций с отрицательными суммами

```cpp
DomainFactory factory;
auto account = factory.createAccount("Основной счет", 1000.0);
auto category = factory.createCategory(MoneyType::Income, "Зарплата");
```

### Структурные паттерны

2. **Facade** (`AccountFacade`, `CategoryFacade`, `OperationFacade`, `AnalyticsFacade`)
   - Упрощенный интерфейс для работы со сложными подсистемами
   - Группировка связанных операций по смыслу
   - Сокрытие сложности работы с репозиториями и валидацией

```cpp
AccountFacade accountFacade(accountRepo);
accountFacade.create("Основной счет", 1000.0);
accountFacade.updateBalance(1, 1500.0);
```
3. **Visitor** (`ExportVisitor`, `CSVExportVisitor`, `JSONExportVisitor`)
   - Экспорт данных в различные форматы без изменения классов доменной модели
   - Легкое добавление новых форматов экспорта
   - Посещение различных типов объектов единым интерфейсом

```cpp
CSVExportVisitor csvVisitor;
for (auto& account : accounts) {
    cout << csvVisitor.visitAccount(account) << endl;
}
```
4. **Repository** (`IRepository<T>`, `InMemoryRepository<T>`)

    - Абстракция доступа к данным

    - Единый интерфейс для работы с различными источниками данных

    - Легкая замена хранилища (in-memory, база данных, файлы)
```
IRepository<BankAccount>& repo = InMemoryRepository<BankAccount>();
repo.add(account);
auto found = repo.get(1);

```

**Decorator** (`TimedCommand`)

- Динамическое добавление функциональности без изменения классов

- Логирование времени выполнения операций

- Возможность цепочки декораторов

```
auto command = make_unique<CreateAccountCommand>(facade, "Счет", 1000.0);
auto timedCommand = make_unique<TimedCommand>(move(command), logger);
timedCommand->execute();
```

### Поведенческие паттерны

6. **Template Method** (`BaseImporter`, `CSVImporter`, `JSONImporter`)
   - Единый алгоритм импорта с изменяющимися шагами парсинга
   - Переиспользование общей логики чтения файлов и валидации
   - Легкое добавление новых форматов импорта

```cpp
class BaseImporter {
protected:
    virtual void parseAndLoad(const string& data) = 0;
public:
    void importFromFile(const string& path) {
        auto data = readFile(path);
        validateData(data);
        parseAndLoad(data);
        postImportFix();
    }
};
```

7. **Command** (`ICommand`, `CreateAccountCommand`, `CreateCategoryCommand`, `CreateOperationCommand`)
   - Инкапсуляция пользовательских сценариев в объекты
   - Поддержка отмены операций и очереди команд
   - Единый интерфейс для выполнения операций

```cpp
unique_ptr<ICommand> cmd = make_unique<CreateAccountCommand>(facade, "Счет", 1000.0);
cmd->execute();
```
8. **Strategy** (различные импортеры и экспортеры)
   - Взаимозаменяемые алгоритмы импорта/экспорта
   - Динамический выбор формата работы с данными
   - Изоляция алгоритмов от клиентского кода

##  Принципы SOLID

### Single Responsibility Principle (SRP)
- `AccountFacade` - управление только счетами
- `CategoryFacade` - управление только категориями  
- `AnalyticsFacade` - только аналитические вычисления
- `ExportVisitor` - только преобразование в форматы
- Каждый класс имеет одну и только одну причину для изменения

### Open/Closed Principle (OCP)
- Легкое добавление новых форматов экспорта через наследование от `ExportVisitor`
- Новые импортеры через наследование от `BaseImporter`
- Новые команды через реализацию `ICommand`
- Система открыта для расширения, но закрыта для модификации

### Liskov Substitution Principle (LSP)
- Все реализации `IRepository` взаимозаменяемы
- Все команды реализуют единый интерфейс `ICommand`
- Все посетители могут быть использованы вместо базового `ExportVisitor`
- Все импортеры могут заменить `BaseImporter`

### Interface Segregation Principle (ISP)
- Узкоспециализированные интерфейсы: `IRepository<T>` для каждого типа
- `ICommand` содержит только методы execute() и name()
- `ExportVisitor` разделен по типам посещаемых объектов
- Клиенты не зависят от методов, которые они не используют

### Dependency Inversion Principle (DIP)
- Зависимости от абстракций (`IRepository`, `ICommand`, `ExportVisitor`)
- Внедрение зависимостей через конструкторы
- Модули верхнего уровня не зависят от модулей нижнего уровня

```cpp
class AccountFacade {
    IRepository<BankAccount>& repo; 
public:
    AccountFacade(IRepository<BankAccount>& r) : repo(r) {}
};
```

##  Принципы GRASP

### High Cohesion
- Логически связанные операции сгруппированы в фасадах
- `OperationFacade` управляет только операциями и связанными счетами
- `AnalyticsFacade` содержит только аналитические методы
- Каждый класс имеет четкую и сфокусированную ответственность

### Low Coupling  
- Минимальные зависимости между модулями
- Связи через абстракции, а не конкретные реализации
- Фасады уменьшают coupling между клиентами и подсистемами
- Посетители позволяют добавлять функциональность без изменения классов

```cpp
class AnalyticsFacade {
    IRepository<Operation>& opRepo;    
    IRepository<Category>& catRepo;    
};
```

##  Запуск проекта

### Требования
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10+

### Сборка
```bash
mkdir build && cd build
cmake ..
make
```
### Запуск демонстрации
```bash
./src/finance_app
```
### Запуск тестов

```bash
./tests/finance_tests
```