# Object-Oriented Programming Anti-Patterns

Object-oriented programming is useful when types have clear responsibilities, encapsulate invariants, and collaborate through honest interfaces.
It becomes harmful when inheritance, abstraction, or indirection are used without a real design need.

This page focuses on anti-patterns that commonly appear in object-oriented design regardless of programming language.

## What Makes An OOP Anti-Pattern

Most object-oriented anti-patterns have one or more of these traits:

- responsibilities accumulate in one place
- inheritance is used for code reuse instead of true substitutability
- behavior is separated from the data and rules it depends on
- abstractions exist for imagined future variation instead of current needs
- one change requires edits across many types

In Refactoring.Guru terms, many of these problems show up as large classes, feature envy, refused bequest, data classes, speculative generality, and shotgun surgery.

## God Object Or Large Class

A God object is a class that has absorbed too many responsibilities.
Refactoring.Guru describes the related Large Class smell as a class that grows to contain many fields, methods, or lines of code.

Common symptoms:

- one class coordinates business rules, persistence, formatting, validation, and I/O
- many parts of the system depend on the same class
- the class has weak cohesion and many reasons to change

Why it is harmful:

- it violates the Single Responsibility Principle
- testing becomes expensive because many dependencies are pulled together
- changes collide in the same file and the same type

Better approach:

- split the class by responsibility
- extract smaller services or domain types
- keep orchestration thin and move detailed rules to the types that own them

## Refused Bequest

Refused bequest appears when a subclass inherits behavior that it does not actually support or want.
Refactoring.Guru describes this as a hierarchy where the subclass uses only part of what it inherits, or overrides inherited behavior just to reject it.

Typical example:

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

- the type hierarchy lies about capabilities
- clients must know special cases about concrete subclasses
- substitutability breaks down

Better approach:

- replace convenience inheritance with composition or delegation
- design abstractions around guaranteed behavior
- use smaller interfaces for optional capabilities

## Feature Envy

Feature envy is a method that uses another object's data more than its own.
Refactoring.Guru recommends moving behavior to the place where the related data changes together.

Why it is harmful:

- behavior is far from the rules it depends on
- encapsulation weakens over time
- change ripples across unrelated types

Better approach:

- move behavior closer to the data and invariants it operates on
- expose domain operations instead of leaking raw internal state

## Anemic Domain Model Or Data Class Abuse

Refactoring.Guru defines a Data Class as a class that mainly contains fields plus crude getters and setters, while other classes perform the real work.

In domain-heavy code, this becomes an anemic model: objects store state, but services scattered around the system own the actual rules.

Why it is harmful:

- invariants are not protected where the state lives
- logic is duplicated in service classes and helpers
- domain behavior becomes harder to discover and test

Better approach:

- keep data-transfer objects simple and explicit
- when a type owns business rules, put those rules on the type
- move methods to the object that owns the underlying concept

## Shotgun Surgery

Refactoring.Guru defines shotgun surgery as a situation where one conceptual change requires many small edits across many classes.

Why it is harmful:

- every change becomes slow and risky
- fixes are easy to miss in one of many locations
- the design spreads one responsibility across too many types

Better approach:

- gather behavior that changes together into one place
- use extracted classes or strategies where variation is real
- reduce cross-cutting duplication

## Speculative Generality

Speculative generality is code built for a future that never arrives.
Refactoring.Guru describes it as unused classes, methods, fields, or parameters created "just in case".

Common symptoms:

- abstract base classes with one implementation and no real variation
- factories, managers, or wrappers with no clear need
- hooks and extension points that nobody uses

Why it is harmful:

- the reader must understand flexibility that provides no current value
- code becomes harder to navigate
- future changes must preserve unnecessary abstractions

Better approach:

- start from the simplest design that satisfies current requirements
- add polymorphism only when multiple real behaviors exist
- delete unused extension points unless a framework boundary genuinely needs them

## Parallel Or Deep Hierarchy Problems

Deep hierarchies and parallel inheritance structures often signal that inheritance is carrying too much design weight.
They make it harder to understand construction rules, override chains, and the real source of behavior.

Why it is harmful:

- readers must inspect many layers to understand one type
- base-class changes affect many descendants
- one new feature may require creating multiple matching subclasses

Better approach:

- prefer composition over deep inheritance
- keep inheritance shallow and intentional
- use interfaces or strategies for behavior variation

## Practical Review Checklist

When reviewing object-oriented code, ask:

- which class owns this responsibility?
- does this inheritance model represent true substitutability?
- are data and the rules that govern it close together?
- is this abstraction solving a present problem or an imagined one?
- if one requirement changes, how many classes must change with it?

## Summary

Object-oriented anti-patterns usually come from unclear responsibilities, dishonest inheritance, and abstractions created without a stable need.
The most reliable countermeasures are cohesion, encapsulation, shallow and honest hierarchies, and composition where reuse does not imply subtype identity.

## Web References

- [Python Software Foundation, Python 3 documentation](https://docs.python.org/3/)
- [Refactoring.Guru code smells catalog](https://refactoring.guru/refactoring/smells)
- [Refactoring.Guru, Large Class](https://refactoring.guru/smells/large-class)
- [Refactoring.Guru, Feature Envy](https://refactoring.guru/smells/feature-envy)
- [Refactoring.Guru, Refused Bequest](https://refactoring.guru/smells/refused-bequest)
- [Refactoring.Guru, Shotgun Surgery](https://refactoring.guru/smells/shotgun-surgery)
- [Refactoring.Guru, Data Class](https://refactoring.guru/smells/data-class)
- [Refactoring.Guru, Speculative Generality](https://refactoring.guru/smells/speculative-generality)
- [Martin Fowler, software architecture and refactoring articles](https://martinfowler.com/)
