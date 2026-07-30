# Clean Code Principles

Clean code is code that is easy to understand, safe to change, and honest about what it does.
Two common principles that help with this are **SOLID** and **DRY**.

This page uses Robert C. Martin, also known as Uncle Bob, as the reference point.

## SOLID

SOLID is a set of five object-oriented design principles.
They help keep code modular, testable, and easier to extend without breaking existing behavior.

| Letter | Principle | Meaning |
| --- | --- | --- |
| S | Single Responsibility Principle | A class or module should have one clear reason to change. |
| O | Open/Closed Principle | Code should be open for extension but closed for modification. |
| L | Liskov Substitution Principle | A derived type should be usable anywhere its base type is expected. |
| I | Interface Segregation Principle | Prefer small, focused interfaces over large interfaces that force unused methods. |
| D | Dependency Inversion Principle | High-level code should depend on abstractions, not concrete low-level details. |

### Single Responsibility Principle

A class should focus on one responsibility.
For example, a `ReportGenerator` should not also know how to save files, send emails, and format database queries.

Better design separates those concerns:

- `ReportGenerator` creates the report.
- `ReportFormatter` formats the output.
- `ReportRepository` stores or loads report data.
- `EmailSender` sends the report.

This makes each part easier to test and change.

### Open/Closed Principle

When a new behavior is needed, prefer adding a new implementation instead of editing stable code repeatedly.

For example, instead of one large function with many `if` statements for payment types, define a common `PaymentProcessor` interface and add a new processor for each payment method.

The existing checkout code can stay stable while new payment methods are added.

### Liskov Substitution Principle

If code expects a base type, derived types should not surprise it.

For example, if `Bird` has a `Fly()` method, then a `Penguin` subclass that cannot fly breaks expectations.
A better design might separate flying behavior into a smaller interface, such as `FlyingAnimal`.

The key idea is that inheritance should model real substitutability, not just similarity.

### Interface Segregation Principle

Clients should not be forced to depend on methods they do not use.

Instead of one large interface like this:

```cpp
class Machine {
public:
    virtual void Print() = 0;
    virtual void Scan() = 0;
    virtual void Fax() = 0;
};
```

Prefer smaller interfaces:

```cpp
class Printer {
public:
    virtual void Print() = 0;
};

class Scanner {
public:
    virtual void Scan() = 0;
};
```

A simple printer can implement only `Printer` without pretending it can scan or fax.

### Dependency Inversion Principle

High-level business logic should not be tightly coupled to low-level implementation details.

For example, an order service should depend on an abstract `PaymentGateway`, not directly on one concrete payment provider.
That makes it easier to test the service and replace the provider later.

```cpp
class PaymentGateway {
public:
    virtual ~PaymentGateway() = default;
    virtual void Charge(int amountInCents) = 0;
};

class OrderService {
public:
    explicit OrderService(PaymentGateway& paymentGateway)
        : paymentGateway_(paymentGateway) {}

    void PlaceOrder(int amountInCents) {
        paymentGateway_.Charge(amountInCents);
    }

private:
    PaymentGateway& paymentGateway_;
};
```

`OrderService` does not need to know which payment provider is used.

## DRY

DRY means **Don't Repeat Yourself**.

The principle says that each piece of knowledge should have one clear, authoritative place in the codebase.
When the same rule, calculation, constant, or behavior is copied into many places, every future change must be repeated carefully in all of them.

Duplication creates common problems:

- A bug fix may be applied in one place but missed in another.
- Business rules can drift apart over time.
- Tests may pass for one copy and fail for another.
- The code becomes harder to read because the reader must compare similar blocks.

## DRY Example

Repeated logic:

```cpp
double priceWithTax = price * 1.21;
double shippingWithTax = shipping * 1.21;
double discountWithTax = discount * 1.21;
```

Better:

```cpp
constexpr double taxMultiplier = 1.21;

double AddTax(double value) {
    return value * taxMultiplier;
}

double priceWithTax = AddTax(price);
double shippingWithTax = AddTax(shipping);
double discountWithTax = AddTax(discount);
```

Now the tax rule has one clear location.

## DRY Is Not Blind Deduplication

DRY does not mean every similar-looking line must be merged.
Sometimes two pieces of code look similar today but represent different ideas that may change independently later.

Avoid premature abstraction when:

- The duplication is small and harmless.
- The repeated code belongs to different concepts.
- A shared abstraction would make the code harder to understand.
- The future direction is still unclear.

A useful rule of thumb is: remove duplication when the repeated code represents the same knowledge, not merely the same shape.

## SOLID vs DRY

| Principle | Focus | Main Benefit |
| --- | --- | --- |
| SOLID | Design structure and dependencies | Easier extension, testing, and substitution |
| DRY | Avoiding repeated knowledge | Fewer bugs and simpler maintenance |

They often work together.
SOLID helps decide where responsibilities belong.
DRY helps avoid scattering the same decision across the codebase.

## More Clean Code Principles From Uncle Bob

SOLID and DRY are important, but Uncle Bob's clean code guidance covers more than design principles.
These ideas come from his books and Clean Coder blog.

| Principle | Meaning |
| --- | --- |
| Meaningful names | Names should reveal intent and make the code read close to the problem being solved. |
| Small functions | Functions should be short, focused, and do one thing. |
| One level of abstraction per function | A function should not mix high-level policy with low-level details. |
| Avoid surprising side effects | A function should not secretly change unrelated state while appearing to do something else. |
| Prefer expressive code over comments | Comments are useful sometimes, but code should first try to explain itself through names and structure. |
| Format code consistently | Formatting communicates structure and helps readers move through the code. |
| Handle errors cleanly | Error handling should not obscure the normal flow of the program. |
| Keep classes small | Classes should have focused responsibilities and high cohesion. |
| Separate objects and data structures | Objects hide data behind behavior; data structures expose data and have little behavior. Mixing the two carelessly creates confusion. |
| Write clean tests | Tests should be readable, fast enough to run often, and clear about the behavior they verify. |
| Leave the code better than you found it | Small continuous improvements keep code from degrading over time. |
| Keep details out of high-level policy | Business rules should not depend directly on databases, frameworks, user interfaces, or other low-level details. |

## Meaningful Names

A good name answers the reader's first question: what is this?

Prefer names that describe intent:

```cpp
int elapsedDays;
int retryCount;
bool isAccountLocked;
```

Avoid names that force the reader to decode purpose:

```cpp
int d;
int n;
bool flag;
```

Short names are fine for very small scopes, such as loop indexes.
For important concepts, use names that carry meaning from the problem domain.

## Small Functions

Uncle Bob strongly favors small functions that do one thing.
A function is easier to read when its name says what it does and its body stays at the same conceptual level.

Harder to read:

```cpp
void ProcessOrder(Order& order) {
    if (order.items.empty()) {
        throw std::invalid_argument("empty order");
    }

    int total = 0;
    for (const auto& item : order.items) {
        total += item.priceInCents * item.quantity;
    }

    DatabaseConnection connection;
    connection.Execute("insert into orders ...");

    EmailClient emailClient;
    emailClient.Send(order.customerEmail, "Order confirmed");
}
```

Cleaner:

```cpp
void ProcessOrder(Order& order) {
    Validate(order);
    const auto total = CalculateTotal(order);
    Save(order, total);
    SendConfirmation(order);
}
```

The second version lets the reader understand the policy first.
The details still exist, but they are moved into focused functions.

## Comments

Comments are not automatically bad, but they should not compensate for unclear code.

Prefer this:

```cpp
if (isEligibleForDiscount(customer, order)) {
    ApplyDiscount(order);
}
```

Over this:

```cpp
// Check if the customer can get a discount.
if (customer.type == 2 && order.total > 10000 && !customer.isBlocked) {
    order.total -= 1000;
}
```

Useful comments explain things that the code itself cannot express well, such as legal constraints, surprising decisions, warnings, or public API expectations.

## Error Handling

Error handling should be clear and separated from the main idea when possible.
The normal path of the code should remain readable.

Prefer meaningful exceptions, clear return types, and narrow error scopes.
Avoid spreading the same error checks everywhere if a helper, guard, or boundary can centralize the rule.

## Clean Tests

Tests are also code.
If tests are hard to read, they become hard to trust and expensive to maintain.

Clean tests usually follow this shape:

```cpp
TEST(OrderServiceTest, RejectsEmptyOrder) {
    Order order;
    OrderService service;

    EXPECT_THROW(service.Process(order), std::invalid_argument);
}
```

A good test makes the expected behavior obvious.

## Clean Architecture Principle

Uncle Bob's architecture guidance extends the same clean code idea to system structure.
High-level policy should not depend on low-level details.

Examples of low-level details:

- Database engines
- Web frameworks
- UI frameworks
- File formats
- External services

The business rules should be protected from those details.
This keeps the system easier to test and easier to change when infrastructure changes.

## References

Only Robert C. Martin sources are listed here:

- Robert C. Martin, *Clean Code: A Handbook of Agile Software Craftsmanship*, Prentice Hall, 2008.
- Robert C. Martin, *Agile Software Development, Principles, Patterns, and Practices*, Prentice Hall, 2002.
- Robert C. Martin, *Clean Architecture: A Craftsman's Guide to Software Structure and Design*, Prentice Hall, 2017.
- Robert C. Martin, "The Single Responsibility Principle", The Clean Code Blog, 2014.
- Robert C. Martin, "The Open Closed Principle", The Clean Code Blog, 2014.
- Robert C. Martin, "Solid Relevance", The Clean Code Blog, 2020.
- Robert C. Martin, "Necessary Comments", The Clean Code Blog, 2017.
- Robert C. Martin, "The Clean Architecture", The Clean Code Blog, 2012.
