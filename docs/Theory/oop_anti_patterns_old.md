# Object-Oriented Programming Anti-Patterns

Object-oriented programming helps model behavior, ownership, and collaboration between parts of a system.
It becomes harmful when classes, inheritance, and abstraction are used without clear responsibility boundaries.

An anti-pattern is a recurring design choice that looks useful at first but usually creates rigidity, hidden coupling, or confusing behavior.

This page focuses on common object-oriented anti-patterns and practical ways to avoid them.

## God Object

A God object knows too much or does too much.
It becomes the center of the system and starts owning unrelated responsibilities.

Common symptoms:

- One class coordinates business rules, persistence, logging, and user interface concerns.
- Many parts of the codebase depend on one large class.
- Small changes in unrelated features require editing the same type.
- The class has many methods and many private fields with weak cohesion.

Example shape:

```cpp
class ApplicationManager {
public:
    void LoadSettings();
    void SaveSettings();
    void ConnectDatabase();
    void RenderUi();
    void ProcessOrders();
    void SendEmails();
};
```

Why it is harmful:

- It violates the Single Responsibility Principle.
- Testing becomes expensive because many dependencies are pulled in together.
- Reuse becomes harder because useful logic is trapped inside a large type.
- The class becomes a merge-conflict hotspot in team development.

Better approach:

- Split the class by responsibility.
- Move infrastructure concerns away from domain logic.
- Let a thin coordinator compose smaller services instead of owning all behavior directly.

## Deep Inheritance Hierarchies

Inheritance is useful when derived types are true substitutes for the base type.
It becomes an anti-pattern when behavior is spread across many layers and understanding one class requires reading half the hierarchy.

Common symptoms:

- A type has many base classes across several abstraction layers.
- Small behavior changes require overrides in distant subclasses.
- Constructors must call a fragile chain of base-class initialization.
- Readers need to inspect base implementations to understand current behavior.

Why it is harmful:

- Tight coupling between parent and child classes.
- Fragile behavior because changes in a base class affect many descendants.
- Difficult reasoning about construction order, virtual dispatch, and invariants.
- Increased risk of violating Liskov Substitution.

Better approach:

- Prefer composition when sharing behavior.
- Keep inheritance shallow and intentional.
- Use interfaces or abstract base classes for polymorphism when shared state is not required.

## Base Class With Optional Behavior

This appears when a base class defines methods that only some subclasses can support.

Example:

```cpp
class Bird {
public:
    virtual void Fly() = 0;
};

class Penguin : public Bird {
public:
    void Fly() override {
        throw std::logic_error("Penguins do not fly");
    }
};
```

Why it is harmful:

- The inheritance model lies about what the derived type can do.
- Callers need special-case knowledge of specific subclasses.
- Polymorphism stops being safe.

Better approach:

- Model capabilities with smaller interfaces.
- Use composition for optional behavior.
- Design abstractions around guaranteed behavior, not wishful similarity.

## Feature Envy

Feature envy happens when a method spends most of its time reaching into another object's data instead of using its own object's state and behavior.

Example shape:

```cpp
double PriceCalculator::Calculate(const Order& order) {
    return order.ItemsTotal() - order.Discount() + order.ShippingCost();
}
```

The example itself is not automatically bad, but it becomes a problem if one class repeatedly manipulates another class's internals and understands that object's rules better than the object itself.

Why it is harmful:

- Behavior is placed far from the data and rules it depends on.
- Changes to one domain concept ripple into unrelated classes.
- Encapsulation weakens over time.

Better approach:

- Move logic closer to the object that owns the underlying rules.
- Expose domain operations instead of raw data when appropriate.

## Anemic Domain Model

An anemic domain model stores data in classes while all real behavior lives elsewhere.
The result is object syntax with procedural design.

Example shape:

```cpp
class BankAccount {
public:
    double balance;
};

class BankAccountService {
public:
    void Withdraw(BankAccount& account, double amount);
    void Deposit(BankAccount& account, double amount);
};
```

Why it is harmful:

- The type that owns the state does not protect its invariants.
- The codebase accumulates service classes that manipulate exposed data structures.
- Domain logic becomes scattered across helpers.

Better approach:

- Put invariant-protecting behavior into the domain type when the type truly owns that rule.
- Keep simple data-transfer objects separate from rich domain objects.

## Shotgun Surgery Through Inheritance

This anti-pattern appears when one conceptual change requires editing many related classes because behavior is duplicated or partially overridden throughout a hierarchy.

Example symptoms:

- A new rule requires touching the base class and several subclasses.
- Similar methods are copied with small variations across related types.
- Bug fixes must be repeated in multiple overrides.

Why it is harmful:

- Changes become slow and risky.
- Regressions appear when one override is missed.
- The design gives the illusion of reuse without centralizing knowledge.

Better approach:

- Extract shared behavior into one place.
- Prefer composition or strategy objects for varying behavior.
- Keep differences explicit and local.

## Interface Pollution

Interface pollution happens when a type exposes more methods than its clients need.
This often grows from convenience or from trying to satisfy too many use cases with one abstraction.

Why it is harmful:

- Clients depend on more surface area than necessary.
- Implementations become heavier and harder to change.
- Mocking and testing become noisy.

Better approach:

- Use small, focused interfaces.
- Split read and write operations when they have different consumers.
- Design APIs around client needs, not around all possible future uses.

## Object-Oriented Overengineering

Sometimes object orientation is used where simpler code would be clearer.
This includes creating factories, managers, abstract base classes, and wrappers for behavior that has only one implementation and no real variation.

Why it is harmful:

- More files and indirection without corresponding value.
- Harder debugging because readers must jump through abstractions.
- The code communicates imagined flexibility rather than actual requirements.

Better approach:

- Start with the simplest design that fits current needs.
- Add polymorphism only when multiple behaviors are real and stable enough to justify abstraction.

## How To Review OOP Design For Anti-Patterns

When reading object-oriented code, ask:

- Which class owns this responsibility?
- Does inheritance represent real substitutability?
- Are invariants protected close to the data they depend on?
- Is this abstraction reducing complexity or only moving it around?
- Would composition express the relationship more clearly than inheritance?
- Are clients forced to know too much about concrete subclasses?

## Summary

Object-oriented anti-patterns usually come from unclear responsibility boundaries, inheritance used for convenience, or abstractions that promise more than they safely provide.

The most reliable countermeasures are small cohesive types, honest interfaces, composition over deep inheritance, and behavior placed close to the rules it enforces.

## Sources Used For This Page

This page was drafted from established object-oriented design knowledge and aligned to the style of nearby theory documentation in this repository.
No external web source was consulted while writing it.

In-repository references used for tone and adjacent concepts:

- `docs/Theory/clean_code_principles.md`

Related background concepts referenced from general engineering knowledge:

- SOLID design principles
- cohesion and coupling
- composition over inheritance
